/***************************************************************************
                          kstborderedviewobject.cpp
                             -------------------
    begin                : Mar 11, 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "kstborderedviewobject.h"

#include <qpainter.h>
#include <qstylesheet.h>

KstBorderedViewObject::KstBorderedViewObject(const QString& type)
: KstViewObject(type), _borderColor(QColor(0, 0, 0)), _borderWidth(0), _padding(0), _margin(0) {
}


KstBorderedViewObject::KstBorderedViewObject(const QDomElement& e)
: KstViewObject(e), _borderColor(QColor(0, 0, 0)), _borderWidth(0), _padding(0), _margin(0) {
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement el = n.toElement(); // try to convert the node to an element.
    if( !el.isNull() ) { // the node was really an element.
      if (el.tagName() == "border") {
        _borderColor.setNamedColor(el.attribute( "color", "#7f0000" ));
        _borderWidth = el.attribute( "width", "0" ).toInt();
        _padding = el.attribute( "padding", "0" ).toInt();
        _margin = el.attribute( "margin", "0" ).toInt();
      }
    }
    n = n.nextSibling();
  }
}


KstBorderedViewObject::KstBorderedViewObject(const KstBorderedViewObject& borderedViewObject)
: KstViewObject(borderedViewObject) {
  setBorderColor(borderedViewObject.borderColor());
  setBorderWidth(borderedViewObject.borderWidth());
  setMargin(borderedViewObject.margin());
  setPadding(borderedViewObject.padding());
}


KstBorderedViewObject::~KstBorderedViewObject() {
}


void KstBorderedViewObject::save(QTextStream& ts, const QString& indent) {
  ts << indent << "<border color=\""
    << QStyleSheet::escape(_borderColor.name())
    << "\" width=\"" << _borderWidth
    << "\" padding=\"" << _padding
    << "\" margin=\"" << _margin << "\" />" << endl;
  KstViewObject::save(ts, indent);
}


void KstBorderedViewObject::saveTag(QTextStream& ts, const QString& indent) {
  ts << indent << "<border color=\""
    << QStyleSheet::escape(_borderColor.name())
    << "\" width=\"" << _borderWidth
    << "\" padding=\"" << _padding
    << "\" margin=\"" << _margin << "\" />" << endl;
  KstViewObject::saveTag(ts, indent);
}


void KstBorderedViewObject::paint(KstPaintType type, QPainter& p) {
  KstViewObject::paint(type, p);
  if (_borderWidth > 0 && !_focus && !_selected) {
    QRect r;
    QPen pen(_borderColor, _borderWidth);

    p.setPen(pen);
    r.setX(_geom.left() + _margin);
    r.setY(_geom.top() + _margin);
    r.setWidth(_geom.width() - 2 * _margin);
    r.setHeight(_geom.height() - 2 * _margin);
    p.drawRect(r);
  }
}


void KstBorderedViewObject::setBorderColor(const QColor& c) {
  _borderColor = c;
}


const QColor& KstBorderedViewObject::borderColor() const {
  return _borderColor;
}


void KstBorderedViewObject::setBorderWidth(int w) {
  _borderWidth = w;
}


int KstBorderedViewObject::borderWidth() const {
  return _borderWidth;
}


void KstBorderedViewObject::setMargin(int w) {
  _margin = w;
}


int KstBorderedViewObject::margin() const {
  return _margin;
}


void KstBorderedViewObject::setPadding(int p) {
  _padding = p;
}


int KstBorderedViewObject::padding() const {
  return _padding;
}


QRect KstBorderedViewObject::contentsRect() const {
  QRect rc;
  int mpb = _margin + _padding + _borderWidth;
  rc.setX(_geom.left() + mpb);
  rc.setY(_geom.top() + mpb);
  rc.setWidth(_geom.width() + 2 * mpb);
  rc.setHeight(_geom.height() + 2 * mpb);
  return rc;
}


void KstBorderedViewObject::setContentsRect(QRect& rect) {
  int mpb = _margin + _padding + _borderWidth;
  _geom.setX(rect.left() - mpb);
  _geom.setY(rect.top() - mpb);
  _geom.setWidth(rect.width() - 2 * mpb);
  _geom.setHeight(rect.height() - 2 * mpb);
}


void KstBorderedViewObject::writeBinary(QDataStream& str) {
  KstViewObject::writeBinary(str);
  str << _borderColor << _borderWidth << _padding << _margin;
}


void KstBorderedViewObject::readBinary(QDataStream& str) {
  KstViewObject::readBinary(str);
  str >> _borderColor >> _borderWidth >> _padding >> _margin;
}


#include "kstborderedviewobject.moc"
// vim: ts=2 sw=2 et
