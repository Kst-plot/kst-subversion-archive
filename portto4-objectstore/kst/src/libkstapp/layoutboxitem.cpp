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

#include "layoutboxitem.h"

#include "viewgridlayout.h"
#include "viewitemzorder.h"

#include "debug.h"
#include <QDebug>
#include <QMenu>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>

namespace Kst {

LayoutBoxItem::LayoutBoxItem(View *parent)
    : ViewItem(parent) {
  setName("Layout Box");
  setZValue(LAYOUTBOX_ZVALUE);
  setPen(Qt::NoPen);
  setBrush(Qt::NoBrush);
  setAllowedGripModes(Move);

  parentView()->scene()->addItem(this);
  //setZValue(1);
  setPos(parentView()->sceneRect().topLeft());
  setViewRect(parentView()->sceneRect());

  setEnabled(true);
}


void LayoutBoxItem::appendItem(ViewItem *item) {
  if (item->parentItem() == this)
    return;

  item->setParentItem(this);
  layout()->addViewItem(item, layout()->rowCount(), 0, 1, layout()->columnCount());
  setPos(parentView()->sceneRect().topLeft());
  setViewRect(parentView()->sceneRect());
  //singleshot to give plots a chance to set a projection rect
  QTimer::singleShot(0, layout(), SLOT(update()));
}


void LayoutBoxItem::save(QXmlStreamWriter &xml) {
  xml.writeStartElement("layoutbox");
  ViewItem::save(xml);

  QList<QGraphicsItem*> list = QGraphicsItem::children();
  foreach (QGraphicsItem *item, list) {
    ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(item);
    if (!viewItem)
      continue;

    xml.writeStartElement("layoutitem");
    //TODO Update this with proper Object Tag's.
    xml.writeAttribute("name", viewItem->name());
    xml.writeEndElement();
  }

  xml.writeEndElement();
}


void LayoutBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  Q_UNUSED(option);
  Q_UNUSED(widget);

  painter->setPen(pen());
  painter->setBrush(brush());
  painter->drawRect(rect());
}


LayoutBoxItem::~LayoutBoxItem() {
}


void LayoutBoxItem::setEnabled(bool enabled) {
  if (!parentView())
    return;

  if (enabled) {
    QList<QGraphicsItem*> list = parentView()->items();
    foreach (QGraphicsItem *item, list) {
      ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(item);
      if (!viewItem || viewItem->parentItem() || !viewItem->isVisible() || viewItem == this)
        continue;

      viewItem->setParentItem(this);
    }

    parentView()->setLayoutBoxItem(this);

    show();
  } else {
    QList<QGraphicsItem*> list = QGraphicsItem::children();
    foreach (QGraphicsItem *item, list) {
      ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(item);
      if (!viewItem)
        continue;

      viewItem->setParentItem(0);
    }

    parentView()->setLayoutBoxItem(0);

    hide();
  }
}


void LayoutBoxItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  QMenu menu;

  addTitle(&menu);

  QAction *editAction = menu.addAction(tr("Edit"));
  connect(editAction, SIGNAL(triggered()), this, SLOT(edit()));

  QAction *breakLayoutAction = menu.addAction(tr("Break layout"));
  connect(breakLayoutAction, SIGNAL(triggered()), this, SLOT(breakLayout()));

  menu.exec(event->screenPos());
}


void LayoutBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  event->ignore();
}


bool LayoutBoxItem::appendItemFromXml(QXmlStreamReader &xml) {
  bool validTag = false;
  if (xml.isStartElement() && xml.name().toString() == "layoutitem") {
    QXmlStreamAttributes attrs = xml.attributes();
    QStringRef av;
    av = attrs.value("name");
    if (!av.isNull()) {
      //TODO Use ObjectTag to locate the correct item and add it to this layout.
    }
    xml.readNext();
    if (xml.isEndElement() && xml.name().toString() == "layoutitem") {
      validTag = true;
    }
  }
  return validTag;
}


LayoutBoxItemFactory::LayoutBoxItemFactory()
: GraphicsFactory() {
  registerFactory("layoutbox", this);
}


LayoutBoxItemFactory::~LayoutBoxItemFactory() {
}


ViewItem* LayoutBoxItemFactory::generateGraphics(QXmlStreamReader& xml, View *view, ViewItem *parent) {
  LayoutBoxItem *rc = 0;
  while (!xml.atEnd()) {
    bool validTag = true;
    if (xml.isStartElement()) {
      if (xml.name().toString() == "layoutbox") {
        Q_ASSERT(!rc);
        rc = new LayoutBoxItem(view);
        if (parent) {
          rc->setParentItem(parent);
        }
        // TODO add any specialized BoxItem Properties here.
      } else if (xml.name().toString() == "layoutitem") {
        Q_ASSERT(rc);
        validTag = rc->appendItemFromXml(xml);
      } else {
        Q_ASSERT(rc);
        if (!rc->parse(xml, validTag) && validTag) {
          ViewItem *i = GraphicsFactory::parse(xml, view, rc);
          if (!i) {
          }
        }
      }
    } else if (xml.isEndElement()) {
      if (xml.name().toString() == "layoutbox") {
        break;
      } else {
        validTag = false;
      }
    }
    if (!validTag) {
      qDebug("invalid Tag\n");
      Debug::self()->log(QObject::tr("Error creating layoutbox object from Kst file."), Debug::Warning);
      delete rc;
      return 0;
    }
    xml.readNext();
  }
  //TODO LayoutBoxItem automatically adds itself to the parent. Don't return the item here as it
  // has already been added to the scene.  
  return 0;
}

}

// vim: ts=2 sw=2 et
