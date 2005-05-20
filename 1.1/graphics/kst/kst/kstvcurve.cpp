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

// includes for Qt
#include <qstylesheet.h>

// includes for KDE
#include <klocale.h>
#include <kdebug.h>

// application specific includes
#include "dialoglauncher.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstrvector.h"
#include "kstvcurve.h"

static const QString& COLOR_XVECTOR = KGlobal::staticQString("X");
static const QString& COLOR_YVECTOR = KGlobal::staticQString("Y");
static const QString& EXVECTOR = KGlobal::staticQString("EX");
static const QString& EYVECTOR = KGlobal::staticQString("EY");
static const QString& EXMINUSVECTOR = KGlobal::staticQString("EXMinus");
static const QString& EYMINUSVECTOR = KGlobal::staticQString("EYMinus");

KstVCurve::KstVCurve(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y,
                      KstVectorPtr in_EX, KstVectorPtr in_EY,
                      KstVectorPtr in_EXMinus, KstVectorPtr in_EYMinus,
                      const QColor &in_color)
: KstBaseCurve() {
  setHasPoints(false);
  setHasLines(true);
  setLineWidth(0);
  setLineStyle(0);
  setPointDensity(0);

  commonConstructor(in_tag, in_color);
  VX = in_X;
  VY = in_Y;
  EX = in_EX;
  EY = in_EY;
  EXMinus = in_EXMinus;
  EYMinus = in_EYMinus;
  setDirty();
}


KstVCurve::KstVCurve(QDomElement &e)
: KstBaseCurve(e) {
  QString in_tag, xname, yname, exname, eyname, exminusname, eyminusname;
  // QColor in_color(KstColorSequence::next(-1));
  QColor in_color("red"); // the above line is invalid.
  bool hasMinus = false;

  setHasPoints(false);
  setHasLines(false);
  setHasBars(false);
  setLineWidth(0);
  setLineStyle(0);
  setBarStyle(0);
  setPointDensity(0);

  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
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

      // the following options are only needed to change from the default
      } else if (e.tagName() == "hasLines") {
        HasLines = (e.text() != "0");
      } else if (e.tagName() == "hasPoints") {
        HasPoints = (e.text() != "0");
      } else if (e.tagName() == "hasBars") {
        HasBars = (e.text() != "0");
      } else if (e.tagName() == "pointType") {
        Point.setType(e.text().toInt());
      } else if (e.tagName() == "lineWidth") {
        LineWidth = e.text().toInt();
      } else if (e.tagName() == "lineStyle") {
        LineStyle = e.text().toInt();
      } else if (e.tagName() == "barStyle") {
        BarStyle = e.text().toInt();
      } else if (e.tagName() == "pointDensity") {
        PointDensity = e.text().toInt();
      } else if (e.tagName() == "ignoreAutoScale") {
        _ignoreAutoScale = true;
      }
    }
    n = n.nextSibling();
  }

  _inputVectorLoadQueue.append(qMakePair(COLOR_XVECTOR, xname));
  _inputVectorLoadQueue.append(qMakePair(COLOR_YVECTOR, yname));
  _inputVectorLoadQueue.append(qMakePair(EXVECTOR, exname));
  _inputVectorLoadQueue.append(qMakePair(EYVECTOR, eyname));
  _inputVectorLoadQueue.append(qMakePair(EXMINUSVECTOR, exminusname));
  _inputVectorLoadQueue.append(qMakePair(EYMINUSVECTOR, eyminusname));
  commonConstructor(in_tag, in_color);
}


void KstVCurve::commonConstructor(const QString &in_tag, const QColor &in_color) {
  _typeString = i18n("Curve");
  Color = in_color;
  setTagName(in_tag);
}


KstVCurve::~KstVCurve() {
}


bool KstVCurve::loadInputs() {
  QValueList<QPair<QString,QString> >::Iterator i;
  KST::vectorList.lock().readLock();
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    if ((*i).first == COLOR_XVECTOR) {
      VX = *KST::vectorList.findTag((*i).second);
      if (!VX.data()) {
        KstDebug::self()->log(i18n("Unable to find X vector for %2: [%1]").arg((*i).second).arg(tagName()), KstDebug::Warning);
        KST::vectorList.lock().readUnlock();
        return false;
      }
    } else if ((*i).first == COLOR_YVECTOR) {
      VY = *KST::vectorList.findTag((*i).second);
      if (!VY.data()) {
        KstDebug::self()->log(i18n("Unable to find Y vector for %2: [%1]").arg((*i).second).arg(tagName()), KstDebug::Warning);
        KST::vectorList.lock().readUnlock();
        return false;
      }
    } else if ((*i).first == EXVECTOR) {
      EX = *KST::vectorList.findTag((*i).second);
    } else if ((*i).first == EYVECTOR) {
      EY = *KST::vectorList.findTag((*i).second);
    } else if ((*i).first == EXMINUSVECTOR) {
      EXMinus = *KST::vectorList.findTag((*i).second);
    } else if ((*i).first == EYMINUSVECTOR) {
      EYMinus = *KST::vectorList.findTag((*i).second);
    }
  }
  KST::vectorList.lock().readUnlock();

  setDirty();

  return true;
}


KstObject::UpdateType KstVCurve::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  if (!VX || !VY) {
    return setLastUpdateResult(NO_CHANGE);
  }

  bool depUpdated = force;

  depUpdated = UPDATE == VX->update(update_counter) || depUpdated;
  depUpdated = UPDATE == VY->update(update_counter) || depUpdated;

  if (EX) {
    depUpdated = UPDATE == EX->update(update_counter) || depUpdated;
  }

  if (EY) {
    depUpdated = UPDATE == EY->update(update_counter) || depUpdated;
  }

  if (EXMinus) {
    depUpdated = UPDATE == EXMinus->update(update_counter) || depUpdated;
  }

  if (EYMinus) {
    depUpdated = UPDATE == EYMinus->update(update_counter) || depUpdated;
  }

  MaxX = VX->max();
  MinX = VX->min();
  MeanX = VX->mean();
  MinPosX = VX->minPos();
  _ns_maxx = VX->ns_max();
  _ns_minx = VX->ns_min();

  if (MinPosX > MaxX) {
    MinPosX = 0;
  }
  MaxY = VY->max();
  MinY = VY->min();
  MeanY = VY->mean();
  MinPosY = VY->minPos();
  _ns_maxy = VY->ns_max();
  _ns_miny = VY->ns_min();

  if (MinPosY > MaxY) {
    MinPosY = 0;
  }

  NS = QMAX(VX->length(), VY->length());

  return setLastUpdateResult(depUpdated ? UPDATE : NO_CHANGE);
}


void KstVCurve::point(int i, double &x, double &y) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
}


void KstVCurve::getEXPoint(int i, double &x, double &y, double &ex) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  ex = EX->interpolate(i, NS);
}


void KstVCurve::getEXMinusPoint(int i, double &x, double &y, double &ex) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  ex = EXMinus->interpolate(i, NS);
}


void KstVCurve::getEXPoints(int i, double &x, double &y, double &exminus, double &explus) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  explus = EX->interpolate(i, NS);
  exminus = EXMinus->interpolate(i, NS);
}


void KstVCurve::getEYPoint(int i, double &x, double &y, double &ey) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  ey = EY->interpolate(i, NS);
}


void KstVCurve::getEYMinusPoint(int i, double &x, double &y, double &ey) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  ey = EYMinus->interpolate(i, NS);
}


void KstVCurve::getEYPoints(int i, double &x, double &y, double &eyminus, double &eyplus) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  eyplus = EY->interpolate(i, NS);
  eyminus = EYMinus->interpolate(i, NS);
}


QString KstVCurve::xVTag() const {
  if (!VX.data()) {
    return QString::null;
  }
  return VX->tagName();
}


QString KstVCurve::yVTag() const {
  if (!VY.data()) {
    return QString::null;
  }
  return VY->tagName();
}


QString KstVCurve::xETag() const {
  if (!EX.data()) {
    return "<None>";
  }
  return EX->tagName();
}


QString KstVCurve::yETag() const {
  if (!EY.data()) {
    return "<None>";
  }
  return EY->tagName();
}


QString KstVCurve::xEMinusTag() const {
  if (!EXMinus.data()) {
    return "<None>";
  }
  return EXMinus->tagName();
}


QString KstVCurve::yEMinusTag() const {
  if (!EYMinus.data()) {
    return "<None>";
  }
  return EYMinus->tagName();
}


bool KstVCurve::hasXError() const {
  return EX.data() != 0L;
}


bool KstVCurve::hasYError() const {
  return EY.data() != 0L;
}


bool KstVCurve::hasXMinusError() const {
  return EXMinus.data() != 0L;
}


bool KstVCurve::hasYMinusError() const {
  return EYMinus.data() != 0L;
}


void KstVCurve::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<curve>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<xvectag>" << QStyleSheet::escape(VX->tagName()) << "</xvectag>" << endl;
  ts << l2 << "<yvectag>" << QStyleSheet::escape(VY->tagName()) << "</yvectag>" << endl;
  ts << l2 << "<hasMinus/>" << endl;
  if (EX.data()) {
    ts << l2 << "<exVectag>" << QStyleSheet::escape(EX->tagName()) << "</exVectag>" << endl;
  }
  if (EY.data()) {
    ts << l2 << "<eyVectag>" << QStyleSheet::escape(EY->tagName()) << "</eyVectag>" << endl;
  }
  if (EXMinus.data()) {
    ts << l2 << "<exMinusVectag>" << QStyleSheet::escape(EXMinus->tagName()) << "</exMinusVectag>" << endl;
  }
  if (EYMinus.data()) {
    ts << l2 << "<eyMinusVectag>" << QStyleSheet::escape(EYMinus->tagName()) << "</eyMinusVectag>" << endl;
  }
  ts << l2 << "<color>" << Color.name() << "</color>" << endl;
  if (HasLines) {
    ts << l2 << "<hasLines/>" << endl;
  }
  ts << l2 << "<lineWidth>" << LineWidth << "</lineWidth>" << endl;
  ts << l2 << "<lineStyle>" << LineStyle << "</lineStyle>" << endl;
  if (HasPoints) {
    ts << l2 << "<hasPoints/>" << endl;
  }
  ts << l2 << "<pointType>" << Point.type() << "</pointType>" << endl;
  ts << l2 << "<pointDensity>" << PointDensity << "</pointDensity>" << endl;
  if (HasBars) {
    ts << l2 << "<hasBars/>" << endl;
  }
  ts << l2 << "<barStyle>" << BarStyle << "</barStyle>" << endl;
  if (_ignoreAutoScale) {
    ts << l2 << "<ignoreAutoScale/>" << endl;
  }
  ts << indent << "</curve>" << endl;
}


void KstVCurve::setXVector(KstVectorPtr new_vx) {
  VX = new_vx;
  setDirty();
}


void KstVCurve::setYVector(KstVectorPtr new_vy) {
  VY = new_vy;
  setDirty();
}


void KstVCurve::setXError(KstVectorPtr new_ex) {
  EX = new_ex;
  setDirty();
}


void KstVCurve::setYError(KstVectorPtr new_ey) {
  EY = new_ey;
  setDirty();
}


void KstVCurve::setXMinusError(KstVectorPtr new_ex) {
  EXMinus = new_ex;
  setDirty();
}


void KstVCurve::setYMinusError(KstVectorPtr new_ey) {
  EYMinus = new_ey;
  setDirty();
}


QString KstVCurve::xLabel() const {
  return VX->label();
}


QString KstVCurve::yLabel() const {
  return VY->label();
}


QString KstVCurve::topLabel() const {
  return QString::null;
  //return VY->fileLabel();
}


KstCurveType KstVCurve::type() const {
  return KST_VCURVE;
}


QString KstVCurve::propertyString() const {
  return i18n("%1 vs %2").arg(yVTag()).arg(xVTag());
}


void KstVCurve::_showDialog() {
  KstDialogs::showCurveDialog(tagName());
}


int KstVCurve::samplesPerFrame() const {
  const KstRVector *rvp = dynamic_cast<const KstRVector*>(VY.data());
  return rvp ? rvp->samplesPerFrame() : 1;
}


bool KstVCurve::uses(KstObjectPtr p) const {
  bool rc = KstBaseCurve::uses(p);
  KstVectorPtr v = kst_cast<KstVector>(p);

  if (v) {
    rc = rc || VX == v || VY == v || EX == v || EY == v || EXMinus == v || EYMinus == v;
  }

  return rc;
}


KstVectorPtr KstVCurve::xVector() const {
  return VX;
}


KstVectorPtr KstVCurve::yVector() const {
  return VY;
}


KstVectorPtr KstVCurve::xErrorVector() const {
  return EX;
}


KstVectorPtr KstVCurve::yErrorVector() const {
  return EY;
}


KstVectorPtr KstVCurve::xMinusErrorVector() const {
  return EXMinus;
}


KstVectorPtr KstVCurve::yMinusErrorVector() const {
  return EYMinus;
}


// vim: ts=2 sw=2 et
