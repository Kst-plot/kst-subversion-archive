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
#include <klocale.h>
#include <kdebug.h>

#include "kstvcurve.h"
#include "kstdoc.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"
#include "kstcurvedialog_i.h"

static const QString XVECTOR = "X";
static const QString YVECTOR = "Y";
static const QString EXVECTOR = "EX";
static const QString EYVECTOR = "EY";

KstVCurve::KstVCurve(const QString &in_tag, KstVectorPtr in_X, KstVectorPtr in_Y,
		   KstVectorPtr in_EX, KstVectorPtr in_EY,
                   const QColor &in_color)
: KstBaseCurve() {
  setHasPoints(false);
  setHasLines(true);

  commonConstructor(in_tag, in_color);
  VX = in_X;
  VY = in_Y;
  EX = in_EX;
  EY = in_EY;
  update();
}


KstVCurve::KstVCurve(QDomElement &e)
: KstBaseCurve(e) {
  QString in_tag, xname, yname, exname, eyname;
  QColor in_color(KstColorSequence::next());

  setHasPoints(false);
  setHasLines(true);

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "xvectag") {
        xname = e.text();
      } else if (e.tagName() == "yvectag") {
        yname = e.text();
      } else if (e.tagName() == "exVectag") {
        exname = e.text();
      } else if (e.tagName() == "eyVectag") {
        eyname = e.text();
      } else if (e.tagName() == "color") {
        in_color.setNamedColor(e.text());
      } else if (e.tagName() == "hasLines") {
        HasLines = (e.text() != "0");
      } else if (e.tagName() == "hasPoints") {
	HasPoints = (e.text() != "0");
      } else if (e.tagName() == "pointType") {
	Point.setType(e.text().toInt());
      }
    }
    n = n.nextSibling();
  }

  _inputVectorLoadQueue.append(qMakePair(XVECTOR, xname));
  _inputVectorLoadQueue.append(qMakePair(YVECTOR, yname));
  _inputVectorLoadQueue.append(qMakePair(EXVECTOR, exname));
  _inputVectorLoadQueue.append(qMakePair(EYVECTOR, eyname));
  commonConstructor(in_tag, in_color);
}

void KstVCurve::commonConstructor(const QString &in_tag, const QColor &in_color) {
  _typeString = i18n("Curve");
  NumUsed = 0;
  Color = in_color;
  _tag = in_tag;
}


void KstVCurve::loadInputs() {
  QValueList<QPair<QString,QString> >::Iterator i;
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    if ((*i).first == XVECTOR) {
      VX = *KST::vectorList.findTag((*i).second);
    } else if ((*i).first == YVECTOR) {
      VY = *KST::vectorList.findTag((*i).second);
    } else if ((*i).first == EXVECTOR) {
      EX = *KST::vectorList.findTag((*i).second);
    } else if ((*i).first == EYVECTOR) {
      EY = *KST::vectorList.findTag((*i).second);
    }
  }

  if (!VX.data()) {
      kdWarning() << "Unable to find required X vector!" << endl;
  }

  if (!VY.data()) {
      kdWarning() << "Unable to find required Y vector!" << endl;
  }

  update();
}


KstVCurve::~KstVCurve() {
}

KstObject::UpdateType KstVCurve::update(int update_counter) {

  if (KstObject::checkUpdateCounter(update_counter))
    return NO_CHANGE;

  VX->update(update_counter);
  VY->update(update_counter);

  if (hasXError()) {
    VX->update(update_counter);
  }

  if (hasYError()) {
    VY->update(update_counter);
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

  if (VX->sampleCount() > VY->sampleCount()) {
    NS = VX->sampleCount();
  } else {
    NS = VY->sampleCount();
  }

  return UPDATE;
}

void KstVCurve::getPoint(int i, double &x, double &y) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
}

void KstVCurve::getEXPoint(int i, double &x, double &y, double &ex) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  ex = EX->interpolate(i, NS);
}

void KstVCurve::getEYPoint(int i, double &x, double &y, double &ey) {
  x = VX->interpolate(i, NS);
  y = VY->interpolate(i, NS);
  ey = EY->interpolate(i, NS);
}

QString KstVCurve::getXVTag() const{
  return VX->tagName();
}

QString KstVCurve::getYVTag() const{
  return VY->tagName();
}

QString KstVCurve::getXETag() const{
  if (!EX.data()) {
    return i18n("<None>");
  } else {
    return EX->tagName();
  }
}

QString KstVCurve::getYETag() const {
  if (!EY.data()) {
    return i18n("<None>");
  } else {
    return EY->tagName();
  }
}

bool KstVCurve::hasXError() const {
  return EX.data() != 0L;
}

bool KstVCurve::hasYError() const {
  return EY.data() != 0L;
}

/** Save curve information */
void KstVCurve::save(QTextStream &ts) {
  ts << " <curve>\n";
  ts << "  <tag>" << _tag << "</tag>\n";
  ts << "  <xvectag>" << VX->tagName() << "</xvectag>\n";
  ts << "  <yvectag>" << VY->tagName() << "</yvectag>\n";
  if (EX.data()) {
    ts << "  <exVectag>" << EX->tagName() << "</exVectag>";
  }
  if (EY.data()) {
    ts << "  <eyVectag>" << EY->tagName() << "</eyVectag>";
  }
  ts << "  <color>" << Color.name() << "</color>\n";
  ts << "  <hasLines>" << HasLines << "</hasLines>\n";
  ts << "  <hasPoints>" << HasPoints << "</hasPoints>\n";
  ts << "  <pointType>" << Point.getType() << "</pointType>\n";
  ts << " </curve>\n";
}

void KstVCurve::setXVector(KstVectorPtr new_vx) {
  VX = new_vx;
}

void KstVCurve::setYVector(KstVectorPtr new_vy) {
  VY = new_vy;
}

void KstVCurve::setXError(KstVectorPtr new_ex) {
  EX = new_ex;
}

void KstVCurve::setYError(KstVectorPtr new_ey) {
  EY = new_ey;
}

QString KstVCurve::getXLabel() const {
  return VX->label();
}

QString KstVCurve::getYLabel() const {
  return VY->label();
}

QString KstVCurve::getTopLabel() const {
  return VY->fileLabel();
}

KstCurveType KstVCurve::type() const {
  return KST_VCURVE;
}

QString KstVCurve::propertyString() const {
  return i18n("%1 vs %2").arg(getXVTag()).arg(getYVTag());
}

void KstVCurve::_showDialog() {
  KstCurveDialogI::globalInstance()->show_I(tagName());
}

