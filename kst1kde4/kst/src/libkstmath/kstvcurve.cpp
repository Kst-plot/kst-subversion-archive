/***************************************************************************
                          kstcurve.cpp: holds info for a curve
                             -------------------
    begin                : Fri Nov 3 2000
    copyright            : (C) 2000 by cbn
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

#include <time.h>

#include <QTextDocument>

#include "dialoglauncher.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstdefaultnames.h"
#include "kstlinestyle.h"
#include "kstmath.h"
#include "kstrvector.h"
#include "kstvcurve.h"

#ifndef KDE_IS_LIKELY
#if __GNUC__ - 0 >= 3
# define KDE_ISLIKELY( x )    __builtin_expect(!!(x),1)
# define KDE_ISUNLIKELY( x )  __builtin_expect(!!(x),0)
#else
# define KDE_ISLIKELY( x )   ( x )
# define KDE_ISUNLIKELY( x )  ( x )
#endif
#endif

// for painting
#define MAX_NUM_POLYLINES       1000

static const QString& COLOR_XVECTOR = KGlobal::staticQString("X");
static const QString& COLOR_YVECTOR = KGlobal::staticQString("Y");
static const QString& EXVECTOR = KGlobal::staticQString("EX");
static const QString& EYVECTOR = KGlobal::staticQString("EY");
static const QString& EXMINUSVECTOR = KGlobal::staticQString("EXMinus");
static const QString& EYMINUSVECTOR = KGlobal::staticQString("EYMinus");
static const QString& YOFFSETSCALAR = KGlobal::staticQString("YOffset");

KstVCurve::KstVCurve(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y,
                      KstVectorPtr in_EX, KstVectorPtr in_EY,
                      KstVectorPtr in_EXMinus, KstVectorPtr in_EYMinus,
                      const QColor &in_color)
: KstBaseCurve() {
  setHasPoints(false);
  setHasBars(false);
  setHasLines(true);
  setLineWidth(1);
  setLineStyle(0);
  setBarStyle(0);
  setPointDensity(0);
  setPointStyle(0);
  setInterp(InterpY);

  if (in_X) {
    _inputVectors[COLOR_XVECTOR] = in_X;
  }

  if (in_Y) {
    _inputVectors[COLOR_YVECTOR] = in_Y;
  }

  if (in_EX) {
    _inputVectors[EXVECTOR] = in_EX;
  }

  if (in_EY) {
    _inputVectors[EYVECTOR] = in_EY;
  }

  if (in_EXMinus) {
    _inputVectors[EXMINUSVECTOR] = in_EXMinus;
  }

  if (in_EYMinus) {
    _inputVectors[EYMINUSVECTOR] = in_EYMinus;
  }

  commonConstructor(in_tag, in_color);
  setDirty();
}


KstVCurve::KstVCurve(QDomElement &e)
: KstBaseCurve(e) {
  QString in_tag;
  QString xname, yname;
  QString exname, eyname;
  QString exminusname, eyminusname;
  QString yVectorOffsetScalarName;
  QColor in_color("red");
  bool hasMinus = false;

  setHasPoints(false);
  setHasLines(false);
  setHasBars(false);
  setLineWidth(1);
  setLineStyle(0);
  setBarStyle(0);
  setPointDensity(0);
  setInterp(InterpMax); // pre 1.4.0 files use old default.

  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if (!e.isNull()) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "hasMinus") {
        hasMinus = true;
      } else if (e.tagName() == "xvectag") {
        xname = e.text();
      } else if (e.tagName() == "yvectag") {
        yname = e.text();
      } else if (e.tagName() == "exVectag") {
        exname = e.text();
        if (!hasMinus) {
          exminusname = e.text();
        }
      } else if (e.tagName() == "eyVectag") {
        eyname = e.text();
        if (!hasMinus) {
          eyminusname = e.text();
        }
      } else if (e.tagName() == "exMinusVectag") {
        exminusname = e.text();
      } else if (e.tagName() == "eyMinusVectag") {
        eyminusname = e.text();
      } else if (e.tagName() == "color") {
        in_color.setNamedColor(e.text());
      } else if (e.tagName() == "legend") {
        setLegendText(e.text());
      // the following options are only needed to change from the default
      } else if (e.tagName() == "hasLines") {
        _hasLines = e.text() != "0";
      } else if (e.tagName() == "hasPoints") {
        _hasPoints = e.text() != "0";
      } else if (e.tagName() == "hasBars") {
        _hasBars = e.text() != "0";
      } else if (e.tagName() == "pointType") {
        _pointStyle = e.text().toInt();
      } else if (e.tagName() == "lineWidth") {
        _lineWidth = e.text().toInt();
      } else if (e.tagName() == "lineStyle") {
        _lineStyle = e.text().toInt();
      } else if (e.tagName() == "barStyle") {
        _barStyle = e.text().toInt();
      } else if (e.tagName() == "pointDensity") {
        _pointDensity = e.text().toInt();
      } else if (e.tagName() == "ignoreAutoScale") {
        _ignoreAutoScale = true;
      } else if (e.tagName() == "yVectorOffset") {
        _yVectorOffset = true;
      } else if (e.tagName() == "yVectorOffsetScalar") {
         yVectorOffsetScalarName = e.text();
      } else if (e.tagName() == "interp") {
        _interp = KstVCurve::InterpType(e.text().toInt());
      }
    }
    n = n.nextSibling();
  }

  if (!xname.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(COLOR_XVECTOR, xname));
  }
  if (!yname.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(COLOR_YVECTOR, yname));
  }
  if (!exname.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(EXVECTOR, exname));
  }
  if (!eyname.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(EYVECTOR, eyname));
  }
  if (!exminusname.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(EXMINUSVECTOR, exminusname));
  }
  if (!eyminusname.isEmpty()) {
    _inputVectorLoadQueue.append(qMakePair(EYMINUSVECTOR, eyminusname));
  }
  if (!yVectorOffsetScalarName.isEmpty()) {
    _inputScalarLoadQueue.append(qMakePair(YOFFSETSCALAR, yVectorOffsetScalarName));
  }

  commonConstructor(in_tag, in_color);
}


void KstVCurve::commonConstructor(const QString &in_tag, const QColor &in_color) {
  _maxX = _minX = _meanX = _maxY = _minY = _meanY = _minPosX = _minPosY = 0.0;
  NS = 0;
  _typeString = QObject::tr("Curve");
  _type = "Curve";
  _color = in_color;
  if (in_tag == QString::null) {
    QString tag_name = KST::suggestCurveName(yVTag());
    setTagName(KstObjectTag::fromString(tag_name));
  } else {
    setTagName(KstObjectTag::fromString(in_tag));
  }
  updateParsedLegendTag();
}


KstVCurve::~KstVCurve() {
}


KstObject::UpdateType KstVCurve::update(int update_counter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  KstVectorPtr cxV = *_inputVectors.find(COLOR_XVECTOR);
  KstVectorPtr cyV = *_inputVectors.find(COLOR_YVECTOR);
  if (!cxV || !cyV) {
    return setLastUpdateResult(NO_CHANGE);
  }

  writeLockInputsAndOutputs();

  bool depUpdated = force;

  depUpdated = UPDATE == cxV->update(update_counter) || depUpdated;
  depUpdated = UPDATE == cyV->update(update_counter) || depUpdated;

  KstVectorPtr exV = *_inputVectors.find(EXVECTOR);
  if (exV) {
    depUpdated = UPDATE == exV->update(update_counter) || depUpdated;
  }

  KstVectorPtr eyV = *_inputVectors.find(EYVECTOR);
  if (eyV) {
    depUpdated = UPDATE == eyV->update(update_counter) || depUpdated;
  }

  KstVectorPtr exmV = *_inputVectors.find(EXMINUSVECTOR);
  if (exmV) {
    depUpdated = UPDATE == exmV->update(update_counter) || depUpdated;
  }

  KstVectorPtr eymV = *_inputVectors.find(EYMINUSVECTOR);
  if (eymV) {
    depUpdated = UPDATE == eymV->update(update_counter) || depUpdated;
  }

  _maxX = cxV->max();
  _minX = cxV->min();
  _meanX = cxV->mean();
  _minPosX = cxV->minPos();
  _ns_maxx = cxV->ns_max();
  _ns_minx = cxV->ns_min();

  if (_minPosX > _maxX) {
    _minPosX = 0.0;
  }
  _maxY = cyV->max();
  _minY = cyV->min();
  _meanY = cyV->mean();
  _minPosY = cyV->minPos();
  _ns_maxy = cyV->ns_max();
  _ns_miny = cyV->ns_min();

  if (_minPosY > _maxY) {
    _minPosY = 0;
  }

  switch (interp()) {
    case InterpMax:
      NS = qMax(cxV->length(), cyV->length());
      break;
    case InterpMin:
      NS = qMin(cxV->length(), cyV->length());
      break;
    case InterpX:
      NS = cxV->length();
      break;
    case InterpY:
      NS = cyV->length();
      break;
    default:
      NS = qMax(cxV->length(), cyV->length());
  }

  unlockInputsAndOutputs();

  return setLastUpdateResult(depUpdated ? UPDATE : NO_CHANGE);
}


void KstVCurve::point(int i, double &x, double &y) const {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
}


void KstVCurve::getEXPoint(int i, double &x, double &y, double &ex) {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
  KstVectorPtr exv = xErrorVector();
  if (exv) {
    ex = exv->interpolate(i, NS);
  }
}


void KstVCurve::getEXMinusPoint(int i, double &x, double &y, double &ex) {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
  KstVectorPtr exmv = xMinusErrorVector();
  if (exmv) {
    ex = exmv->interpolate(i, NS);
  }
}


void KstVCurve::getEXPoints(int i, double &x, double &y, double &exminus, double &explus) {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
  KstVectorPtr exv = xErrorVector();
  if (exv) {
    explus = exv->interpolate(i, NS);
  }
  KstVectorPtr exmv = xMinusErrorVector();
  if (exmv) {
    exminus = exmv->interpolate(i, NS);
  }
}


void KstVCurve::getEYPoint(int i, double &x, double &y, double &ey) {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
  KstVectorPtr eyv = yErrorVector();
  if (eyv) {
    ey = eyv->interpolate(i, NS);
  }
}


void KstVCurve::getEYMinusPoint(int i, double &x, double &y, double &ey) {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
  KstVectorPtr eyv = yMinusErrorVector();
  if (eyv) {
    ey = eyv->interpolate(i, NS);
  }
}


void KstVCurve::getEYPoints(int i, double &x, double &y, double &eyminus, double &eyplus) {
  KstVectorPtr xv = xVector();
  if (xv) {
    x = xv->interpolate(i, NS);
  }
  KstVectorPtr yv = yVector();
  if (yv) {
    y = yv->interpolate(i, NS);
  }
  KstVectorPtr eyv = yErrorVector();
  if (eyv) {
    eyplus = eyv->interpolate(i, NS);
  }
  KstVectorPtr eymv = yMinusErrorVector();
  if (eymv) {
    eyminus = eymv->interpolate(i, NS);
  }
}


KstObjectTag KstVCurve::xVTag() const {
  KstVectorPtr xv = xVector();
  if (xv) {
    return xv->tag();
  }
  return KstObjectTag::invalidTag;
}


KstObjectTag KstVCurve::yVTag() const {
  KstVectorPtr yv = yVector();
  if (yv) {
    return yv->tag();
  }
  return KstObjectTag::invalidTag;
}


KstObjectTag KstVCurve::xETag() const {
  KstVectorPtr v = xErrorVector();
  if (v) {
    return v->tag();
  }
  return KstObjectTag::invalidTag;
}


KstObjectTag KstVCurve::yETag() const {
  KstVectorPtr v = yErrorVector();
  if (v) {
    return v->tag();
  }
  return KstObjectTag::invalidTag;
}


KstObjectTag KstVCurve::xEMinusTag() const {
  KstVectorPtr v = xMinusErrorVector();
  if (v) {
    return v->tag();
  }
  return KstObjectTag::invalidTag;
}


KstObjectTag KstVCurve::yEMinusTag() const {
  KstVectorPtr v = yMinusErrorVector();
  if (v) {
    return v->tag();
  }
  return KstObjectTag::invalidTag;
}


KstObjectTag KstVCurve::yVectorOffsetTag() const {
  KstScalarPtr s = yVectorOffsetScalar();
  if (s) {
    return s->tag();
  }
  return KstObjectTag::invalidTag;
}


bool KstVCurve::hasXError() const {
  return _inputVectors.contains(EXVECTOR);
}


bool KstVCurve::hasYError() const {
  return _inputVectors.contains(EYVECTOR);
}


bool KstVCurve::hasXMinusError() const {
  return _inputVectors.contains(EXMINUSVECTOR);
}


bool KstVCurve::hasYMinusError() const {
  return _inputVectors.contains(EYMINUSVECTOR);
}


void KstVCurve::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<curve>" << endl;
  ts << l2 << "<tag>" << Qt::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<xvectag>" << Qt::escape(_inputVectors[COLOR_XVECTOR]->tag().tagString()) << "</xvectag>" << endl;
  ts << l2 << "<yvectag>" << Qt::escape(_inputVectors[COLOR_YVECTOR]->tag().tagString()) << "</yvectag>" << endl;
  ts << l2 << "<legend>" << Qt::escape(legendText()) << "</legend>" << endl;
  ts << l2 << "<interp>" << interp() << "</interp>" << endl;
  ts << l2 << "<hasMinus/>" << endl;
  if (_inputVectors.contains(EXVECTOR)) {
    ts << l2 << "<exVectag>" << Qt::escape(_inputVectors[EXVECTOR]->tag().tagString()) << "</exVectag>" << endl;
  }
  if (_inputVectors.contains(EYVECTOR)) {
    ts << l2 << "<eyVectag>" << Qt::escape(_inputVectors[EYVECTOR]->tag().tagString()) << "</eyVectag>" << endl;
  }
  if (_inputVectors.contains(EXMINUSVECTOR)) {
    ts << l2 << "<exMinusVectag>" << Qt::escape(_inputVectors[EXMINUSVECTOR]->tag().tagString()) << "</exMinusVectag>" << endl;
  }
  if (_inputVectors.contains(EYMINUSVECTOR)) {
    ts << l2 << "<eyMinusVectag>" << Qt::escape(_inputVectors[EYMINUSVECTOR]->tag().tagString()) << "</eyMinusVectag>" << endl;
  }
  ts << l2 << "<color>" << _color.name() << "</color>" << endl;
  if (_hasLines) {
    ts << l2 << "<hasLines/>" << endl;
  }
  ts << l2 << "<lineWidth>" << _lineWidth << "</lineWidth>" << endl;
  ts << l2 << "<lineStyle>" << _lineStyle << "</lineStyle>" << endl;
  if (_hasPoints) {
    ts << l2 << "<hasPoints/>" << endl;
  }
  ts << l2 << "<pointType>" << _pointStyle << "</pointType>" << endl;
  ts << l2 << "<pointDensity>" << _pointDensity << "</pointDensity>" << endl;
  if (_hasBars) {
    ts << l2 << "<hasBars/>" << endl;
  }
  ts << l2 << "<barStyle>" << _barStyle << "</barStyle>" << endl;
  if (_ignoreAutoScale) {
    ts << l2 << "<ignoreAutoScale/>" << endl;
  }
  if (_yVectorOffset) {
    ts << l2 << "<yVectorOffset/>" << endl;
    ts << l2 << "<yVectorOffsetScalar>" << Qt::escape(_inputScalars[YOFFSETSCALAR]->tag().tagString()) << "</yVectorOffsetScalar>" << endl;
  }
  ts << indent << "</curve>" << endl;
}


void KstVCurve::setXVector(KstVectorPtr new_vx) {
  if (new_vx) {
    _inputVectors[COLOR_XVECTOR] = new_vx;
  } else {
    _inputVectors.remove(COLOR_XVECTOR);
  }
  setDirty();
}


void KstVCurve::setYVector(KstVectorPtr new_vy) {
  if (new_vy) {
    _inputVectors[COLOR_YVECTOR] = new_vy;
  } else {
    _inputVectors.remove(COLOR_YVECTOR);
  }
  setDirty();
}


void KstVCurve::setXError(KstVectorPtr new_ex) {
  if (new_ex) {
    _inputVectors[EXVECTOR] = new_ex;
  } else {
    _inputVectors.remove(EXVECTOR);
  }
  setDirty();
}


void KstVCurve::setYError(KstVectorPtr new_ey) {
  if (new_ey) {
    _inputVectors[EYVECTOR] = new_ey;
  } else {
    _inputVectors.remove(EYVECTOR);
  }
  setDirty();
}


void KstVCurve::setXMinusError(KstVectorPtr new_ex) {
  if (new_ex) {
    _inputVectors[EXMINUSVECTOR] = new_ex;
  } else {
    _inputVectors.remove(EXMINUSVECTOR);
  }
  setDirty();
}


void KstVCurve::setYMinusError(KstVectorPtr new_ey) {
  if (new_ey) {
    _inputVectors[EYMINUSVECTOR] = new_ey;
  } else {
    _inputVectors.remove(EYMINUSVECTOR);
  }
  setDirty();
}


QString KstVCurve::xLabel() const {
  return _inputVectors[COLOR_XVECTOR]->label();
}


QString KstVCurve::yLabel() const {
  return _inputVectors[COLOR_YVECTOR]->label();
}


QString KstVCurve::topLabel() const {
  return QString::null;
  //return VY->fileLabel();
}


KstCurveType KstVCurve::curveType() const {
  return KST_VCURVE;
}


QString KstVCurve::propertyString() const {
  return QObject::tr("%1 vs %2").arg(yVTag().displayString()).arg(xVTag().displayString());
}


void KstVCurve::showNewDialog() {
  KstDialogs::self()->showCurveDialog();
}


void KstVCurve::showEditDialog() {
  KstDialogs::self()->showCurveDialog(tagName(), true);
}


int KstVCurve::samplesPerFrame() const {
  const KstRVector *rvp = dynamic_cast<const KstRVector*>(_inputVectors[COLOR_YVECTOR].data());
  return rvp ? rvp->samplesPerFrame() : 1;
}


KstVectorPtr KstVCurve::xVector() const {
  return *_inputVectors.find(COLOR_XVECTOR);
}


KstVectorPtr KstVCurve::yVector() const {
  return *_inputVectors.find(COLOR_YVECTOR);
}


KstVectorPtr KstVCurve::xErrorVector() const {
  return *_inputVectors.find(EXVECTOR);
}


KstVectorPtr KstVCurve::yErrorVector() const {
  return *_inputVectors.find(EYVECTOR);
}


KstVectorPtr KstVCurve::xMinusErrorVector() const {
  return *_inputVectors.find(EXMINUSVECTOR);
}


KstVectorPtr KstVCurve::yMinusErrorVector() const {
  return *_inputVectors.find(EYMINUSVECTOR);
}


KstScalarPtr KstVCurve::yVectorOffsetScalar() const {
  return *_inputScalars.find(YOFFSETSCALAR);
}


bool KstVCurve::xIsRising() const {
  return _inputVectors[COLOR_XVECTOR]->isRising();
}


inline int indexNearX(double x, KstVectorPtr& xv, int NS) {
  // monotonically rising: we can do a binary search
  // should be reasonably fast
  if (xv->isRising()) {
    int i_top = NS - 1;
    int i_bot = 0;

    // don't pre-check for x outside of the curve since this is not
    // the common case.  It will be correct - just slightly slower...
    while (i_bot + 1 < i_top) {
      int i0 = (i_top + i_bot)/2;
      double rX = xv->interpolate(i0, NS);
      if (x < rX) {
        i_top = i0;
      } else {
        i_bot = i0;
      }
    }
    double xt = xv->interpolate(i_top, NS);
    double xb = xv->interpolate(i_bot, NS);
    if (xt - x < x - xb) {
      return i_top;
    } else {
      return i_bot;
    }
  } else {
    // Oh Oh... not monotonically rising - we have to search the entire curve!
    // May be unbearably slow for large vectors
    double rX = xv->interpolate(0, NS);
    double dx0 = fabs(x - rX);
    int i0 = 0;

    for (int i = 1; i < NS; ++i) {
      rX = xv->interpolate(i, NS);
      double dx = fabs(x - rX);
      if (dx < dx0) {
        dx0 = dx;
        i0 = i;
      }
    }
    return i0;
  }
}


/** getIndexNearXY: return index of point within (or closest too)
    x +- dx which is closest to y **/
int KstVCurve::getIndexNearXY(double x, double dx_per_pix, double y) const {
  KstVectorPtr xv = *_inputVectors.find(COLOR_XVECTOR);
  KstVectorPtr yv = *_inputVectors.find(COLOR_YVECTOR);
  if (!xv || !yv) {
    return 0; // anything better we can do?
  }

  double xi, yi, dx, dxi, dy, dyi;
  bool first = true;
  int i,i0, iN, index;
  int sc = sampleCount();

  if (xv->isRising()) {
    iN = i0 = indexNearX(x, xv, NS);

    xi = xv->interpolate(i0, NS);
    while (i0 > 0 && x-dx_per_pix < xi) {
      xi = xv->interpolate(--i0, NS);
    }

    xi = xv->interpolate(iN, NS);
    while (iN < sc-1 && x+dx_per_pix > xi) {
      xi = xv->interpolate(++iN, NS);
    }
  } else {
    i0 = 0;
    iN = sampleCount()-1;
  }

  index = i0;
  xi = xv->interpolate(index, NS);
  yi = yv->interpolate(index, NS);
  dx = fabs(x - xi);
  dy = fabs(y - yi);

  for (i = i0 + 1; i <= iN; i++) {
    xi = xv->interpolate(i, NS);
    dxi = fabs(x - xi);
    if (dxi < dx_per_pix) {
      dx = dxi;
      yi = yv->interpolate(i, NS);
      dyi = fabs(y - yi);
      if (first || dyi < dy) {
        first = false;
        index = i;
        dy = dyi;
      }
    } else if (dxi < dx) {
      dx = dxi;
      index = i;
    }
  }

  return index;
}


void KstVCurve::setHasPoints(bool in_HasPoints) {
  _hasPoints = in_HasPoints;
  setDirty();
  emit modifiedLegendEntry();
}


void KstVCurve::setHasLines(bool in_HasLines) {
  _hasLines = in_HasLines;
  setDirty();
  emit modifiedLegendEntry();
}


void KstVCurve::setHasBars(bool in_HasBars) {
  _hasBars = in_HasBars;
  setDirty();
  emit modifiedLegendEntry();
}


void KstVCurve::setLineWidth(int in_LineWidth) {
  _lineWidth = in_LineWidth;
  setDirty();
  emit modifiedLegendEntry();
}


void KstVCurve::setLineStyle(int in_LineStyle) {
  if (in_LineStyle >= 0 && (unsigned int)in_LineStyle < KSTLINESTYLE_MAXTYPE) {
    _lineStyle = in_LineStyle;
    setDirty();
    emit modifiedLegendEntry();
  }
}


void KstVCurve::setBarStyle(int in_BarStyle) {
  _barStyle = in_BarStyle;
  setDirty();
  emit modifiedLegendEntry();
}


void KstVCurve::setPointDensity(int in_PointDensity) {
  if (in_PointDensity >= 0 && (unsigned int)in_PointDensity < KSTPOINTDENSITY_MAXTYPE) {
    _pointDensity = in_PointDensity;
    setDirty();
  }
}


void KstVCurve::setPointStyle(int in_PointStyle) {
  if (in_PointStyle >= 0 && (unsigned int)in_PointStyle < KSTPOINT_MAXTYPE) {
    _pointStyle = in_PointStyle;
    setDirty();
    emit modifiedLegendEntry();
  }
}


void KstVCurve::setColor(const QColor& new_c) {
  setDirty();
  _color = new_c;
  emit modifiedLegendEntry();
}


double KstVCurve::maxX() const {
  if (hasBars() && sampleCount() > 0) {
    return _maxX + (_maxX - _minX)/(2*(sampleCount()-1));
  }
  return _maxX;
}


double KstVCurve::minX() const {
  if (hasBars() && sampleCount() > 0) {
    return _minX - (_maxX - _minX)/(2*(sampleCount()-1));
  }
  return _minX;
}


void KstVCurve::setYVectorOffsetScalar(const KstScalarPtr& scalar) {
  if (scalar) {
    _inputScalars[YOFFSETSCALAR] = scalar;
  } else {
    _inputScalars.remove(YOFFSETSCALAR);
  }
  setDirty();
}


double KstVCurve::yVectorOffsetValue() const {
  double value = 0.0;

  if (_yVectorOffset) {
    KstScalarPtr scalar;

    scalar = *_inputScalars.find(YOFFSETSCALAR);
    if (scalar) {
      value = -scalar->value();
    }
  }

  return value;
}


KstDataObjectPtr KstVCurve::makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap) {
  KstVectorPtr VX = *_inputVectors.find(COLOR_XVECTOR);
  KstVectorPtr VY = *_inputVectors.find(COLOR_YVECTOR);
  KstVectorPtr EX = *_inputVectors.find(EXVECTOR);
  KstVectorPtr EY = *_inputVectors.find(EYVECTOR);
  KstVectorPtr EXMinus = *_inputVectors.find(EXMINUSVECTOR);
  KstVectorPtr EYMinus = *_inputVectors.find(EYMINUSVECTOR);

  QString name(tagName() + '\'');

  while (KstData::self()->dataTagNameNotUnique(name, false)) {
    name += '\'';
  }

  KstVCurvePtr vcurve(new KstVCurve(name, VX, VY, EX, EY, EXMinus, EYMinus, _color));

  //
  // copy some other properties as well
  //
  vcurve->setHasPoints(_hasPoints);
  vcurve->setHasLines(_hasLines);
  vcurve->setHasBars(_hasBars);
  vcurve->setBarStyle(_barStyle);
  vcurve->setLineWidth(_lineWidth);
  vcurve->setLineStyle(_lineStyle);
  vcurve->setPointDensity(_pointDensity);
  vcurve->setIgnoreAutoScale(_ignoreAutoScale);
  vcurve->setYVectorOffset(_yVectorOffset);
  if (_inputScalars.find(YOFFSETSCALAR) != _inputScalars.end()) {
    vcurve->setYVectorOffsetScalar(*_inputScalars.find(YOFFSETSCALAR));
  }

  duplicatedMap.insert(KstVCurvePtr(this), KstDataObjectPtr(vcurve));
  
  return KstDataObjectPtr(vcurve);
}


void KstVCurve::paint(const KstCurveRenderContext& context) {
  KstVectorPtr xv = *_inputVectors.find(COLOR_XVECTOR);
  KstVectorPtr yv = *_inputVectors.find(COLOR_YVECTOR);

  if (!xv || !yv) {
    return;
  }

  KstPainter *p = context.p;
  QColor foregroundColor = context.foregroundColor;
  double Lx = context.Lx;
  double Hx = context.Hx;
  double Ly = context.Ly;
  double Hy = context.Hy;
  double m_X = context.m_X;
  double m_Y = context.m_Y;
  double b_X = context.b_X;
  double b_Y = context.b_Y;
  double XMin = context.XMin;
  double XMax = context.XMax;
  bool xLog = context.xLog;
  bool yLog = context.yLog;
  double xLogBase = context.xLogBase;
  double yLogBase = context.yLogBase;
  int penWidth = context.penWidth;
  double maxY = 0.0, minY = 0.0;
  double rX = 0.0, rY = 0.0;
  double rEX = 0.0, rEY = 0.0;
  double X1 = 0.0, Y1 = 0.0;
  double X2 = 0.0, Y2 = 0.0;
  double last_x1, last_y1;
  double yOffset = 0.0;
  bool overlap = false;
  int i_pt;

#ifdef BENCHMARK
  QTime bench_time;
  QTime benchtmp;
  int benchmark0 = 0;
  int benchmark1 = 0;
  int benchmark2 = 0;
  int benchmark3 = 0;

  bench_time.start();
  benchtmp.start();
#endif

  if (!yLog) {
    yOffset = yVectorOffsetValue();
  }

  int pointDim = KstCurvePointSymbol::dim(p);
  if (sampleCount() > 0) {
    Qt::PenStyle style = KstLineStyle[lineStyle()];
    int i0, iN;
    int width;

    if (lineWidth() == 0) {
      width = penWidth;
    } else {
      width = lineWidth() * penWidth;
    }

    if (xv->isRising()) {
      i0 = indexNearX(XMin, xv, NS);
      if (i0 > 0) {
        --i0;
      }
      iN = indexNearX(XMax, xv, NS);
      if (iN < sampleCount() - 1) {
        ++iN;
      }
    } else {
      i0 = 0;
      iN = sampleCount() - 1;
    }

#ifdef BENCHMARK
    clock_t linesStart = clock();
#endif

    if (hasLines()) {
      QPolygon points(MAX_NUM_POLYLINES);
      int lastPlottedX = 0;
      int lastPlottedY = 0;
      int index = 0;
      int i0Start = i0;

      p->setPen(QPen(color(), width, style));

// optimize - isnan seems expensive, at least in gcc debug mode
//            cachegrind backs this up.
#undef isnan
#define isnan(x) (x != x)
      rX = xv->interpolate(i0, NS);
      rY = yv->interpolate(i0, NS);
      // if invalid point then look backward for the last valid point.
      while (i0 > 0 && (isnan(rX) || isnan(rY))) {
        --i0;
        rX = xv->interpolate(i0, NS);
        rY = yv->interpolate(i0, NS);
      }

      // if invalid point then look forward for the next valid point...
      if (isnan(rX) || isnan(rY)) {
        i0 = i0Start;
        while (i0 < iN && (isnan(rX) || isnan(rY))) {
          ++i0;
          rX = xv->interpolate(i0, NS);
          rY = yv->interpolate(i0, NS);
        }
      }

      if (xLog) {
        rX = logXLo(rX, xLogBase);
      }
      if (yLog) {
        rY = logYLo(rY, yLogBase);
      }
      last_x1 = m_X*rX + b_X;
      last_y1 = m_Y*(rY + yOffset) + b_Y;

      i_pt = i0;

      while (i_pt < iN) {
        X2 = last_x1;
        Y2 = last_y1;

        ++i_pt;
        rX = xv->interpolate(i_pt, NS);
        rY = yv->interpolate(i_pt, NS);
        bool foundNan = false;

        // if necessary continue looking for the first valid point...
        while (i_pt < iN && (isnan(rX) || isnan(rY))) {
#undef isnan
          foundNan = true;
          ++i_pt;
          rX = xv->interpolate(i_pt, NS);
          rY = yv->interpolate(i_pt, NS);
        }

        if (KDE_ISUNLIKELY(foundNan)) {
          if (index > 0) {
            p->drawPolyline(points);
          }
          index = 0;
          if (overlap) {
            if (X2 >= Lx && X2 <= Hx) {
              if (maxY > Hy && minY <= Hy)
                maxY = Hy;
              if (minY < Ly && maxY >= Ly)
                minY = Ly;
              if (minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy) {
                p->drawLine(d2i(X2), d2i(minY), d2i(X2), d2i(maxY));
              }
            }
            overlap = false;
          }
        }

        if (xLog) {
          rX = logXLo(rX, xLogBase);
        }
        if (yLog) {
          rY = logYLo(rY, yLogBase);
        }
        X1 = m_X*rX + b_X;
        Y1 = m_Y*(rY + yOffset) + b_Y;

        last_x1 = X1;
        last_y1 = Y1;

        if (KDE_ISLIKELY(!foundNan)) {
          int X1i = d2i(X1);
          int X2i = d2i(X2);
          if (KDE_ISLIKELY(X1i == X2i)) {
            if (KDE_ISLIKELY(overlap)) {
              if (KDE_ISUNLIKELY(Y1 > maxY)) {
                maxY = Y1;
              }
              if (KDE_ISUNLIKELY(Y1 < minY)) {
                minY = Y1;
              }
            } else {
              if (Y1 < Y2) {
                minY = Y1;
                maxY = Y2;
              } else {
                maxY = Y1;
                minY = Y2;
              }
              overlap = true;
            }
          } else {
            if (KDE_ISLIKELY(overlap)) {
              if (KDE_ISLIKELY(X2 >= Lx && X2 <= Hx)) {
                if (KDE_ISUNLIKELY(maxY <= Hy && minY >= Ly)) {
                  int Y2i = d2i(Y2);
                  int maxYi = d2i(maxY);
                  int minYi = d2i(minY);

                  if (index >= MAX_NUM_POLYLINES-2) {
                    p->drawPolyline(points);
                    index = 0;
                  }
                  if (KDE_ISUNLIKELY(minYi == maxYi)) {
                    points.setPoint(index++, X2i, maxYi);
                  } else if (KDE_ISUNLIKELY(Y2 == minY)) {
                    points.setPoint(index++, X2i, maxYi);
                    points.setPoint(index++, X2i, minYi);
                  } else if (KDE_ISUNLIKELY(Y2 == maxY)) {
                    points.setPoint(index++, X2i, minYi);
                    points.setPoint(index++, X2i, maxYi);
                  } else {
                    points.setPoint(index++, X2i, minYi);
                    points.setPoint(index++, X2i, maxYi);
                    if (KDE_ISLIKELY(Y2 >= Ly && Y2 <= Hy)) {
                      points.setPoint(index++, X2i, Y2i);
                    }
                  }
                  lastPlottedX = X2i;
                  lastPlottedY = Y2i;
                } else {
                  if (KDE_ISUNLIKELY(maxY > Hy && minY <= Hy)) {
                    maxY = Hy;
                  }
                  if (KDE_ISUNLIKELY(minY < Ly && maxY >= Ly)) {
                    minY = Ly;
                  }
                  if (KDE_ISUNLIKELY(minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy)) {
                    if (index > 0) {
                      p->drawPolyline(points);
                      index = 0;
                    }
                    p->drawLine(X2i, d2i(minY), X2i, d2i(maxY));
                  }
                }
              }
              overlap = false;
            }

            if (KDE_ISLIKELY(!((X1 < Lx && X2 < Lx) || (X1 > Hx && X2 > Hx)))) {
              // trim the line to be within the plot...
              if (KDE_ISUNLIKELY(isinf(X1))) {
                Y1 = Y2;
                if (X1 > 0.0) {
                  X1 = Hx;
                } else {
                  X1 = Lx;
                }
              }

              if (KDE_ISUNLIKELY(isinf(X2))) {
                Y2 = Y1;
                if (X2 > 0.0) {
                  X2 = Hx;
                } else {
                  X2 = Lx;
                }
              }

              if (KDE_ISUNLIKELY(isinf(Y1))) {
                X1 = X2;
                if (Y1 > 0.0) {
                  Y1 = Hy;
                } else {
                  Y1 = Ly;
                }
              }

              if (KDE_ISUNLIKELY(isinf(Y2))) {
                X2 = X1;
                if (Y2 > 0.0) {
                  Y2 = Hy;
                } else {
                  Y2 = Ly;
                }
              }

              if (KDE_ISUNLIKELY(X1 < Lx && X2 > Lx)) {
                Y1 = (Y2 - Y1) / (X2 - X1) * (Lx - X1) + Y1;
                X1 = Lx;
              } else if (KDE_ISUNLIKELY(X2 < Lx && X1 > Lx)) {
                Y2 = (Y1 - Y2) / (X1 - X2) * (Lx - X2) + Y2;
                X2 = Lx;
              }

              if (KDE_ISUNLIKELY(X1 < Hx && X2 > Hx)) {
                Y2 = (Y2 - Y1) / (X2 - X1) * (Hx - X1) + Y1;
                X2 = Hx;
              } else if (KDE_ISUNLIKELY(X2 < Hx && X1 > Hx)) {
                Y1 = (Y1 - Y2) / (X1 - X2) * (Hx - X2) + Y2;
                X1 = Hx;
              }

              if (KDE_ISUNLIKELY(Y1 < Ly && Y2 > Ly)) {
                X1 = (X2 - X1) / (Y2 - Y1) * (Ly - Y1) + X1;
                Y1 = Ly;
              } else if (KDE_ISUNLIKELY(Y2 < Ly && Y1 > Ly)) {
                X2 = (X1 - X2) / (Y1 - Y2) * (Ly - Y2) + X2;
                Y2 = Ly;
              }

              if (KDE_ISUNLIKELY(Y1 < Hy && Y2 > Hy)) {
                X2 = (X2 - X1) / (Y2 - Y1) * (Hy - Y1) + X1;
                Y2 = Hy;
              } else if (KDE_ISUNLIKELY(Y2 < Hy && Y1 > Hy)) {
                X1 = (X1 - X2) / (Y1 - Y2) * (Hy - Y2) + X2;
                Y1 = Hy;
              }

              if (X1 >= Lx && X1 <= Hx && X2 >= Lx && X2 <= Hx &&
                  Y1 >= Ly && Y1 <= Hy && Y2 >= Ly && Y2 <= Hy) {
                int X1i = d2i(X1);
                int Y1i = d2i(Y1);
                int X2i = d2i(X2);
                int Y2i = d2i(Y2);

                if (KDE_ISUNLIKELY(index == 0)) {
                  points.setPoint(index++, X2i, Y2i);
                  points.setPoint(index++, X1i, Y1i);
                } else if (lastPlottedX == X2i &&
                    lastPlottedY == Y2i &&
                    index < MAX_NUM_POLYLINES) {
                  points.setPoint(index++, X1i, Y1i);
                } else {
                  if (KDE_ISLIKELY(index > 1)) {
                    p->drawPolyline(points);
                  }
                  index = 0;
                  points.setPoint(index++, X2i, Y2i);
                  points.setPoint(index++, X1i, Y1i);
                }
                lastPlottedX = X1i;
                lastPlottedY = Y1i;
              }
            }
          } // end if (X1 == X2)
        } // end if (!foundNan)
      } // end while

      // we might a have polyline left undrawn...
      if (index > 1) {
        p->drawPolyline(points);
        index = 0;
      }

      // we might have some overlapping points still unplotted...
      if (overlap) {
        if (X2 >= Lx && X2 <= Hx) {
          if (maxY > Hy && minY <= Hy) {
            maxY = Hy;
          }
          if (minY < Ly && maxY >= Ly) {
            minY = Ly;
          }
          if (minY >= Ly && minY <= Hy && maxY >= Ly && maxY <= Hy) {
            p->drawLine(d2i(X2), d2i(minY), d2i(X2), d2i(maxY));
          }
        }
        overlap = false;
      }
    } // end if hasLines()

    KstVectorPtr exv = *_inputVectors.find(EXVECTOR);
    KstVectorPtr eyv = *_inputVectors.find(EYVECTOR);
    KstVectorPtr exmv = *_inputVectors.find(EXMINUSVECTOR);
    KstVectorPtr eymv = *_inputVectors.find(EYMINUSVECTOR);

    // draw the bargraph bars, if any...
    if (hasBars()) {
      bool has_top = true;
      bool has_bot = true;
      bool has_left = true;
      bool has_right = true;
      bool visible = true;
      double rX2 = 0.0;
      double drX = 0.0;

      if (barStyle() == 1) { // filled
        p->setPen(QPen(foregroundColor, width, style));
      } else {
        p->setPen(QPen(color(), width, style));
      }

      if (!exv) {
        // determine the bar position width. NOTE: This is done
        //  only if xv->isRising() as in this case the calculation
        //  is simple...
        drX = (maxX() - minX())/double(sampleCount());
        if (xv->isRising()) {
          double oldX = 0.0;

          for (i_pt = i0; i_pt <= iN; i_pt++) {
            rX = xv->interpolate(i_pt, NS);
            if (i_pt > i0) {
              if (rX - oldX < drX) {
                drX = rX - oldX;
              }
            }
            oldX = rX; 
          }
        }
      }

      for (i_pt = i0; i_pt <= iN; i_pt++) {
        visible = has_bot = has_top = has_left = has_right = true;

        if (exv) {
          drX = exv->interpolate(i_pt, NS);
        }
        rX = xv->interpolate(i_pt, NS);
        rY = yv->interpolate(i_pt, NS);
        rX -= drX/2.0;
        rX2 = rX + drX;
        if (xLog) {
          rX = logXLo(rX, xLogBase);
          rX2 = logXLo(rX2, xLogBase);
        }
        if (yLog) {
          rY = logYLo(rY, yLogBase);
        }

        X1 = m_X * rX + b_X;
        X2 = m_X * rX2 + b_X;
        if (X1 > Hx || X2 < Lx) {
          visible = false;
        } else {
          if (X1 < Lx) {
            has_left = false;
            X1 = Lx;
          }
          if (X2 > Hx) {
            has_right = false;
            X2 = Hx;
          }
        }

        // determine where the top of the bar is and whether
        // to draw the top line
        Y1 = m_Y * (rY + yVectorOffsetValue()) + b_Y;
        if (Y1 < Ly) {
          Y1 = Ly;
          has_top = false;
        }
        if (Y1 > Hy) {
          Y1 = Hy;
          has_top = false;
        }

        // determine where the bottom of the bar is and whether
        // to draw the bottom line
        if (yLog) {
          Y2 = Hy;
          has_bot = false;
        } else {
          Y2 = b_Y;
          if (Y2 < Ly) {
            Y2 = Ly;
            has_bot = false;
          }
          if (Y2 > Hy) {
            Y2 = Hy;
            has_bot = false;
          }
        }

        if (Y1 == Ly && Y2 == Ly) {
          visible = false;
        }
        else if (Y1 == Hy && Y2 == Hy) {
          visible = false;
        }

        if (visible) {
          if (barStyle() == 1) { // filled
            int X1i = d2i(X1);
            int Y1i = d2i(Y1);
            p->fillRect(X1i, Y1i, d2i(X2) - X1i, d2i(Y2) - Y1i, color());
          }
          if (has_top) {
            int Y1i = d2i(Y1);
            p->drawLine(d2i(X1-(width/2)), Y1i, d2i(X2+(width/2)), Y1i);
          }
          if (has_bot) {
            int Y2i = d2i(Y2);
            p->drawLine(d2i(X1-(width/2)), Y2i, d2i(X2-(width/2)), Y2i);
          }
          if (has_left) {
            int X1i = d2i(X1);
            p->drawLine(X1i, d2i(Y1-(width/2)), X1i, d2i(Y2+(width/2)));
          }
          if (has_right) {
            int X2i = d2i(X2);
            p->drawLine(X2i, d2i(Y1-(width/2)), X2i, d2i(Y2+(width/2)));
          }
        }
      }
    }

#ifdef BENCHMARK
    benchmark1 = benchtmp.elapsed();
#endif

    QPen pen(color(), width);
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);

    // draw the points, if any...
    if (hasPoints()) {
      if (hasLines() && pointDensity() > 0 && 
          (unsigned int)pointDensity() < KSTPOINTDENSITY_MAXTYPE) {
        const double w = Hx - Lx;
        const double h = Hy - Ly;
        const int size = int(qMax(w, h)) / int(pow(3.0, KSTPOINTDENSITY_MAXTYPE - pointDensity()));
        QRegion rgn((int)Lx, (int)Ly, (int)w, (int)h);
        QPoint pt;

        for (i_pt = i0; i_pt <= iN; ++i_pt) {
          rX = xv->interpolate(i_pt, NS);
          rY = yv->interpolate(i_pt, NS);
          if (xLog) {
            rX = logXLo(rX, xLogBase);
          }
          if (yLog) {
            rY = logYLo(rY, yLogBase);
          }

          pt.setX(d2i(m_X * rX + b_X));
          pt.setY(d2i(m_Y * (rY + yOffset) + b_Y));
          if (rgn.contains(pt)) {
            KstCurvePointSymbol::draw(_pointStyle, p, pt.x(), pt.y(), width);
            rgn -= QRegion(pt.x()-(size/2), pt.y()-(size/2), size, size, QRegion::Ellipse);
          }
        }
      } else {
        for (i_pt = i0; i_pt <= iN; ++i_pt) {
          rX = xv->interpolate(i_pt, NS);
          rY = yv->interpolate(i_pt, NS);
          if (xLog) {
            rX = logXLo(rX, xLogBase);
          }
          if (yLog) {
            rY = logYLo(rY, yLogBase);
          }

          X1 = m_X * rX + b_X;
          Y1 = m_Y * (rY + yOffset) + b_Y;
          if (X1 >= Lx && X1 <= Hx && Y1 >= Ly && Y1 <= Hy) {
            KstCurvePointSymbol::draw(_pointStyle, p, d2i(X1), d2i(Y1), width);
          }
        }
      }
    }

#ifdef BENCHMARK
    benchmark2 = benchtmp.elapsed();
#endif

    // draw the x-errors, if any...
    if ((exv || exmv) && !hasBars()) {
      double rX1;
      double rX2;
      bool do_low_flag = true;
      bool do_high_flag = true;
      bool errorSame = false;

      if (exv && exmv && xETag() == xEMinusTag()) {
        errorSame = true;
      }

      for (i_pt = i0; i_pt <= iN; i_pt++) {
        do_low_flag = true;
        do_high_flag = true;

        rX = xv->interpolate(i_pt, NS);
        rY = yv->interpolate(i_pt, NS);
        if (errorSame) {
          rEX = fabs(exv->interpolate(i_pt, NS));
          if (xLog) {
            rX1 = logXLo(rX - rEX, xLogBase);
            rX2 = logXLo(rX + rEX, xLogBase);
          } else {
            rX1 = rX - rEX;
            rX2 = rX + rEX;
          }
        } else if (exv && exmv) {
          double rEXHi = fabs(exv->interpolate(i_pt, NS));
          double rEXLo = fabs(exmv->interpolate(i_pt, NS));
          if (xLog) {
            rX1 = logXLo(rX - rEXLo, xLogBase);
            rX2 = logXLo(rX + rEXHi, xLogBase);
          } else {
            rX1 = rX - rEXLo;
            rX2 = rX + rEXHi;
          }
        } else if (exv) {
          rEX = exv->interpolate(i_pt, NS);
          if (xLog) {
            rX1 = logXLo(rX, xLogBase);
            rX2 = logXLo(rX + fabs(rEX), xLogBase);
          } else {
            rX1 = rX;
            rX2 = rX + fabs(rEX);
          }
          do_low_flag = false;
        } else {
          rEX = fabs(exmv->interpolate(i_pt, NS));
          if (xLog) {
            rX1 = logXLo(rX - rEX, xLogBase);
            rX2 = logXLo(rX, xLogBase);
          } else {
            rX1 = rX - rEX;
            rX2 = rX;
          }
          do_high_flag = false;
        }

        if (yLog) {
          rY = logYLo(rY, yLogBase);
        }

        X1 = m_X * rX1 + b_X;
        X2 = m_X * rX2 + b_X;
        Y1 = m_Y * (rY + yOffset) + b_Y;

        if (X1 < Lx && X2 > Lx) {
          X1 = Lx;
          do_low_flag = false;
        } else if (X1 > Lx && X2 < Lx) {
          X2 = Lx;
          do_high_flag = false;
        }

        if (X1 < Hx && X2 > Hx) {
          X2 = Hx;
          do_high_flag = false;
        } else if (X1 > Hx && X2 < Hx) {
          X1 = Hx;
          do_low_flag = false;
        }


        if (X1 >= Lx && X2 <= Hx && Y1 >= Ly && Y1 <= Hy) {
          int X1i = d2i(X1);
          int X2i = d2i(X2);
          int Y1i = d2i(Y1);
          p->drawLine(X1i, Y1i, X2i, Y1i);
          if (do_low_flag) {
            p->drawLine(X1i, Y1i + pointDim, X1i, Y1i - pointDim);
          }
          if (do_high_flag) {
            p->drawLine(X2i, Y1i + pointDim, X2i, Y1i - pointDim);
          }
        }
      }
    }

    // draw the y-errors, if any...
    if (eyv || eymv) {
      double rY1;
      double rY2;
      bool do_low_flag = true;
      bool do_high_flag = true;
      bool errorSame = false;

      if (eyv && eymv && yETag() == yEMinusTag()) {
        errorSame = true;
      }

      for (i_pt = i0; i_pt <= iN; i_pt++) {
        do_low_flag = true;
        do_high_flag = true;

        rX = xv->interpolate(i_pt, NS);
        rY = yv->interpolate(i_pt, NS);
        if (errorSame) {
          rEY = eyv->interpolate(i_pt, NS);
          if (yLog) {
            rY1 = logYLo(rY-fabs(rEY), yLogBase);
            rY2 = logYLo(rY+fabs(rEY), yLogBase);
          } else {
            rY1 = rY-fabs(rEY);
            rY2 = rY+fabs(rEY);
          }
        } else if (eyv && eymv) {
          double rEYHi = fabs(eyv->interpolate(i_pt, NS));
          double rEYLo = fabs(eymv->interpolate(i_pt, NS));
          if (yLog) {
            rY1 = logYLo(rY - rEYLo, yLogBase);
            rY2 = logYLo(rY + rEYHi, yLogBase);
          } else {
            rY1 = rY - rEYLo;
            rY2 = rY + rEYHi;
          }
        } else if (eyv) {
          rEY = fabs(eyv->interpolate(i_pt, NS));
          if (yLog) {
            rY1 = logYLo(rY, yLogBase);
            rY2 = logYLo(rY + rEY, yLogBase);
          } else {
            rY1 = rY;
            rY2 = rY + rEY;
          }
          do_low_flag = false;
        } else {
          rEY = fabs(eymv->interpolate(i_pt, NS));
          if (yLog) {
            rY1 = logYLo(rY - rEY, yLogBase);
            rY2 = logYLo(rY, yLogBase);
          } else {
            rY1 = rY - rEY;
            rY2 = rY;
          }
          do_high_flag = false;
        }

        if (xLog) {
          rX = logXLo(rX, xLogBase);
        }

        X1 = m_X * rX + b_X;
        Y1 = m_Y * (rY1 + yOffset) + b_Y;
        Y2 = m_Y * (rY2 + yOffset) + b_Y;

        if (Y1 < Ly && Y2 > Ly) {
          Y1 = Ly;
          do_low_flag = false;
        } else if (Y1 > Ly && Y2 < Ly) {
          Y2 = Ly;
          do_high_flag = false;
        }

        if (Y1 < Hy && Y2 > Hy) {
          Y2 = Hy;
          do_high_flag = false;
        } else if (Y1 > Hy && Y2 < Hy) {
          Y1 = Hy;
          do_low_flag = false;
        }

        if (X1 >= Lx && X1 <= Hx && Y1 >= Ly && Y2 <= Hy) {
          int X1i = d2i(X1);
          int Y1i = d2i(Y1);
          int Y2i = d2i(Y2);
          p->drawLine(X1i, Y1i, X1i, Y2i);
          if (do_low_flag) {
            p->drawLine(X1i + pointDim, Y1i, X1i - pointDim, Y1i);
          }
          if (do_high_flag) {
            p->drawLine(X1i + pointDim, Y2i, X1i - pointDim, Y2i);
          }
        }
      }
    } // end if (hasYError())
  } // end if (sampleCount() > 0)
}


void KstVCurve::yRange(double xFrom, double xTo, double* yMin, double* yMax) {
  if (!yMin || !yMax) {
    return;
  }

  KstVectorPtr xv = *_inputVectors.find(COLOR_XVECTOR);
  KstVectorPtr yv = *_inputVectors.find(COLOR_YVECTOR);
  if (!xv || !yv) {
    *yMin = *yMax = 0.0;
    return;
  }

  // get range of the curve to search for min/max
  int i0, iN;
  if (xv->isRising()) {
    i0 = indexNearX(xFrom, xv, NS);
    iN = indexNearX(xTo, xv, NS);
  } else {
    i0 = 0;
    iN = sampleCount() - 1;
  }

  // search for min/max
  bool first = true;
  double newYMax = 0.0;
  double newYMin = 0.0;

  for (int i_pt = i0; i_pt <= iN; i_pt++) {
    double rX = xv->interpolate(i_pt, NS);
    double rY = yv->interpolate(i_pt, NS);

    // make sure this point is visible
    if (rX >= xFrom && rX <= xTo) {
      // update min/max
      if (first || rY > newYMax) {
        newYMax = rY;
      }
      if (first || rY < newYMin) {
        newYMin = rY;
      }
      first = false;
    }
  }
  *yMin = newYMin;
  *yMax = newYMax;
}


KstDataObjectPtr KstVCurve::providerDataObject() const {
  KstVectorPtr vp;
  KstDataObjectPtr provider;
    
  KST::vectorList.lock().readLock();
// xxx  vp = *KST::vectorList.findTag(yVTag().tag());  // FIXME: should use full tag
  KST::vectorList.lock().unlock();;

  if (vp) {
    vp->readLock();
    provider = kst_cast<KstDataObject>(vp->provider());
    vp->unlock();
  }
  
  return provider;
}


double KstVCurve::distanceToPoint(double xpos, double ypos, double distanceMax, const KstCurveRenderContext& context) {
  //
  // xpos, ypos are values rather than pixel position...
  //
  double distance = -1.0;

  if (hasLines() || hasPoints()) {
    KstVectorPtr xv = *_inputVectors.find(COLOR_XVECTOR);
    double distanceNew;
    double xposPixel;
    double yposPixel;
    double xposPoint;
    double yposPoint;
    double yOffset;
    double xNear;
    double yNear;
    double yMin;
    double yMax;
    double xLo;
    double xHi;
    bool doneLine = false;
    int iNearX;
    int i;

    if (context.xLog) {
      xposPixel = logXLo(xpos, context.xLogBase);
      xposPixel = context.m_X*xposPixel + context.b_X;
    } else {
      xposPixel = context.m_X*xpos + context.b_X;
    }

    if (context.yLog) {
      yposPixel = logYLo(ypos, context.yLogBase);
      yposPixel = context.m_Y*yposPixel + context.b_Y;
    } else {
      yposPixel = context.m_Y*ypos + context.b_Y;
    }

    iNearX = getIndexNearXY(xpos, distanceMax, ypos);
    if (iNearX != -1) {
      point(iNearX, xNear, yNear);
      if (context.xLog) {
        xposPoint = logXLo(xNear, context.xLogBase);
        xposPoint = context.m_X*xposPoint + context.b_X;
      } else {
        xposPoint = context.m_X*xNear + context.b_X;
      }

      if (context.yLog) {
        yposPoint = logYLo(yNear, context.yLogBase);
        yposPoint = context.m_Y*yposPoint + context.b_Y;
      } else {
        yposPoint = context.m_Y*yNear + context.b_Y;
      }

      distance = sqrt(((xposPoint - xposPixel)*(xposPoint - xposPixel)) + ((yposPoint - yposPixel)*(yposPoint - yposPixel)));
    }

    if (xv && xv->isRising()) {
      if (hasLines()) {
        //
        // check how far the closest line is from -5 to +5 pixels from our current x-position...
        //
        for (i=-5; i<5; ++i) {
          if (context.xLog) {
            xLo = pow(context.xLogBase, logXLo(xpos) + ((double)i / context.m_X));
            xHi = pow(context.xLogBase, logXLo(xpos) + (((double)i+1.0) / context.m_X));
          } else {
            xLo = xpos + ((double)i / context.m_X);
            xHi = xpos + (((double)i+1.0) / context.m_X);
          }

          if (xLo >= minX() && xHi <= maxX()) {
            yRange(xLo, xHi, &yMin, &yMax);

            if (yMin == 0.0 && yMax == 0.0 && !doneLine) {
              //
              // we are probably on a line, in between two points...
              //
              KstVectorPtr xv = *_inputVectors.find(COLOR_XVECTOR);

              if (xv && xv->isRising()) {
                // if hasLines then we should find the distance between the curve and the point, not the data and 
                //  the point. if isRising because it is (probably) to slow to use this technique if the data is 
                //  unordered. borrowed from indexNearX. use binary search to find the indices immediately above 
                //  and below our xpos.
                double xBottom;
                double yBottom;
                double xTop;
                double yTop;
                int iTop = NS - 1;
                int iBottom = 0;

                while (iBottom + 1 < iTop) {
                  int i0 = (iTop + iBottom)/2;
                  double rX = xv->interpolate(i0, NS);

                  if (xpos < rX) {
                    iTop = i0;
                  } else {
                    iBottom = i0;
                  }
                }

                point(iBottom, xBottom, yBottom);
                point(iTop, xTop, yTop);

                if (xBottom <= xpos && xTop >= xpos) {
                  if (context.xLog) {
                    xTop = logXLo(xTop, context.xLogBase);
                    xTop = context.m_X*xTop + context.b_X;
                    xBottom = logXLo(xBottom, context.xLogBase);
                    xBottom = context.m_X*xBottom + context.b_X;
                  } else {
                    xTop = context.m_X*xTop + context.b_X;
                    xBottom = context.m_X*xBottom + context.b_X;
                  }

                  if (context.yLog) {
                    yTop = logYLo(yTop, context.yLogBase);
                    yTop = context.m_Y*yTop + context.b_Y;
                    yBottom = logYLo(yBottom, context.yLogBase);
                    yBottom = context.m_Y*yBottom + context.b_Y;
                  } else {
                    yTop = context.m_Y*yTop + context.b_Y;
                    yBottom = context.m_Y*yBottom + context.b_Y;
                  }

                  distanceNew = fabs(((xTop-xBottom)*(yBottom-yposPixel)) - ((xBottom-xposPixel)*(yTop-yBottom)));
                  distanceNew /= sqrt(((xTop-xBottom)*(xTop-xBottom)) + ((yTop-yBottom)*(yTop-yBottom)));

                  if (distance == -1.0 || distanceNew < distance) {
                    distance = distanceNew;
                  }
                }
              }
              doneLine = true;
            } else {
              //
              // we are probably in an area of very dense lines...
              //
              if (context.yLog) {
                yMin = logYLo(yMin, context.yLogBase);
                yMax = logYLo(yMax, context.yLogBase);
              }
              yMin = context.m_Y*yMin + context.b_Y;
              yMax = context.m_Y*yMax + context.b_Y;

              if ((yMin <= yposPixel && yposPixel <= yMax) || 
                  (yMax <= yposPixel && yposPixel <= yMin)) {
                yOffset = 0.0;
              } else if (fabs(yMin - yposPixel) < fabs(yMax - yposPixel)) {
                yOffset = fabs(yMin - yposPixel);
              } else {
                yOffset = fabs(yMax - yposPixel);
              }
              distanceNew = ((double)i * (double)i) + (yOffset * yOffset);
              distanceNew = sqrt(distanceNew);
              if (distance == -1.0 || distanceNew < distance) {
                distance = distanceNew;
              }
            }
          }
        }
      } else {
        //
        // we have points only and need to check more carefully if we are close to a point...
        //
        KstVectorPtr xv = xVector();
        KstVectorPtr yv = yVector();
        double xPixel;
        double yPixel;
        double x;
        double y;
        int iNearXLoop;

        if (xv && yv) {
          iNearXLoop = iNearX;
          xPixel = context.Hx;
          while (iNearXLoop > 0 && xPixel > context.Lx && xPixel > xposPixel - distanceMax) {
            x = xv->interpolate(iNearXLoop, NS);
            y = yv->interpolate(iNearXLoop, NS);
            if (context.xLog) {
              xPixel = logXLo(x, context.xLogBase);
              xPixel = context.m_X*xPixel + context.b_X;
            } else {
              xPixel = context.m_X*x + context.b_X;
            }

            if (context.yLog) {
              yPixel = logYLo(y, context.yLogBase);
              yPixel = context.m_Y*yPixel + context.b_Y;
            } else {
              yPixel = context.m_Y*y + context.b_Y;
            }

            distanceNew = sqrt(((xPixel - xposPixel)*(xPixel - xposPixel)) + 
                               ((yPixel - yposPixel)*(yPixel - yposPixel)));
            if (distanceNew < distance || distance == -1.0) {
              distance = distanceNew;
            }
            iNearXLoop--;
          }

          iNearXLoop = iNearX;
          xPixel = context.Lx;
          while (iNearXLoop < NS && xPixel < context.Hx && xPixel < xposPixel + distanceMax) {
            x = xv->interpolate(iNearXLoop, NS);
            y = yv->interpolate(iNearXLoop, NS);
            if (context.xLog) {
              xPixel = logXLo(x, context.xLogBase);
              xPixel = context.m_X*xPixel + context.b_X;
            } else {
              xPixel = context.m_X*x + context.b_X;
            }

            if (context.yLog) {
              yPixel = logYLo(y, context.yLogBase);
              yPixel = context.m_Y*yPixel + context.b_Y;
            } else {
              yPixel = context.m_Y*y + context.b_Y;
            }

            distanceNew = sqrt(((xPixel - xposPixel)*(xPixel - xposPixel)) + 
                               ((yPixel - yposPixel)*(yPixel - yposPixel)));
            if (distanceNew < distance || distance == -1.0) {
              distance = distanceNew;
            }
            iNearXLoop++;
          }
        }
      }
    }
  }

  return distance;
}


void KstVCurve::paintLegendSymbol(KstPainter *p, const QRect& bound) {
  int width;

  if (lineWidth() == 0) {
    width = p->lineWidthAdjustmentFactor();
  } else {
    width = lineWidth() * p->lineWidthAdjustmentFactor();
  }

  p->save();
  if (hasLines()) {
    // draw a line from left to right centered vertically
    p->setPen(QPen(color(), width, KstLineStyle[lineStyle()]));
    p->drawLine(bound.left(), bound.top() + bound.height()/2,
                bound.right(), bound.top() + bound.height()/2);
  }
  if (hasPoints()) {
    // draw a point in the middle
    p->setPen(QPen(color(), width));
    KstCurvePointSymbol::draw(_pointStyle, p, bound.left() + bound.width()/2, bound.top() + bound.height()/2, width, 600);
  }
  p->restore();
}


KstVCurve::InterpType KstVCurve::interp() const {
  return (_interp);
}


void KstVCurve::setInterp(KstVCurve::InterpType itype) {
  _interp = itype;
  setDirty();
}

