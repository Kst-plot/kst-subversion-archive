/***************************************************************************
                          kstvectorview.cpp: Curve subsets for KST
                             -------------------
    begin                : May 2007
    copyright            : (C) 2007 by D. Hanson
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <QTextDocument>

#include "dialoglauncher.h"
#include "kstdatacollection.h"
#include "kstdefaultnames.h"
#include "kstvectorview.h"
#include "kstdebug.h"

static const QString& IN_XVECTOR = KGlobal::staticQString("IN_XVECTOR");
static const QString& IN_YVECTOR = KGlobal::staticQString("IN_YVECTOR");
static const QString& IN_FLAGVECTOR = KGlobal::staticQString("IN_FLAGVECTOR");
static const QString& OUT_XVECTOR = KGlobal::staticQString("OUT_XVECTOR");
static const QString& OUT_YVECTOR = KGlobal::staticQString("OUT_YVECTOR");

KstVectorView::KstVectorView(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y, KstVectorView::InterpType itype, bool useXmin, KstScalarPtr xmin, bool useXmax, KstScalarPtr xmax, bool useYmin, KstScalarPtr ymin, bool useYmax, KstScalarPtr ymax, KstVectorPtr flag )
: KstDataObject() {
  _inputVectors[IN_XVECTOR] = in_X;
  _inputVectors[IN_YVECTOR] = in_Y;

  setInterp(itype);

  setUseXmin(useXmin);
  setUseXmax(useXmax);
  setUseYmin(useYmin);
  setUseYmax(useYmax);

  setXminScalar(xmin);
  setXmaxScalar(xmax);
  setYminScalar(ymin);
  setYmaxScalar(ymax);

  setFlagVector(flag);

  commonConstructor(in_tag);
}


KstVectorView::KstVectorView(const QDomElement &e)
: KstDataObject(e) {

  QString in_X_tag, in_Y_tag, in_flagtag;
  QString xmintag, xmaxtag, ymintag, ymaxtag;
  QString in_tag;

  _interp = KstVectorView::InterpY;
  _useXmin = false; 
  _useXmax = false;
  _useYmin = false;
  _useYmax = false;
  _xmin = NULL;
  _xmax = NULL;
  _ymin = NULL;
  _ymax = NULL;

  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "inxvectag") {
        in_X_tag = e.text();
      } else if (e.tagName() == "inyvectag") {
        in_Y_tag = e.text();
      } else if (e.tagName() == "flagtag") {
        in_flagtag = e.text();
      } else if (e.tagName() == "usexmin") {
        setUseXmin( e.text() != "0" );
      } else if (e.tagName() == "usexmax") {
        setUseXmax( e.text() != "0" );
      } else if (e.tagName() == "useymin") {
        setUseYmin( e.text() != "0" );
      } else if (e.tagName() == "useymax") {
        setUseYmax( e.text() != "0" );
      } else if (e.tagName() == "xmintag") {
        KstScalarPtr xmin;

        xmin = *KST::scalarList.findTag(e.text());
        if (xmin) { 
          setXminScalar(xmin); 
        }
      } else if (e.tagName() == "xmaxtag") {
        KstScalarPtr xmax;

        xmax = *KST::scalarList.findTag(e.text());
        if (xmax) { 
          setXmaxScalar(xmax); 
        }
      } else if (e.tagName() == "ymintag") {
        KstScalarPtr ymin;

        ymin = *KST::scalarList.findTag(e.text());
        if (ymin) { 
          setYminScalar(ymin); 
        }
      } else if (e.tagName() == "ymaxtag") {
        KstScalarPtr ymax;

        ymax = *KST::scalarList.findTag(e.text());
        if (ymax) { 
          setYmaxScalar(ymax); 
        }
      } else if (e.tagName() == "interp") {
        setInterp(KstVectorView::InterpType(e.text().toInt()));
      }
    }
    n = n.nextSibling();
  }

  //
  // assume these tags are always saved...
  //

  _inputVectorLoadQueue.append(qMakePair(IN_XVECTOR, in_X_tag)); 
  _inputVectorLoadQueue.append(qMakePair(IN_YVECTOR, in_Y_tag));

  if (!in_flagtag.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(IN_FLAGVECTOR, in_flagtag));
  }

  commonConstructor(in_tag);
}


void KstVectorView::commonConstructor(const QString &in_tag) {
  KstVectorPtr v;
  QString tagName;

  _typeString = QObject::tr("Vector View");
  _type = "VectorView";

  if (in_tag.isEmpty()) {
    if (_inputVectors[IN_XVECTOR]) {
      tagName = KST::suggestVectorViewName(_inputVectors[IN_XVECTOR]->tag());
    }
  } else {
    tagName = in_tag;
  }
  KstObject::setTagName(KstObjectTag::fromString(tagName));

  v = new KstVector(KstObjectTag("X", tag()), 0, this);
  _cxVector = _outputVectors.insert(OUT_XVECTOR, v);

  v = new KstVector(KstObjectTag("Y", tag()), 0, this);
  _cyVector = _outputVectors.insert(OUT_YVECTOR, v);

  setDirty();
}


KstVectorView::~KstVectorView() {
  _cxVector = _outputVectors.end();
  _cyVector = _outputVectors.end();
  KST::vectorList.lock().writeLock();
/* xxx
  KST::vectorList.remove(_outputVectors[OUT_XVECTOR]);
  KST::vectorList.remove(_outputVectors[OUT_YVECTOR]);
*/
  KST::vectorList.lock().unlock();
}


KstObject::UpdateType KstVectorView::update(int update_counter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();
  setDirty(false);

  if (recursed()) {
    return setLastUpdateResult(KstObject::NO_CHANGE);
  }

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  writeLockInputsAndOutputs();

  if (update_counter <= 0) {
    assert(update_counter == 0);
    force = true;
  }

  KstVectorPtr flagVec;
  bool vecsUpdated;;

  vecsUpdated = (KstObject::UPDATE == _inputVectors[IN_XVECTOR]->update(update_counter)) ||
                (KstObject::UPDATE == _inputVectors[IN_YVECTOR]->update(update_counter));

  flagVec = *_inputVectors.find(IN_FLAGVECTOR);
  if (flagVec) {
    vecsUpdated = (vecsUpdated || (KstObject::UPDATE == flagVec->update(update_counter)));
  }

  if (!vecsUpdated && !force) {
    unlockInputsAndOutputs();

    return setLastUpdateResult(KstObject::NO_CHANGE);
  }

  KstVectorPtr inXVec = _inputVectors[IN_XVECTOR];
  KstVectorPtr inYVec = _inputVectors[IN_YVECTOR];
  KstVectorPtr outXVec = _outputVectors[OUT_XVECTOR];
  KstVectorPtr outYVec = _outputVectors[OUT_YVECTOR];
  double xmin = inXVec->min();
  double xmax = inXVec->max();
  double ymin = inYVec->min();
  double ymax = inYVec->max();

  if (_useXmin && _xmin) {
    xmin = _xmin->value();
  }

  if (_useXmax && _xmax) {
    xmax = _xmax->value();
  }

  if (_useYmin && _ymin) {
    ymin = _ymin->value();
  }

  if (_useYmax && _ymax) {
    ymax = _ymax->value();
  }

  int NS;

  switch (interp()) {
    case InterpMax:
      NS = qMax(inXVec->length(), inYVec->length());
      break;

    case InterpMin:
      NS = qMin(inXVec->length(), inYVec->length());
      break;

    case InterpX:
      NS = inXVec->length();
      break;

    case InterpY:
      NS = inYVec->length();
      break;

    default:
      NS = qMax(inXVec->length(), inYVec->length());
      break;
  }

  double *outXArr;
  double *outYArr;
  int NSm1 = NS-1;

  if (inXVec->length() == NS && inXVec->isRising()) { //good scenario.
    int i_bot = inXVec->indexNearX(xmin, NS); //closest index to xmin
    int i_top = inXVec->indexNearX(xmax, NS);

    if (inXVec->value(i_bot) < xmin && i_bot < NSm1) { 
      i_bot++;
    }

    if (inXVec->value(i_top) > xmax && i_top > 0) { 
      i_top--;
    }

    if (i_top - i_bot + 1 > outXVec->length()) {
      outXVec->resize(i_top - i_bot + 1, false); //initial resize. will trim later
    }

    if (i_top - i_bot + 1 > outYVec->length()) {
      outYVec->resize(i_top - i_bot + 1, false);
    }

    outXArr = outXVec->value();
    outYArr = outYVec->value();

    double *inXArr = inXVec->value();

    int in = 0;
    double yv;

    for (long i=i_bot; i<=i_top; i++) {
        if (!flagVec || !flagVec->interpolate(i, NS)) {
          yv = inYVec->interpolate(i, NS);
          if (ymin <= yv && ymax >= yv) {
            outXArr[in] = inXArr[i];
            outYArr[in] = yv;
            in++;
          }
        }
    }

    if (in > 0) {
      outXVec->resize(in, false);
      outYVec->resize(in, false);
    } else {
      outXArr[0] = 0.0;
      outYArr[0] = 0.0;
      outXVec->resize(1, false);
      outYVec->resize(1, false);
    }
  } else if (inYVec->length() == NS && inYVec->isRising()) { //also good.
    int i_bot = inYVec->indexNearX(ymin, NS); //closest index to xmin
    int i_top = inYVec->indexNearX(ymax, NS);

    if (inYVec->value(i_bot) < ymin && i_bot < NSm1) { 
      i_bot++;
    } // closest index above xmin

    if (inYVec->value(i_top) > ymax && i_top > 0) { 
      i_top--;
    }

    if (i_top - i_bot + 1 > outXVec->length()) {
      outXVec->resize(i_top - i_bot + 1, false); //initial resize. will trim later.
    }

    if (i_top - i_bot + 1 > outYVec->length()) {
      outYVec->resize(i_top - i_bot + 1, false);
    }

    outXArr = outXVec->value();
    outYArr = outYVec->value();

    double *inYArr = inYVec->value();

    int in = 0;
    double xv;

    for (long i=i_bot; i<=i_top; i++) {
        if (!flagVec || !flagVec->interpolate(i, NS)) {
          xv = inXVec->interpolate(i, NS);
          if (xmin <= xv && xmax >= xv) {
            outXArr[in] = inYArr[i];
            outYArr[in] = xv;
            in++;
          }
        }
    }

    if (in > 0) {
      outXVec->resize(in, false);
      outYVec->resize(in, false);
    } else {
      outXArr[0] = 0.0;
      outYArr[0] = 0.0;
      outXVec->resize(1, false);
      outYVec->resize(1, false);
    }
  } else { //worst case scenario. could do more ifs on y->isRising();
    if (NS > outXVec->length()) {
      outXVec->resize(NS, false); // initial resize. will trim later.
    }
    if (NS > outYVec->length()) {
      outYVec->resize(NS, false);
    }

    outXArr = outXVec->value();
    outYArr = outYVec->value();

    int in = 0;
    double yv;

    for (long i=0; i<=NSm1; i++) {
      if (!flagVec || !flagVec->interpolate(i, NS)) {
        double xv = inXVec->interpolate(i, NS);
        if (xmin <= xv && xmax >= xv) {
          yv = inYVec->interpolate(i,NS);
          if (ymin <= yv && ymax >= yv) {
            outXArr[in] = xv;
            outYArr[in] = yv;
            in++;
          }
        }
      }
    }

    if (in > 0) {
      outXVec->resize(in, false);
      outYVec->resize(in, false);
    } else {
      outXArr[0] = 0.0;
      outYArr[0] = 0.0;
      outXVec->resize(1, false);
      outYVec->resize(1, false);
    }
  }

  unlockInputsAndOutputs();

  return setLastUpdateResult(UPDATE);
}


void KstVectorView::setXminScalar(KstScalarPtr xmin) {
  if (xmin != _xmin) {
    if (_xmin) {
      _xmin->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }

    _xmin = xmin;
    if (xmin && _useXmin) {
      connect(xmin.data(), SIGNAL(trigger()), SLOT(scalarChanged()));
    }
  }
}


void KstVectorView::setXmaxScalar(KstScalarPtr xmax) {
  if (xmax != _xmax) {
    if (_xmax) {
      _xmax->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }

    _xmax = xmax;
    if (xmax && _useXmax) {
      connect(xmax.data(), SIGNAL(trigger()), SLOT(scalarChanged()));
    }
  }
}


void KstVectorView::setYminScalar(KstScalarPtr ymin) {
  if (ymin != _ymin) {
    if (_ymin) {
      _ymin->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }

    _ymin = ymin;
    if (ymin && _useYmin) {
      connect(ymin.data(), SIGNAL(trigger()), SLOT(scalarChanged()));
    }
  }
}


void KstVectorView::setYmaxScalar(KstScalarPtr ymax) {
  if (ymax != _ymax) {
    if (_ymax) {
      _ymax->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }

    _ymax = ymax;
    if (ymax && _useYmax) {
      connect(ymax.data(), SIGNAL(trigger()), SLOT(scalarChanged()));
    }
  }
}


void KstVectorView::setUseXmin(bool useXmin) {
  _useXmin = useXmin;

  if (!_useXmin) {
    if (_xmin) {
      _xmin->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }
  }
  if (_useXmin && _xmin) {
    connect(_xmin.data(), SIGNAL(trigger()), SLOT(scalarChanged())); 
  }
}


void KstVectorView::setUseXmax(bool useXmax) {
  _useXmax = useXmax;

  if (!_useXmax) { 
    if (_xmax) {
      _xmax->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }
  }

  if (_useXmax && _xmax) { 
    connect(_xmax.data(), SIGNAL(trigger()), SLOT(scalarChanged())); 
  }
}


void KstVectorView::setUseYmin(bool useYmin) {
  _useYmin = useYmin;

  if (!_useYmin) {
    if (_ymin) {
      _ymin->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }
  }

  if (_useYmin && _ymin) { 
    connect(_ymin.data(), SIGNAL(trigger()), SLOT(scalarChanged())); 
  }
}


void KstVectorView::setUseYmax(bool useYmax) {
  _useYmax = useYmax;

  if (!_useYmax) { 
    if (_ymax) {
      _ymax->disconnect(SIGNAL(trigger()), this, SLOT(scalarChanged()));
    }
  }

  if (_useYmax && _ymax) { 
    connect(_ymax.data(), SIGNAL(trigger()), SLOT(scalarChanged())); 
  }
}


void KstVectorView::setXVector(KstVectorPtr new_v) {
  _inputVectors[IN_XVECTOR] = new_v;
}


void KstVectorView::setYVector(KstVectorPtr new_v) {
  _inputVectors[IN_YVECTOR] = new_v;
}


void KstVectorView::setFlagVector(KstVectorPtr flag) {
  if (flag) {
    _inputVectors[IN_FLAGVECTOR] = flag;
  } else {
    _inputVectors.remove(IN_FLAGVECTOR);
  }
  setDirty();
}


bool KstVectorView::hasFlag() {
  return _inputVectors.contains(IN_FLAGVECTOR);
}


QString KstVectorView::xLabel() const {
  return _inputVectors[IN_XVECTOR]->label();
}


QString KstVectorView::yLabel() const {
  return _inputVectors[IN_YVECTOR]->label();
}


QString KstVectorView::in_xVTag() const {
  return _inputVectors[IN_XVECTOR]->tag().displayString();
}


QString KstVectorView::in_yVTag() const {
  return _inputVectors[IN_YVECTOR]->tag().displayString();
}


QString KstVectorView::FlagTag() const {
  return _inputVectors[IN_FLAGVECTOR]->tag().displayString();
}


void KstVectorView::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";

  ts << indent << "<vectorview>" << endl;
  ts << l2 << "<tag>" << Qt::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<inxvectag>" << Qt::escape(_inputVectors[IN_XVECTOR]->tag().tagString()) << "</inxvectag>" << endl;
  ts << l2 << "<inyvectag>" << Qt::escape(_inputVectors[IN_YVECTOR]->tag().tagString()) << "</inyvectag>" << endl;
  ts << l2 << "<interp>" << interp() << "</interp>" << endl;
  ts << l2 << "<usexmin>" << _useXmin << "</usexmin>" << endl;
  ts << l2 << "<xmintag>" << Qt::escape(_xmin->tag().displayString()) << "</xmintag>" << endl;
  ts << l2 << "<usexmax>" << _useXmax << "</usexmax>" << endl;
  ts << l2 << "<xmaxtag>" << Qt::escape(_xmax->tag().displayString()) << "</xmaxtag>" << endl;
  ts << l2 << "<useymin>" << _useYmin << "</useymin>" << endl;
  ts << l2 << "<ymintag>" << Qt::escape(_ymin->tag().displayString()) << "</ymintag>" << endl;
  ts << l2 << "<useymax>" << _useYmax << "</useymax>" << endl;
  ts << l2 << "<ymaxtag>" << Qt::escape(_ymax->tag().displayString()) << "</ymaxtag>" << endl;
  if (_inputVectors.contains(IN_FLAGVECTOR)) { 
    ts << l2 << "<flagtag>" << Qt::escape(_inputVectors[IN_FLAGVECTOR]->tag().displayString()) << "</flagtag>" << endl; 
  }
  ts << indent << "</vectorview>" << endl;
}


QString KstVectorView::propertyString() const {
  return QObject::tr("VectorView: %1 vs %2").arg(xVTag(),yVTag());
}


void KstVectorView::showNewDialog() {
  KstDialogs::self()->showVectorViewDialog();
}


void KstVectorView::showEditDialog() {
  KstDialogs::self()->showVectorViewDialog(tagName(), true);
}


bool KstVectorView::slaveVectorsUsed() const {
  return true;
}


KstDataObjectPtr KstVectorView::makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) {
  QString name(tagName() + '\'');
  KstVectorViewPtr vectorview;

  while (KstData::self()->dataTagNameNotUnique(name, false)) {
    name += '\'';
  }
  vectorview = new KstVectorView(name, _inputVectors[IN_XVECTOR], _inputVectors[IN_YVECTOR], interp(), _useXmin, xMinScalar(), _useXmax, xMaxScalar(), _useYmin, yMinScalar(), _useYmax, yMaxScalar(), _inputVectors[IN_FLAGVECTOR]);

  duplicatedMap.insert(KstVectorViewPtr(this), KstDataObjectPtr(vectorview));

  return KstDataObjectPtr(vectorview);
}


void KstVectorView::scalarChanged() {
    setDirty();
}


KstVectorView::InterpType KstVectorView::interp() const {
  return (_interp);
}


void KstVectorView::setInterp(KstVectorView::InterpType itype) {
  _interp = itype;
  setDirty();
}


void KstVectorView::setTagName(const QString &in_tag) {
  KstObjectTag newTag(in_tag, tag().context());

  if (newTag == tag()) {
    return;
  }

  KstObject::setTagName(newTag);
  (*_cxVector)->setTagName(KstObjectTag("X", tag()));
  (*_cyVector)->setTagName(KstObjectTag("Y", tag()));
}

#include "kstvectorview.moc"
