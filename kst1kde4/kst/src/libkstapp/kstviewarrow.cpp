/***************************************************************************
                              kstviewarrow.cpp
                             -------------------
    begin                : Jun 14, 2005
    copyright            : (C) 2005 The University of Toronto
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

#include "kstgfxarrowmousehandler.h"
#include "kstmath.h"
#include "kstviewarrow.h"
#include "kstviewobjectfactory.h"

#include <math.h>

#include <QBitmap>
#include <QMetaObject>
#include <QPainter>
#include <QTextStream>
#include <QVariant>

#define SIZE_ARROW (2.0 * sqrt(3.0))

KstViewArrow::KstViewArrow()
: KstViewLine("Arrow") {
  _editTitle = tr("Edit Arrow");
  _newTitle = tr("New Arrow");
  _hasFromArrow = false;
  _hasToArrow = true;
  _fromArrowScaling = 1.0;
  _toArrowScaling = 1.0;
  _standardActions |= Delete | Edit;
}


KstViewArrow::KstViewArrow(const QDomElement& e)
: KstViewLine(e) {
  _hasFromArrow = false;
  _hasToArrow = true;
  _fromArrowScaling = 1.0;
  _toArrowScaling = 1.0;
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement(); 
    if (!el.isNull()) {
      if (metaObject()->indexOfProperty(el.tagName().toLatin1()) > -1) {
        setProperty(el.tagName().toLatin1(), QVariant(el.text()));  
      }
    }
    n = n.nextSibling();
  }

  // always has this value
  _type = "Arrow";
  _editTitle = tr("Edit Arrow");
  _newTitle = tr("New Arrow");
  _standardActions |= Delete | Edit;
}


KstViewArrow::KstViewArrow(const KstViewArrow& arrow)
: KstViewLine(arrow) {
  _hasFromArrow = arrow._hasFromArrow;
  _hasToArrow = arrow._hasToArrow;
  _fromArrowScaling = arrow._fromArrowScaling;
  _toArrowScaling = arrow._toArrowScaling;

  // these always have these values
  _type = "Arrow";
  _standardActions |= Delete | Edit;
}


KstViewArrow::~KstViewArrow() {
}


KstViewObject* KstViewArrow::copyObjectQuietly(KstViewObject& parent, const QString& name) const {
  Q_UNUSED(name)

  KstViewArrow *viewArrow = new KstViewArrow(*this);
  parent.appendChild(KstViewObjectPtr(viewArrow), true);

  return viewArrow;
}


KstViewObject* KstViewArrow::copyObjectQuietly() const {
  KstViewArrow *viewArrow = new KstViewArrow(*this);

  return viewArrow;
}


void KstViewArrow::paintArrow(KstPainter& p, const QPoint& to, const QPoint &from, int w, double scaling) {
  double deltax = scaling * double(w);
  double theta = atan2(double(from.y() - to.y()), double(from.x() - to.x())) - M_PI / 2.0;
  double sina = sin(theta);
  double cosa = cos(theta);
  double yin = SIZE_ARROW * deltax;
  double x1, y1, x2, y2;
  QMatrix m(cosa, sina, -sina, cosa, 0.0, 0.0);

  m.map( deltax, yin, &x1, &y1);
  m.map(-deltax, yin, &x2, &y2);

  QPolygon pts(3);

  pts[0] = to;
  pts[1] = to + QPoint(d2i(x1), d2i(y1));
  pts[2] = to + QPoint(d2i(x2), d2i(y2));

  p.drawPolygon(pts);
}


QRegion KstViewArrow::clipRegion() {
  if (_clipMask.isEmpty()) {
    double scaling = qMax(_fromArrowScaling, _toArrowScaling);
    int w = int(ceil(SIZE_ARROW * scaling * double(width())));
    QRect rect(0, 0, _geom.bottomRight().x() + w + 1, _geom.bottomRight().y() + w + 1);

    QBitmap bm(rect.size());
    if (!bm.isNull()) {
      KstPainter p;

      p.setMakingMask(true);
      p.begin(&bm);
// xxx      p.setViewXForm(true);
      p.eraseRect(rect);
      paintSelf(p, QRegion());
// xxx      p.flush();
      _clipMask = QRegion(bm);
    }
  }

  return _clipMask;
}


void KstViewArrow::paintSelf(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.type() != KstPainter::P_PRINT && p.type() != KstPainter::P_EXPORT) {
    if (p.makingMask()) {
      KstViewLine::paintSelf(p, bounds);
// xxx      p.setRasterOp(Qt::SetROP);
    } else {
      const QRegion clip(clipRegion());
      KstViewLine::paintSelf(p, bounds);
      p.setClipRegion(bounds & clip);
    }
  } else {
    KstViewLine::paintSelf(p, bounds);
  }

  if (hasArrow()) {
    QPoint to = KstViewLine::to();
    QPoint from = KstViewLine::from();
    const int w = width() * p.lineWidthAdjustmentFactor();
    QPen pen(_foregroundColor, w);

    pen.setCapStyle(capStyle());
    p.setPen(pen);
    p.setBrush(_foregroundColor);

    if (_hasToArrow) {
      paintArrow(p, to, from, w, _toArrowScaling);
    }
    if (_hasFromArrow) {
      paintArrow(p, from, to, w, _fromArrowScaling);
    }
  }
  p.restore();
}


void KstViewArrow::save(QTextStream& ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  ts << indent + "  " << "<orientation>" << _orientation << "</orientation>" << endl;
  KstViewObject::save(ts, indent + "  ");
  ts << indent << "</" << type() << ">" << endl;
}


bool KstViewArrow::hasArrow() const {
  return _hasToArrow || _hasFromArrow;
}


QMap<QString, QVariant> KstViewArrow::widgetHints(const QString& propertyName) const {
  QMap<QString, QVariant> map = KstViewLine::widgetHints(propertyName);
  if (!map.empty()) {
    return map;
  }
  if (propertyName == "hasFromArrow") {
    map.insert(QString("_kst_widgetType"), QString("QCheckBox"));
    map.insert(QString("_kst_label"), QString(""));
    map.insert(QString("text"), tr("Arrow at start"));
  } else if (propertyName == "hasToArrow") {
    map.insert(QString("_kst_widgetType"), QString("QCheckBox"));
    map.insert(QString("_kst_label"), QString(""));
    map.insert(QString("text"), tr("Arrow at end"));
  } else if (propertyName == "fromArrowScaling") {
    map.insert(QString("_kst_widgetType"), QString("KDoubleSpinBox"));
    map.insert(QString("_kst_label"), tr("Start arrow scaling")); 
    map.insert(QString("minValue"), 1.0);
    map.insert(QString("maxValue"), 100.0);
    map.insert(QString("lineStep"), 0.1);
    map.insert(QString("precision"), 1);
  } else if (propertyName == "toArrowScaling") {
    map.insert(QString("_kst_widgetType"), QString("KDoubleSpinBox"));
    map.insert(QString("_kst_label"), tr("End arrow scaling"));
    map.insert(QString("minValue"), 1.0);
    map.insert(QString("maxValue"), 100.0);
    map.insert(QString("lineStep"), 0.1);
    map.insert(QString("precision"), 1);
  }

  return map;
}


bool KstViewArrow::hasFromArrow() const {
  return _hasFromArrow;
}


void KstViewArrow::setHasFromArrow(bool yes) {
  if (_hasFromArrow != yes) {
    _hasFromArrow = yes;
    setDirty();
  }
}


bool KstViewArrow::hasToArrow() const {
  return _hasToArrow;
}


void KstViewArrow::setHasToArrow(bool yes) {
  if (_hasToArrow != yes) {
    _hasToArrow = yes;
    setDirty();
  }
}


double KstViewArrow::fromArrowScaling() const {
  return _fromArrowScaling;
}


void KstViewArrow::setFromArrowScaling(double scaling) {
  if (scaling < 1.0) {
    scaling = 1.0;
  }
  if (_fromArrowScaling != scaling) {
    _fromArrowScaling = scaling;
    setDirty();
  }
}


double KstViewArrow::toArrowScaling() const {
  return _toArrowScaling;
}


void KstViewArrow::setToArrowScaling(double scaling) {
  if (scaling < 1.0) {
    scaling = 1.0;
  }
  if (_toArrowScaling != scaling) {
    _toArrowScaling = scaling;
    setDirty();
  }
}


QRect KstViewArrow::surroundingGeometry() const {
  QRect geom(geometry());

  if (_hasFromArrow || _hasToArrow) {
    double scaling;
    if (_hasFromArrow && _hasToArrow) {
      scaling = qMax(_fromArrowScaling, _toArrowScaling);
    } else if (_hasFromArrow) {
      scaling = _fromArrowScaling;
    } else {
      scaling = _toArrowScaling;
    }
    geom.setLeft(geom.left() - int( SIZE_ARROW * scaling * double(width()/2.0) ) - 1);
    geom.setRight(geom.right() + int( SIZE_ARROW * scaling * double(width()/2.0) ) + 1);
    geom.setTop(geom.top() - int( SIZE_ARROW * scaling * double(width()/2.0) ) - 1);
    geom.setBottom(geom.bottom() + int( SIZE_ARROW * scaling * double(width()/2.0) ) + 1);
  } else {
    geom = KstViewLine::surroundingGeometry();
  }

  return geom;
}


namespace {
KstViewObject *create_KstViewArrow() {
  return new KstViewArrow;
}


KstGfxMouseHandler *handler_KstViewArrow() {
  return new KstGfxArrowMouseHandler;
}

KST_REGISTER_VIEW_OBJECT(Arrow, create_KstViewArrow, handler_KstViewArrow)
}


#include "kstviewarrow.moc"

