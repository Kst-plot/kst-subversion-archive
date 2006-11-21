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

class KstScalarListView : public KListView
{
  public:
    KstScalarListView(QWidget *parent = 0, KstObjectTree *tree = NULL);

    void update();

  private:
    KstObjectTree *_tree;
};


class KstScalarListViewItem : public KListViewItem
{
  public:
    KstScalarListViewItem(KstScalarListView *parent, KstObjectTreeNode *node);
    KstScalarListViewItem(KstScalarListViewItem *parent, KstObjectTreeNode *node);

    QString text(int column) const;

    KstObjectTreeNode *node() const { return _node; }

  private:
    KstObjectTreeNode *_node;
};

#endif
// vim: ts=2 sw=2 et
