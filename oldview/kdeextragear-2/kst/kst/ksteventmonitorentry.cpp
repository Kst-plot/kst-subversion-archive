/***************************************************************************
                          ksteventmonitorentry.cpp  -  description
                             -------------------
    begin                : Tue Apr 6 2004
    copyright            : (C) 2004 by arw
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

#include <ctype.h>
#include <qtimer.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <qstylesheet.h>
#include <kdebug.h>
#include <klocale.h>
#include "enodes.h"    
#include "kstdoc.h"
#include "kstdatacollection.h"
#include "kstrvector.h"
#include "kstdebug.h"
#include "ksteventmonitorentry.h"

extern "C" int exparse();
extern "C" void *ParsedExpression;
extern "C" struct yy_buffer_state *ex_scan_string(const char*);

EventMonitorEntry::EventMonitorEntry() : QObject() {
  _level = KstDebug::Warning;
  _pExpression = NULL;
  _iRefCount = 1;
  _bLogKstDebug = TRUE;
}

EventMonitorEntry::EventMonitorEntry(const EventMonitorEntry &e) : QObject() {
  _strEvent	= e._strEvent;
  _strDescription = e._strDescription;
  _level = e._level;
  _pExpression = e._pExpression;
  _iRefCount = e._iRefCount;
  _bLogKstDebug = e._bLogKstDebug;
}

EventMonitorEntry::EventMonitorEntry(QDomElement &e) {
  _level = KstDebug::Warning;
  _pExpression = NULL;
  _bLogKstDebug = TRUE;
  
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "equation") {
        _strEvent = e.text();
      } else if (e.tagName() == "description") {
        _strDescription = e.text();
      } else if (e.tagName() == "logdebug") {
        _bLogKstDebug = e.text().toInt();
      } else if (e.tagName() == "loglevel") {
        _level = (KstDebug::LogLevel)e.text().toInt();
      }
    }
    n = n.nextSibling();
  }
  _iRefCount = 1;
}

void EventMonitorEntry::save(QTextStream &ts) {
  ts << "  <event>" << endl;
  ts << "    <equation>" << QStyleSheet::escape(_strEvent) << "</equation>" << endl;
  ts << "    <description>" << QStyleSheet::escape(_strDescription) << "</description>" << endl;
  ts << "    <logdebug>" << QString::number(_bLogKstDebug) << "</logdebug>" << endl;
  ts << "    <loglevel>" << QString::number(_level) << "</loglevel>" << endl;
  ts << "  </event>" << endl;  
}

EventMonitorEntry::~EventMonitorEntry() {
  logImmediately();
  
  delete _pExpression;
  _pExpression = NULL;
}

bool EventMonitorEntry::needToEvaluate() {
  bool bRetVal = false;
  
  if( _bLogKstDebug ) {
    bRetVal = true;
  }
  
  return bRetVal;
}

bool EventMonitorEntry::compile() {
  Equation::Context ctx;
  bool bRetVal = FALSE;
  int  rc;
  
  if (!_pExpression) {
    if (!_strEvent.isEmpty( )) {
      ex_scan_string(_strEvent.latin1());
      rc = exparse();
      if (rc == 0) {
        _pExpression = static_cast<Equation::Node*>(ParsedExpression);
        _pExpression->fold(&ctx);
//      _pExpression->collectVectors(VectorsUsed);
        ParsedExpression = NULL;
        
        bRetVal = TRUE;
      } else {
        delete (Equation::Node*)ParsedExpression;
        ParsedExpression = NULL;
      }
    }
  } else {
    bRetVal = TRUE;
  }
  
  return bRetVal;
}

void EventMonitorEntry::logImmediately( ) {
  QString str;
  bool bRange = false;
  int iIndexOld = 0;
  int iIndex = 0;
  int iSize = _indexArray.size();
  int i;
  
  if( iSize > 0 ) {
    if( _strDescription.isEmpty( ) ) {
      str = _strEvent;
    } else {
      str = _strDescription;
    }
    
    str = i18n("Event Monitor: %1: ").arg( str );
    for( i=0; i<iSize; i++ ) {
      iIndex = *(_indexArray.at(i));
      if( i == 0 ) {
        str += i18n("%1").arg( iIndex );
      } else if( !bRange && iIndex == iIndexOld+1 ) {        
        bRange = true;
      } else if( bRange && iIndex != iIndexOld+1 ) {
        str += i18n("-%1,%2").arg( iIndexOld ).arg( iIndex );
        bRange = false;
      } else if( iIndex != iIndexOld+1 ) {
        str += i18n(",%1").arg( iIndex );
      }
      iIndexOld = iIndex;
    }
    
    if( bRange ) {
      str += i18n("-%1").arg( iIndex );
    }
    
    if( _bLogKstDebug ) {
      KstDebug::self()->log( str, _level );
    }
    _indexArray.clear();
    _timeLastLogged = QTime::currentTime( );
  }
}

void EventMonitorEntry::log( const int& iIndex ) {
  _timeLastEvent = QTime::currentTime( );
  if( _timeLastEvent > _timeLastLogged.addSecs( 1*60 ) ) {
    _indexArray.append( iIndex );    
    logImmediately( );
  } else {
    _indexArray.append( iIndex );
    if( _indexArray.size() > 100 ) {
      logImmediately();
    } else {
      QTimer::singleShot( 1*60*1000, this, SLOT(logImmediately()) );
    }
  }
}

bool EventMonitorEntry::evaluate( Equation::Context& ctx ) {
  QString str;
  bool    bRetVal = FALSE;
  double  dValue;
  
  if (_pExpression) {
    dValue = _pExpression->value(&ctx);
    if (dValue) {
      log( ctx.i );
    } 
    bRetVal = FALSE;
  }
  
  return bRetVal;
}

void EventMonitorEntry::addRef( ) {
  _iRefCount++;
}


void EventMonitorEntry::release( ) {
  _iRefCount--;
  
  if( _iRefCount == 0 ) {
    delete _pExpression;
    _pExpression = NULL;
  }
}
