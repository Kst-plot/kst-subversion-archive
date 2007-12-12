/***************************************************************************
                          kstscalarlistview.cpp
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

#include "klocale.h"

#include "ksdebug.h"
#include "kstscalar.h"

#include "kstscalarlistview.h"

KstScalarListViewItem::KstScalarListViewItem(KstScalarListView *parent, KstObjectTreeNode<KstScalar> *node) : KListViewItem(parent), _node(node) {
  commonConstructor();
}

KstScalarListViewItem::KstScalarListViewItem(KstScalarListViewItem *parent, KstObjectTreeNode<KstScalar> *node) : KListViewItem(parent), _node(node) {
  commonConstructor();
}

void KstScalarListViewItem::commonConstructor() {
  if (_node) {
    KstScalar *s = dynamic_cast<KstScalar*>(_node->object());
    if (s && s->editable()) {
      setRenameEnabled(0, false);
      setRenameEnabled(1, true);
    } else {
      setRenameEnabled(0, false);
      setRenameEnabled(1, false);
    }
  }
  _remove = false;
}

QString KstScalarListViewItem::text(int column) const {
  if (!_node) {
    return QString::null;
  }

  switch (column) {
    case 0:
      return _node->nodeTag();
    case 1:
      {
        KstScalar *s = dynamic_cast<KstScalar*>(_node->object());
        if (s) {
          return s->label();
        } else {
          return QString::null;
        }
      }
    default:
      return QString::null;
  }
}

void KstScalarListViewItem::setText(int column, const QString& text) {
  if (column == 1 && _node) {
    KstScalar *s = dynamic_cast<KstScalar*>(_node->object());
    if (s && s->editable()) {
      bool ok;
      double val = text.toDouble(&ok);
      if (ok) {
        s->setValue(val);
      }
    }
  }
}

bool KstScalarListViewItem::remove() const {
  return _remove;
}

void KstScalarListViewItem::setRemove(bool remove) {
  _remove = remove;
}

KstScalarListView::KstScalarListView(QWidget *parent, KstObjectCollection<KstScalar> *coll) : KListView(parent), _coll(coll) {
  addColumn(i18n("Scalar"));
  addColumn(i18n("Value"));

  setRootIsDecorated(true);
  setAllColumnsShowFocus(true);

  update();
}

void KstScalarListView::addChildItems(KstScalarListViewItem *parentItem, KstObjectTreeNode<KstScalar> *parentNode) {
  if (!parentItem || !parentNode) {
    return;
  }

  QValueList<KstObjectTreeNode<KstScalar>*> children = parentNode->children().values();
  for (QValueList<KstObjectTreeNode<KstScalar>*>::ConstIterator i = children.begin(); i != children.end(); ++i) {
    QListViewItem *item = parentItem->firstChild();
    bool found = false;

    while (item) {
      if (item->text(0) == (*i)->nodeTag()) {
        found = true;

        KstScalarListViewItem *kItem = dynamic_cast<KstScalarListViewItem*>(item);
        if (kItem) {
          kItem->setRemove(false);
          repaintItem(kItem);
          addChildItems(kItem, *i);
        }

        break;
      }
      item = item->nextSibling();
    }

    if (!found) {
      KstScalarListViewItem *item = new KstScalarListViewItem(parentItem, *i);
      addChildItems(item, *i);
    }
  }
}

void KstScalarListView::update() {
  if (_coll) {
    KstReadLocker(&_coll->lock());

    {
      QListViewItemIterator it(this);

      while (it.current()) {
        KstScalarListViewItem *kItem = dynamic_cast<KstScalarListViewItem*>(it.current());
        if (kItem) {
          kItem->setRemove(true);
        }
        ++it;
      }
    }

    QValueList<KstObjectTreeNode<KstScalar>*> rootItems = _coll->nameTreeRoot()->children().values();
    for (QValueList<KstObjectTreeNode<KstScalar>*>::ConstIterator i = rootItems.begin(); i != rootItems.end(); ++i) {
      QListViewItem *item = firstChild();
      bool found = false;

      while (item) {
        if (item->text(0) == (*i)->nodeTag()) {
          found = true;

          KstScalarListViewItem *kItem = dynamic_cast<KstScalarListViewItem*>(item);
          if (kItem) {
            kItem->setRemove(false);
            repaintItem(kItem);
            addChildItems(kItem, *i);
          }

          break;
        }
        item = item->nextSibling();
      }

      if (!found) {
        KstScalarListViewItem *item = new KstScalarListViewItem(this, *i);
        addChildItems(item, *i);
      }
    }

    {
      QListViewItemIterator it(this);

      while (it.current()) {
        KstScalarListViewItem *kItem = dynamic_cast<KstScalarListViewItem*>(it.current());
        if (kItem) {
          if (kItem->remove()) {
            delete it.current();
          } else {
            ++it;
          }
        } else {
          ++it;
        }
      }
    }
  }

/*
  kstdDebug() << "Updated KstScalarListView: now " << childCount() << " root-child items" << endl;
  QListViewItemIterator it(this);
  while (it.current()) {
    KstScalarListViewItem *item = dynamic_cast<KstScalarListViewItem*>(it.current());
    if (item) {
      kstdDebug() << "  " << item->node()->fullTag().join(KstObjectTag::tagSeparator) << ": " << item->text(0) << ", " << item->text(1) << endl;
    }
    ++it;
  }
*/

  KListView::update();
}

