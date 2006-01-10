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

#include "editviewobjectdialog.h" 
#include "kstgfxrectanglemousehandler.h"
#include "kst.h"
#include "kstviewbox.h"
#include "kstviewobjectfactory.h"

#include <kglobal.h>
#include <klocale.h>

#include <qmetaobject.h>
#include <qpainter.h>
#include <qmap.h>
#include <qpair.h>

KstViewBox::KstViewBox()
: KstBorderedViewObject("Box") {
  _xRound = 0;
  _yRound = 0;
  _cornerStyle = Qt::MiterJoin;
  _fallThroughTransparency = false;
  setTransparent(true);
  _transparentFill = false;
  setFollowsFlow(true);
}


KstViewBox::KstViewBox(const QDomElement& e)
: KstBorderedViewObject(e) {
  
  QDomNode n = e.firstChild();
  while (!n.isNull()) {
    QDomElement el = n.toElement(); 
    if (!el.isNull()) {
      if (metaObject()->findProperty(el.tagName().latin1(), true) > -1) {
        setProperty(el.tagName().latin1(), QVariant(el.text()));  
      }  
    }
    n = n.nextSibling();      
  }
  
  // these always have these values
  _type = "Box";
  _layoutActions |= Delete | Raise | Lower | RaiseToTop | LowerToBottom | Rename | MoveTo | Copy | CopyTo;
  _fallThroughTransparency = false;
  setTransparent(true);
  setFollowsFlow(true);
}


KstViewBox::~KstViewBox() {
}


// FIXME: this object is trying to paint its own borders for some reason, which
// is very wrong.  It should be calling the proper base class and letting it do
// the border (and margin and padding) calculations.
void KstViewBox::paint(KstPainter& p, const QRegion& bounds) {
  p.save();
  if (p.makingMask()) {
    if (p.type() != KstPainter::P_PRINT && p.type() != KstPainter::P_EXPORT) {
      p.setRasterOp(Qt::SetROP);
    }
    KstViewObject::paint(p, bounds);
  } else {
    KstViewObject::paint(p, bounds);
    // FIXME: inefficient
    if (p.type() != KstPainter::P_PRINT && p.type() != KstPainter::P_EXPORT) {
      QRegion boundary = bounds & _lastClipRegion;
      for (KstViewObjectList::Iterator i = _children.begin(); i != _children.end(); ++i) {
        boundary -= (*i)->clipRegion();
      }
      boundary -= p.uiMask();
      p.setClipRegion(boundary);
    }
  }
  
  QPen pen(borderColor(), borderWidth());
  pen.setJoinStyle(_cornerStyle);
  if (borderWidth() == 0) {
    pen.setStyle(Qt::NoPen);
  }
  p.setPen(pen);
  if (_transparentFill) {
    p.setBrush(Qt::NoBrush);  
  } else {
    p.setBrush(_foregroundColor);
  }
  QRect r;
  r.setX(_geom.left() + borderWidth()/2);
  r.setY(_geom.top() + borderWidth()/2);
  r.setWidth(_geom.width() - borderWidth());
  r.setHeight(_geom.height() - borderWidth());

  p.drawRoundRect(r, _xRound, _yRound);
  p.restore();
}


void KstViewBox::save(QTextStream& ts, const QString& indent) {
  ts << indent << "<" << type() << ">" << endl;
  KstBorderedViewObject::save(ts, indent + "  ");
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


void KstViewBox::setCornerStyle(Qt::PenJoinStyle style) {
  if (_cornerStyle != style) {
    setDirty();
    _cornerStyle = style;
  }
}


Qt::PenJoinStyle KstViewBox::cornerStyle() const {
  return _cornerStyle;
}


bool KstViewBox::transparentFill() const {
  return _transparentFill;
}


void KstViewBox::setTransparentFill(bool yes) {
  if (_transparentFill != yes) {
    setDirty();
    _transparentFill = yes;
  }
}


QMap<QString, QVariant > KstViewBox::widgetHints(const QString& propertyName) const {
  QMap<QString, QVariant> map = KstBorderedViewObject::widgetHints(propertyName);
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
    map.insert(QString("_kst_label"), QString::null);
    map.insert(QString("text"), i18n("Transparent fill"));
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
// vim: ts=2 sw=2 et
