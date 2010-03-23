/***************************************************************************
                               kstviewbox.cpp
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

#include "kstgfxrectanglemousehandler.h"
#include "kst.h"
#include "kstviewbox.h"
#include "kstviewobjectfactory.h"

#include <kglobal.h>
#include <klocale.h>

#include <qbitmap.h>
#include <qmetaobject.h>
#include <qpainter.h>
#include <qmap.h>
#include <qpair.h>

KstViewBox::KstViewBox()
: KstViewObject("Box"), _borderColor(QColor(0, 0, 0)), _borderWidth(0) {
  _editTitle = i18n("Edit Box");
  _newTitle = i18n("New Box");
  _xRound = 0;
  _yRound = 0;
  _cornerStyle = Qt::MiterJoin;
  _fallThroughTransparency = false;
  setTransparent(false);
  setFollowsFlow(true);
  _standardActions |= Delete | Edit;
}


KstViewBox::KstViewBox(const QDomElement& e)
: KstViewObject(e), _borderColor(QColor(0, 0, 0)), _borderWidth(0) {
  _xRound = 0;
  _yRound = 0;
  _cornerStyle = Qt::MiterJoin;
  setTransparent(false);

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

  // these always have these values
  _type = "Box";
  _editTitle = i18n("Edit Box");
  _newTitle = i18n("New Box");
  _standardActions |= Delete | Edit;
  _layoutActions |= Delete | Raise | Lower | RaiseToTop | LowerToBottom | Rename | MoveTo | Copy | CopyTo;
  _fallThroughTransparency = false;
  setFollowsFlow(true);
}


KstViewBox::KstViewBox(const KstViewBox& box)
: KstViewObject(box) {
  _standardActions |= Delete | Edit;
  _xRound = box._xRound;
  _yRound = box._xRound;
  _cornerStyle = box._cornerStyle;
  _borderColor = box._borderColor;
  _borderWidth = box._borderWidth;

  // these always have these values
  _type = "Box";
}


KstViewBox::~KstViewBox() {
}


KstViewObject* KstViewBox::copyObjectQuietly(KstViewObject& parent, const QString& name) const {
  Q_UNUSED(name)

  KstViewBox *viewBox = new KstViewBox(*this);
  parent.appendChild(KstViewObjectPtr(viewBox), true);

  return viewBox;
}


KstViewObject* KstViewBox::copyObjectQuietly() const {
  KstViewBox *viewBox = new KstViewBox(*this);

  return viewBox;
}


void KstViewBox::paintSelf(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.type() != KstPainter::P_PRINT && p.type() != KstPainter::P_EXPORT) {
    if (p.makingMask()) {
// xxx      p.setRasterOp(Qt::SetROP);
    } else {
      const QRegion clip(clipRegion());
      KstViewObject::paintSelf(p, bounds - clip);
      p.setClipRegion(bounds & clip);
    }
  }

  // restrict the border width so we do not draw outside of the rectangle itself
  int borderWidthAdjusted(borderWidth() * p.lineWidthAdjustmentFactor());
  if (borderWidthAdjusted > _geom.width() / 2) {
    borderWidthAdjusted = _geom.width() / 2;
  }
  if (borderWidthAdjusted > _geom.height()) {
    borderWidthAdjusted = _geom.height() / 2;
  }

  QRect r;
  QPen pen(borderColor(), borderWidthAdjusted);

  pen.setJoinStyle(_cornerStyle);
  if (borderWidthAdjusted == 0) {
    pen.setStyle(Qt::NoPen);
  }
  p.setPen(pen);

  if (_transparent) {
    p.setBrush(Qt::NoBrush);
  } else {
    p.setBrush(_foregroundColor);
  }

  r.setX(_geom.left() + ( borderWidthAdjusted / 2 ));
  r.setY(_geom.top() + ( borderWidthAdjusted / 2 ));
  r.setWidth(_geom.width() - borderWidthAdjusted + 1);
  r.setHeight(_geom.height() - borderWidthAdjusted + 1);

  p.drawRoundRect(r, _xRound, _yRound);
  p.restore();
}


QRegion KstViewBox::clipRegion() {
  if (_clipMask.isEmpty()) {
    if (transparent() || _xRound != 0 || _yRound != 0) {
      QBitmap bm(_geom.bottomRight().x() + 1, _geom.bottomRight().y() + 1);

      if (!bm.isNull()) {
        KstPainter p;

        p.begin(&bm);
        p.setMakingMask(true);
// xxx        p.setViewXForm(true);
        paint(p, QRegion());
// xxx        p.flush();
        p.end();
        _clipMask = QRegion(bm);
      } else {
        _clipMask = QRegion(); // only invalidate our own variable
      }
    } else {
      _clipMask = QRegion(_geom);
    }
  }

  return _clipMask;
}


void KstViewBox::save(QTextStream& ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  KstViewObject::save(ts, indent + "  ");
  ts << indent << "</" << type() << ">" << endl;
}


void KstViewBox::setXRound(int rnd) {
  int crnd;

  if (rnd < 0) {
    crnd = 0;
  } else if (rnd > 99) {
    crnd = 99;
  } else {
    crnd = rnd;
  }

  if (_xRound != crnd) {
    setDirty();
    _xRound = crnd;
  }
}


void KstViewBox::setYRound(int rnd) {
  int crnd;

  if (rnd < 0) {
    crnd = 0;
  } else if (rnd > 99) {
    crnd = 99;
  } else {
    crnd = rnd;
  }

  if (_yRound != crnd) {
    setDirty();
    _yRound = crnd;
  }
}


int KstViewBox::xRound() const {
  return _xRound;
}


int KstViewBox::yRound() const {
  return _yRound;
}


bool KstViewBox::transparent() const {
  return KstViewObject::transparent();
}


void KstViewBox::setTransparent(bool transparent) {
  KstViewObject::setTransparent(transparent);
}


void KstViewBox::setBorderColor(const QColor& c) {
  if (_borderColor != c) {
    setDirty();
    _borderColor = c;
  }
}


const QColor& KstViewBox::borderColor() const {
  return _borderColor;
}


void KstViewBox::setBorderWidth(int w) {
  int mw = qMax(0, w);
  if (_borderWidth != mw) {
    _borderWidth = mw;
    setDirty();
  }
}


int KstViewBox::borderWidth() const {
  return _borderWidth;
}


void KstViewBox::setBackgroundColor(const QColor& color) {
  KstViewObject::setBackgroundColor(color);
}


QColor KstViewBox::backgroundColor() const {
  return KstViewObject::backgroundColor();
}


void KstViewBox::setForegroundColor(const QColor& color) {
  KstViewObject::setForegroundColor(color);
}


QColor KstViewBox::foregroundColor() const {
  return KstViewObject::foregroundColor();
}


QMap<QString, QVariant > KstViewBox::widgetHints(const QString& propertyName) const {
  QMap<QString, QVariant> map = KstViewObject::widgetHints(propertyName);
  if (!map.empty()) {
    return map;
  }

  if (propertyName == "xRound") {
    map.insert(QString("_kst_widgetType"), QString("QSpinBox"));
    map.insert(QString("_kst_label"), i18n("X Roundness"));
    map.insert(QString("minValue"), 0);
  } else if (propertyName == "yRound") {
    map.insert(QString("_kst_widgetType"), QString("QSpinBox"));
    map.insert(QString("_kst_label"), i18n("Y Roundness"));
    map.insert(QString("minValue"), 0);
  } else if (propertyName == "foregroundColor") {
    map.insert(QString("_kst_widgetType"), QString("KColorButton"));
    map.insert(QString("_kst_label"), i18n("Fill Color"));
  } else if (propertyName == "transparentFill") {
    map.insert(QString("_kst_widgetType"), QString("QCheckBox"));
    map.insert(QString("_kst_label"), QString(""));
    map.insert(QString("text"), i18n("Transparent fill"));
  } if (propertyName == "borderColor") {
    map.insert(QString("_kst_widgetType"), QString("KColorButton"));
    map.insert(QString("_kst_label"), i18n("Border color"));
  } else if (propertyName == "borderWidth") {
    map.insert(QString("_kst_widgetType"), QString("QSpinBox"));
    map.insert(QString("_kst_label"), i18n("Border width"));
    map.insert(QString("minValue"), 0);
  }

  return map;
}


namespace {
KstViewObject *create_KstViewBox() {
  return new KstViewBox;
}


KstGfxMouseHandler *handler_KstViewBox() {
  return new KstGfxRectangleMouseHandler;
}

KST_REGISTER_VIEW_OBJECT(Box, create_KstViewBox, handler_KstViewBox)
}


#include "kstviewbox.moc"

