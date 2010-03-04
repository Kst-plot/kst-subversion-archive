/***************************************************************************
                          kstscalarlistview.h  -  description
                             -------------------
    begin                : Tue Nov 21 2006
    copyright            : (C) 2006 University of Toronto
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

#ifndef KSTSCALARLISTVIEW_H
#define KSTSCALARLISTVIEW_H

#include <klistview.h>

#include "kstobject.h"
#include "kstobjectcollection.h"

class KstScalarListViewItem;

class KstScalarListView : public KListView
{
  public:
    KstScalarListView(QWidget *parent = 0, KstObjectCollection<KstScalar> *coll = NULL);

    void update();

  private:
    void addChildItems(KstScalarListViewItem *parentItem, KstObjectTreeNode<KstScalar> *parentNode);

    KstObjectCollection<KstScalar> *_coll;
};


class KstScalarListViewItem : public KListViewItem
{
  public:
    KstScalarListViewItem(KstScalarListView *parent, KstObjectTreeNode<KstScalar> *node);
    KstScalarListViewItem(KstScalarListViewItem *parent, KstObjectTreeNode<KstScalar> *node);

    QString text(int column) const;
    void setText(int column, const QString& text);

    bool remove() const;
    void setRemove(bool remove);

    KstObjectTreeNode<KstScalar> *node() const { return _node; }

  private:
    void commonConstructor();

    QPointer<KstObjectTreeNode<KstScalar> > _node;
    bool _remove;
};

#endif

