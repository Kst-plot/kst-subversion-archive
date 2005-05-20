/***************************************************************************
                          ksteventmonitorentry.cpp  -  description
                             -------------------
    begin                : Tue Apr 6 2004
    copyright            : (C) 2004 The University of British Columbia
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *   Permission is granted to link with any opensource library             *
 *                                                                         *
 ***************************************************************************/

// include files for Qt
#include <qstylesheet.h>

// include files for KDE
#include <kdebug.h>
#include <klocale.h>

// application specific includes
#include "enodes.h"
#include "emailthread.h"
#include "kst.h"
#include "kstdatacollection.h"
#include "ksteventmonitorentry.h"
#include "ksteventmonitor_i.h"

extern "C" int yyparse();
extern "C" void *ParsedEquation;
extern "C" struct yy_buffer_state *yy_scan_string(const char*);

const QString EventMonitorEntry::OUTXVECTOR = "X";
const QString EventMonitorEntry::OUTYVECTOR = "Y";

EventMonitorEntry::EventMonitorEntry(const QString &in_tag) : KstDataObject() {
  _level = KstDebug::Warning;
  _bLogKstDebug = true;
  _bLogEMail = false;
  _bLogELOG = false;

  commonConstructor( in_tag );
  setDirty();
}


EventMonitorEntry::EventMonitorEntry(const QDomElement &e) {
  QString strTag;

  _level = KstDebug::Warning;
  _bLogKstDebug = true;
  _bLogEMail = false;
  _bLogELOG = false;

  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        strTag = e.text();
      } else if (e.tagName() == "equation") {
        _strEvent = e.text();
      } else if (e.tagName() == "description") {
        _strDescription = e.text();
      } else if (e.tagName() == "logdebug") {
        _bLogKstDebug = e.text().toInt();
      } else if (e.tagName() == "loglevel") {
        _level = (KstDebug::LogLevel)e.text().toInt();
      } else if (e.tagName() == "logemail") {
        _bLogEMail = e.text().toInt();
      } else if (e.tagName() == "logelog") {
        _bLogELOG = e.text().toInt();
      } else if (e.tagName() == "emailRecipients") {
        _strEMailRecipients = e.text();
      }
    }
    n = n.nextSibling();
  }

  commonConstructor( strTag );

  // wait for the initial update, as we don't want to trigger elog entries
  //  until we are sure the document is open...
  QTimer::singleShot( 500, this, SLOT(slotUpdate()));
}


void EventMonitorEntry::commonConstructor(const QString &in_tag) {
  int NS = 1;

  _iNumDone = 0;
  _bIsValid = false;
  _pExpression = 0L;

  _typeString = i18n("Event");
  KstObject::setTagName(in_tag);

  KstVectorPtr xv = new KstVector(in_tag + "-x", NS);
  xv->setProvider(this);
  KST::addVectorToList(xv);
  _xVector = _outputVectors.insert(OUTXVECTOR, xv);

  KstVectorPtr yv = new KstVector(in_tag + "-y", NS);
  yv->setProvider(this);
  KST::addVectorToList(yv);
  _yVector = _outputVectors.insert(OUTYVECTOR, yv);
}


bool EventMonitorEntry::reparse() {
  _bIsValid = false;
  if (!_strEvent.isEmpty()) {
    QMutexLocker ml(&Equation::mutex());
    yy_scan_string(_strEvent.latin1());
    int rc = yyparse();
    if (rc == 0) {
      _pExpression = static_cast<Equation::Node*>(ParsedEquation);
      Equation::Context ctx;
      Equation::FoldVisitor vis(&ctx, &_pExpression);
      KstStringMap stm;
      _pExpression->collectObjects(_vectorsUsed, _inputScalars, stm);

      _bIsValid = true;
    } else {
      delete (Equation::Node*)ParsedEquation;
    }
    ParsedEquation = 0L;
  }
  return _bIsValid;
}


void EventMonitorEntry::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<event>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<equation>" << QStyleSheet::escape(_strEvent) << "</equation>" << endl;
  ts << l2 << "<description>" << QStyleSheet::escape(_strDescription) << "</description>" << endl;
  ts << l2 << "<logdebug>" << QString::number(_bLogKstDebug) << "</logdebug>" << endl;
  ts << l2 << "<loglevel>" << QString::number(_level) << "</loglevel>" << endl;
  ts << l2 << "<logemail>" << QString::number(_bLogEMail) << "</logemail>" << endl;
  ts << l2 << "<logelog>" << QString::number(_bLogELOG) << "</logelog>" << endl;
  ts << l2 << "<emailRecipients>" << QStyleSheet::escape(_strEMailRecipients) << "</emailRecipients>" << endl;
  ts << indent << "</event>" << endl;
}


EventMonitorEntry::~EventMonitorEntry() {
  logImmediately();

  delete _pExpression;
  _pExpression = 0L;
}


void EventMonitorEntry::slotUpdate() {
  // possible deadlock?  bad idea.  updates should only happen in update thread
  KstWriteLocker wl(this);
  update();
}


KstObject::UpdateType EventMonitorEntry::update(int updateCounter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }
  
  KstVectorPtr xv = *_xVector;
  KstVectorPtr yv = *_yVector;
  Equation::Context ctx;
  double *pdRawValuesX = 0L;
  double *pdRawValuesY = 0L;
  double dValue;
  int ns = 1;

  if (!_pExpression) {
    reparse();
  }

  if (_vectorsUsed.count() > 0) {
    for (KstVectorMap::ConstIterator i = _vectorsUsed.begin(); i != _vectorsUsed.end(); ++i) {
      if (i.data()->length() > ns) {
        ns = i.data()->length();
      }
    }
  } else {
    ns = 1;
  }

  if (xv && yv) {
    if (xv->resize(ns)) {
      pdRawValuesX = xv->value();
    }

    if (yv->resize(ns)) {
      pdRawValuesY = yv->value();
    }
  }

  ctx.sampleCount = ns;
  ctx.x = 0.0;

  if (needToEvaluate()) {
    if (_pExpression) {
      for (ctx.i = _iNumDone; ctx.i < ns; ++ctx.i) {
        dValue = _pExpression->value(&ctx);
        if (dValue) {
          log( ctx.i );
          if (pdRawValuesX && pdRawValuesY) {
            pdRawValuesX[ctx.i] = ctx.i;
            pdRawValuesY[ctx.i] = 1.0;
          }
        } else {
          if (pdRawValuesX && pdRawValuesY) {
            pdRawValuesX[ctx.i] = ctx.i;
            pdRawValuesY[ctx.i] = 0.0;
          }
        }
      }
      _iNumDone = ns;
      logImmediately();
    }
  } else {
    _iNumDone = ns;
  }

  if (xv) {
    xv->setDirty();
    xv->update(updateCounter);
  }

  if (yv) {
    yv->setDirty();
    yv->update(updateCounter);
  }

  return setLastUpdateResult(NO_CHANGE);
}


void EventMonitorEntry::setEvent(const QString& strEvent) {
  if (_strEvent != strEvent) {
    _strEvent = strEvent;
    _vectorsUsed.clear();
    _inputScalars.clear();

    _iNumDone = 0;
    _bIsValid = false;

    delete _pExpression;
    _pExpression = 0L;
  }
}


bool EventMonitorEntry::needToEvaluate() {
  return _bLogKstDebug || _bLogEMail || _bLogELOG;
}


void EventMonitorEntry::logImmediately( ) {
  bool bRange = false;
  int iIndexOld = 0;
  int iIndex = 0;
  int iSize = _indexArray.size();
  int i;

  if( iSize > 0 ) {
    QString str;
    QString strRange;

    for( i=0; i<iSize; i++ ) {
      iIndex = *(_indexArray.at(i));
      if( i == 0 ) {
        strRange.setNum( iIndex );
      } else if( !bRange && iIndex == iIndexOld+1 ) {
        bRange = true;
      } else if( bRange && iIndex != iIndexOld+1 ) {
        strRange = i18n("%1-%2,%3").arg( strRange ).arg( iIndexOld ).arg( iIndex );
        bRange = false;
      } else if( iIndex != iIndexOld+1 ) {
        strRange = i18n("%1,%2").arg( strRange ).arg( iIndex );
      }
      iIndexOld = iIndex;
    }

    if( bRange ) {
      strRange = i18n("%1-%2").arg( strRange ).arg( iIndex );
    }

    if( _strDescription.isEmpty() ) {
      str = i18n("Event Monitor: %1: %2").arg( _strEvent ).arg( strRange );
    } else {
      str = i18n("Event Monitor: %1: %2").arg( _strDescription ).arg( strRange );
    }

    if( _bLogKstDebug ) {
      KstDebug::self()->log( str, _level );
    }

    if( _bLogEMail && !_strEMailRecipients.isEmpty() ) {
      // FIXME: wrong thread - can crash (QStrings unguarded, at best)
      EMailThread* pThread = new EMailThread(_strEMailRecipients,
                              i18n("Kst Event Monitoring Notification"), str);
      pThread->send();
    }

    if (_bLogELOG) {
      // FIXME: wrong thread - can crash
      KstApp::inst()->EventELOGSubmitEntry(str);
    }

    _indexArray.clear();
  }
}


void EventMonitorEntry::log( const int& iIndex ) {
  _indexArray.append(iIndex);
  if (_indexArray.size() > 1000) {
    logImmediately();
  }
}


QString EventMonitorEntry::propertyString() const {
  return _strEvent;
}


void EventMonitorEntry::_showDialog() {
  KstEventMonitorI::globalInstance()->show_Edit(tagName());
}


void EventMonitorEntry::setDescription(const QString& str) {
  setDirty();
  _strDescription = str;
}


void EventMonitorEntry::setLevel(KstDebug::LogLevel level) {
  setDirty();
  _level = level;
}


void EventMonitorEntry::setExpression(Equation::Node* pExpression) {
  setDirty();
  _pExpression = pExpression;
}


void EventMonitorEntry::setLogKstDebug(bool bLogKstDebug) {
  setDirty();
  _bLogKstDebug = bLogKstDebug;
}


void EventMonitorEntry::setLogEMail(bool bLogEMail) {
  setDirty();
  _bLogEMail = bLogEMail;
}


void EventMonitorEntry::setLogELOG(bool bLogELOG) {
  setDirty();
  _bLogELOG = bLogELOG;
}


void EventMonitorEntry::setEMailRecipients(const QString& str) {
  setDirty();
  _strEMailRecipients = str;
}

#include "ksteventmonitorentry.moc"

// vim: ts=2 sw=2 et
