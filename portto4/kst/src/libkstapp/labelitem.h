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

#ifndef LABELITEM_H
#define LABELITEM_H

#include <QGraphicsSimpleTextItem>
#include "viewitem.h"

namespace Kst {

class LabelItem : public ViewItem, public QGraphicsSimpleTextItem
{
  Q_OBJECT
public:
  LabelItem(KstPlotView *parent);
  virtual ~LabelItem();

  virtual QGraphicsItem *graphicsItem() { return this; }

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private Q_SLOTS:
  void creationPolygonChanged(KstPlotView::CreationEvent event);
};


class KST_EXPORT CreateLabelCommand : public CreateCommand
{
public:
  CreateLabelCommand() : CreateCommand(QObject::tr("Create Label")) {}
  CreateLabelCommand(KstPlotView *view): CreateCommand(view, QObject::tr("Create Label")) {}
  virtual ~CreateLabelCommand() {}
  virtual void createItem();
};

}

#endif

// vim: ts=2 sw=2 et
