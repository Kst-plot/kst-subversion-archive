/***************************************************************************
                       kstviewmanger_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of British Columbia
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
#include <assert.h>

// include files for Qt
#include <qptrstack.h>

// include files for KDE
#include "ksdebug.h"
#include <klistview.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// application specific includes
#include "kstdatacollection.h"
#include "kstobject.h"
#include "kstviewmanager_i.h"
#include "kstdoc.h"
#include "kstviewwindow.h"


#define RTTI_OBJ_WINDOW          4301
#define RTTI_OBJ_2D_PLOT         4302
#define RTTI_OBJ_PLOT_GROUP      4303
#define RTTI_OBJ_VIEW_OBJECT     4304


KstViewObjectItem::KstViewObjectItem(QListView *parent, KstTopLevelViewPtr x, KstViewManagerI *vm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_WINDOW), _name(x->tagName()), _vm(vm) {
  assert(x);
  _inUse = false;
  setText(0, x->name());
  setText(1, x->type());
  update(KstViewObjectPtr(x), true, localUseCount);
}


KstViewObjectItem::KstViewObjectItem(QListViewItem *parent, KstViewObjectPtr x, KstViewManagerI *vm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_VIEW_OBJECT), _name(x->tagName()), _vm(vm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, x->type());
  update(KstViewObjectPtr(x), true, localUseCount);
}

KstViewObjectItem::~KstViewObjectItem() {
}

void KstViewObjectItem::update(KstViewObjectPtr x, bool recursive, int localUseCount) {
  QPtrStack<QListViewItem> trash;
  
  // garbage collect first
  for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
    bool found = false;
    KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
    if (i->rtti() == RTTI_OBJ_VIEW_OBJECT) {
      for (KstViewObjectList::ConstIterator obj = x->children().begin(); obj != x->children().end(); ++obj) {
        if (x->tagName() == oi->tagName()) {
          found = true;
          break;
        }
      }
    }
    if (!found) {
      trash.push(i);
    }
  }
  
  trash.setAutoDelete(true);
  _vm->blockSignals(true);
  trash.clear();
  _vm->blockSignals(false);
  
  for (KstViewObjectList::ConstIterator obj = x->children().begin(); obj != x->children().end(); ++obj) {
    bool found = false;
    for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
      KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
      if (oi->tagName() == (*obj)->tagName()) {
        oi->update(*obj);
        found = true;
        break;
      }
    }
    if (!found) {
      KstViewObjectItem *i = new KstViewObjectItem(this, *obj, _vm);
    }
  } 
}


void KstViewObjectItem::updateButtons() {
  _vm->Edit->setEnabled(_rtti != RTTI_OBJ_WINDOW);
  _vm->Delete->setEnabled(true);
}


KstViewManagerI::KstViewManagerI(KstDoc *in_doc, QWidget* parent, const char* name, bool modal, WFlags fl)
: KstViewManager(parent, name, modal, fl) {
  doc = in_doc;

  connect(Close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(ViewView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(edit_I()));
  connect(ViewView, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(currentChanged(QListViewItem *)));
  connect(ViewView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(currentChanged(QListViewItem *)));
  connect(ViewView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(contextMenu(QListViewItem*, const QPoint&, int)));
}


KstViewManagerI::~KstViewManagerI() {
}


void KstViewManagerI::show_I() {
  show();
  raise();
  update();
}


void KstViewManagerI::updateContents() {
  update();
}


void KstViewManagerI::update() {
  KstApp *app = KstApp::inst();
    
  if (!isShown()) {
    return;
  }

  QListViewItem *currentItem = ViewView->selectedItem();
  QPtrStack<QListViewItem> trash;
  KMdiIterator<KMdiChildView*> *it = app->createIterator();

  // garbage collect first
  for (QListViewItem *i = ViewView->firstChild(); i; i = i->nextSibling()) {
    bool found = false;
    KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
    it->first();
    if (i->rtti() == RTTI_OBJ_WINDOW) {
      while (it->currentItem()) {      
        KstViewWindow *win = dynamic_cast<KstViewWindow*>(it->currentItem());
        if (win) {
          KstTopLevelViewPtr view = win->view();
          if (view) {
            if (view->tagName() == oi->tagName()) {
              found = true;
            }
          }
        }
        it->next();
      }
    }
    if (!found) {
      trash.push(i);
    }
  }

  trash.setAutoDelete(true);
  ViewView->blockSignals(true);
  trash.clear();
  ViewView->blockSignals(false);
  
  it->first();
  while (it->currentItem()) {      
    KstViewWindow *win = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (win) {
      KstTopLevelViewPtr  view = win->view();
      if (view) {
        bool found = false;

        for (QListViewItem *i = ViewView->firstChild(); i; i = i->nextSibling()) {
          KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
          if (oi->rtti() == RTTI_OBJ_WINDOW && oi->tagName() == view->tagName()) {
            oi->update(KstViewObjectPtr(view));
            found = true;
            break;
          }
        }
        if (!found) {
          KstViewObjectItem *i = new KstViewObjectItem(ViewView, view, this);
        }
      }
    }
    it->next();
  }

  for (QListViewItem *i = ViewView->firstChild(); i; i = i->nextSibling()) {
    if (i == currentItem) {
      ViewView->setCurrentItem(i);
      ViewView->setSelected(i, true);
      break;
    }
  }
  
  if (ViewView->selectedItem()) {
    static_cast<KstViewObjectItem*>(ViewView->currentItem())->updateButtons();
  } else {
    Edit->setEnabled(false);
    Delete->setEnabled(false);
  }
  
  app->deleteIterator(it);
}


void KstViewManagerI::edit_I() {
  QListViewItem *qi;

  if (ViewView->selectedItems().count() > 0) {
    qi = ViewView->selectedItems().at(0);
  } else {
    KMessageBox::sorry(this, i18n("An item must be selected to edit."));
    return;
  }
}


void KstViewManagerI::delete_I() {
  QListViewItem *qi = ViewView->selectedItems().at(0);
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi->removable()) {
    update();
  } else {
  }
}


// Menu IDs:
// 100->499 reserved for plots
// 500->999 reserved for filters

void KstViewManagerI::contextMenu(QListViewItem *i, const QPoint& p, int col) {
  Q_UNUSED(col)

  if (!i) {
    return;
  }

  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(i);
  KPopupMenu *m = new KPopupMenu(this);

  m->insertTitle(koi->text(0));
  
  m->popup(p);
}


void KstViewManagerI::doUpdates() {
  emit docChanged();
}


void KstViewManagerI::currentChanged(QListViewItem *i) {
  if (i) {
    KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(i);
    koi->updateButtons();
  }
}

#include "kstviewmanager_i.moc"
// vim: ts=2 sw=2 et
