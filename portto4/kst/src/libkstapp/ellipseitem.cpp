/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ellipseitem.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>

namespace Kst {

EllipseItem::EllipseItem(View *parent)
  : ViewItem(parent) {
}


EllipseItem::~EllipseItem() {
}


void EllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  const qreal w = pen().widthF();
  painter->drawEllipse(rect().adjusted(w, w, -w, -w));
  QPen p = pen();
  setPen(Qt::NoPen);
  QGraphicsRectItem::paint(painter, option, widget);
  setPen(p);
}


void CreateEllipseCommand::createItem() {
  _item = new EllipseItem(_view);
  _view->setMouseMode(View::Create);
  _view->setCursor(Qt::CrossCursor);

  //If the mouseMode is changed again before we're done with creation
  //delete ourself.
  connect(_view, SIGNAL(mouseModeChanged()), _item, SLOT(deleteLater()));
  connect(_view, SIGNAL(creationPolygonChanged(View::CreationEvent)),
          _item, SLOT(creationPolygonChanged(View::CreationEvent)));
  connect(_item, SIGNAL(creationComplete()), this, SLOT(creationComplete()));
  //If the item is interrupted while creating itself it will destroy itself
  //need to delete this too in response...
  connect(_item, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
}

}

#include "ellipseitem.moc"

// vim: ts=2 sw=2 et
