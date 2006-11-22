/***************************************************************************
                          kststringlistview.h  -  description
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

#ifndef KSTSTRINGLISTVIEW_H
#define KSTSTRINGLISTVIEW_H

#include <klistview.h>

#include "kstobject.h"

class KstStringListView : public KListView
{
  public:
    KstStringListView(QWidget *parent = 0, KstObjectTree *tree = NULL);

    void update();

  private:
    KstObjectTree *_tree;
};


class KstStringListViewItem : public KListViewItem
{
  public:
    KstStringListViewItem(KstStringListView *parent, KstObjectTreeNode *node);
    KstStringListViewItem(KstStringListViewItem *parent, KstObjectTreeNode *node);

    QString text(int column) const;

    KstObjectTreeNode *node() const { return _node; }

  private:
    KstObjectTreeNode *_node;
};

#endif
// vim: ts=2 sw=2 et
