/***************************************************************************
                       kstviewmanager.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003-2006 The University of British Columbia
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

#include <QStack>
#include <qdragobject.h>
#include <qmessagebox.h>

#include "ksdebug.h"
#include <klistview.h>
#include <kmultipledrag.h>
#include <kstandarddirs.h>

#include "kst.h"
#include "kst2dplot.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstdoc.h"
#include "kstobject.h"
#include "kstviewmanager.h"
#include "kstviewobjectimagedrag.h"
#include "kstviewwidget.h"
#include "kstviewwindow.h"
#include "plotmimesource.h"


#define RTTI_OBJ_WINDOW          4301
#define RTTI_OBJ_VIEW_OBJECT     4302
#define RTTI_OBJ_DATA_OBJECT     4303


KstViewListView::KstViewListView(QWidget *parent, const char *name) : QListView(parent, name) {
}

KstViewListView::~KstViewListView() {
}

QDragObject* KstViewListView::dragObject() {
  QListViewItem *qi = selectedItem();
  QDragObject* drag = 0L;

  if (qi) {
    KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

    if (koi) {
      if (koi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
        KMultipleDrag *multipleDrag = new KMultipleDrag(this);
        QStringList objects;
        KstViewObjectList vol;
        KstViewWindow *win;
        KstViewObjectPtr viewObject = koi->viewObject(&win);

        drag = multipleDrag;

        if (viewObject) {
          objects.append(viewObject->tagName());
          vol.append(viewObject);
          multipleDrag->addDragObject(new PlotMimeSource(win->caption(), objects, 0L));
          KstViewObjectImageDrag *imd = new KstViewObjectImageDrag(this);
          imd->setObjects(vol);
          multipleDrag->addDragObject(imd);
        }
      } else if(koi->rtti() == RTTI_OBJ_DATA_OBJECT) {
        Kst2DPlotPtr plot;
        KstDataObjectPtr dataObject = koi->dataObject(plot);

        if (dataObject) {
          QStoredDrag *storedDrag = new QStoredDrag("application/x-kst-curve-list", this);
          QStringList entries;

          drag = storedDrag;

          entries << dataObject->tagName();

          QByteArray data;
          QDataStream ds(data, IO_WriteOnly);
          ds << entries;
          storedDrag->setEncodedData(data);
        }
      }
    }
  }

  return drag;
}

void KstViewListView::contentsMouseMoveEvent(QMouseEvent *e) {
  viewport()->setCursor(Qt::ArrowCursor);

  QListView::contentsMouseMoveEvent(e);
}

// ==============================================

KstViewObjectItem::KstViewObjectItem(QListView *parent, KstTopLevelViewPtr x, KstViewManagerI *vm, int localUseCount)
: QListViewItem(parent), _rtti(RTTI_OBJ_WINDOW), _name(x->tagName()), _vm(vm) {

  if (x) {
    _inUse = false;
    _removable = true;
    setText(0, x->name());
    setText(1, i18n("Tab"));
    setDragEnabled(false);
    setDropEnabled(true);
    update(KstViewObjectPtr(x), true, localUseCount);
  }
}

KstViewObjectItem::KstViewObjectItem(QListViewItem *parent, KstViewObjectPtr x, KstViewManagerI *vm, int localUseCount)
: QListViewItem(parent), _rtti(RTTI_OBJ_VIEW_OBJECT), _name(x->tagName()), _vm(vm) {

  if (x) {
    _inUse = false;
    _removable = true;
    setText(0, x->tagName());
    setText(1, x->type());
    setDragEnabled(true);
    setDropEnabled(true);
    update(KstViewObjectPtr(x), true, localUseCount);
  }
}

KstViewObjectItem::KstViewObjectItem(QListViewItem *parent, KstBaseCurvePtr x, KstViewManagerI *vm, int localUseCount)
: QListViewItem(parent), _rtti(RTTI_OBJ_DATA_OBJECT), _name(x->tagName()), _vm(vm) {
  Q_UNUSED(localUseCount)

  if (x) {
    _inUse = false;
    _removable = true;
    setText(0, x->tagName());
    setText(1, x->type());
    setDragEnabled(true);
    setDropEnabled(false);
  }
}

KstViewObjectItem::~KstViewObjectItem() {
}

bool KstViewObjectItem::acceptDrop(const QMimeSource *mime) const {
  bool retVal = false;

  if (rtti() == RTTI_OBJ_WINDOW) {
    if (mime->provides(PlotMimeSource::mimeType())) {
      retVal = true;
    }
  } else if (rtti() == RTTI_OBJ_VIEW_OBJECT) {
    KstViewWindow *win;
    KstViewObjectPtr obj = viewObject(&win);

    if (mime->provides(PlotMimeSource::mimeType())) {
      if (obj && obj->isContainer()) {
        retVal = true;
      }
    } else {
      if (obj) {
        if (mime->provides("application/x-kst-curve-list")) {
          if (dynamic_cast<Kst2DPlot*>(obj.data())) {
            retVal = true;
          } else if (dynamic_cast<KstViewLegend*>(obj.data())) {
            retVal = true;
          }
        }
      }
    }
  }

  //
  // the following should not be necessary but is due to a bug in QListView...
  //
  if (retVal) {
    _vm->ViewView->viewport()->setCursor(Qt::ArrowCursor);
  } else {
    _vm->ViewView->viewport()->setCursor(Qt::ForbiddenCursor);
  }

  return retVal;
}

void KstViewObjectItem::dropped(QDropEvent *e) {
  bool accepted = false;

  if (rtti() == RTTI_OBJ_WINDOW) {
    KstViewWindow *win;

    viewObject(&win);
    if (win) {
      win->view()->widget()->dropped(e);
      accepted = true;
    }
  } else if (rtti() == RTTI_OBJ_VIEW_OBJECT) {
    KstViewWindow *win;
    KstViewObjectPtr obj = viewObject(&win);

    if (win) {
      if (e->provides(PlotMimeSource::mimeType())) {
        KstViewObjectList list;
        bool added = false;

        if (obj->paste(e, &list)) {
          for (KstViewObjectList::Iterator it = list.begin(); it != list.end(); ++it) {
            new KstViewObjectItem(this, *it, _vm);
            added = true;
          }
          if (added) {
            win->view()->paint(KstPainter::P_PAINT);
          }
          accepted = true;
        }
      } else if (obj && e->provides("application/x-kst-curve-list")) {
        Kst2DPlotPtr plot = dynamic_cast<Kst2DPlot*>(obj.data());
        KstViewLegendPtr legend = dynamic_cast<KstViewLegend*>(obj.data());
        if (plot || legend) {
          KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
          QByteArray data = e->encodedData("application/x-kst-curve-list");
          QDataStream ds(data, IO_ReadOnly);
          QStringList entries;
          bool added = false;

          ds >> entries;
          for (QStringList::ConstIterator i = entries.begin(); i != entries.end(); ++i) {
            KstBaseCurveList::Iterator it = curves.findTag(*i);
            if (it != curves.end()) {
              if (plot && plot->addCurve(*it)) {
                new KstViewObjectItem(this, *it, _vm);
                added = true;
              } else if (legend) {
                legend->addCurve(*it);
              }
            }
          }
          if (added) {
            win->view()->paint(KstPainter::P_PLOT);
          }
          accepted = true;
        }
      }
    }
  }

  e->accept(accepted);
}

KstDataObjectPtr KstViewObjectItem::dataObject(Kst2DPlotPtr &plot) const {
  KstDataObjectPtr dataObj;

  plot = 0L;

  if (rtti() == RTTI_OBJ_VIEW_OBJECT) {
    KstViewWindow *win;
    KstViewObjectPtr viewObj = viewObject(&win);

    if (viewObj) {
      plot = dynamic_cast<Kst2DPlot*>(viewObj.data());
    }
  } else if (rtti() == RTTI_OBJ_DATA_OBJECT) {
    KstViewObjectItem *koi;
    QListViewItem *item;

    dataObj = *KST::dataObjectList.findTag(tagName());
    item = parent();
    if (item) {
      koi = static_cast<KstViewObjectItem*>(item);
      koi->dataObject(plot);
    }
  }

  return dataObj;
}

KstViewObjectPtr KstViewObjectItem::viewObject(KstViewWindow **win) const {
  KstViewObjectPtr viewObj;
  KstViewObjectItem *koi;
  QListViewItem *item;

  *win = 0L;

  if (rtti() == RTTI_OBJ_WINDOW) {
    *win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(tagName()));
    viewObj = (*win)->view()->findChild(tagName(), true);
  } else {
    item = parent();
    if (item) {
      koi = static_cast<KstViewObjectItem*>(item);
      viewObj = koi->viewObject(win);
      if (win) {
        viewObj = (*win)->view()->findChild(tagName(), true);
      }
    }
  }

  return viewObj;
}

void KstViewObjectItem::update(KstViewObjectPtr x, bool recursive, int localUseCount) {
  Q_UNUSED(localUseCount)
  Q_UNUSED(recursive)

  Kst2DPlotPtr plot = dynamic_cast<Kst2DPlot*>(x.data());
  QPtrStack<QListViewItem> trash;

  // garbage collect first
  for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
    bool found = false;
    KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
    if (oi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
      for (KstViewObjectList::ConstIterator obj = x->children().begin(); obj != x->children().end(); ++obj) {
        if ((*obj)->tagName() == oi->tagName()) {
          found = true;
          break;
        }
      }
    } else if (plot && oi->rtti() == RTTI_OBJ_DATA_OBJECT) {
      for (KstBaseCurveList::ConstIterator obj = plot->Curves.begin(); obj != plot->Curves.end(); ++obj) {
        if ((*obj)->tagName() == oi->tagName()) {
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
      new KstViewObjectItem(this, *obj, _vm);
    }
  }

  if (plot) {
    for (KstBaseCurveList::ConstIterator obj = plot->Curves.begin(); obj != plot->Curves.end(); ++obj) {
      bool found = false;
      for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
        KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
        if (oi->tagName() == (*obj)->tagName()) {
          found = true;
          break;
        }
      }
      if (!found) {
        new KstViewObjectItem(this, *obj, _vm);
      }
    } 
  }
}

void KstViewObjectItem::updateButtons() {
  _vm->Edit->setEnabled(true);
  _vm->Delete->setEnabled(true);
}

void KstViewObjectItem::openChildren(bool open) {
  for (QListViewItem *qi = firstChild(); qi; qi = qi->nextSibling()) {
    KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(qi);

    oi->openChildren(open);
    qi->setOpen(open);
  }
}

// ==============================================

KstViewManager::KstViewManager(KstDoc *in_doc, QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstViewManager(parent, name, modal, fl) {
  setupUi(this);
  doc = in_doc;
  delete ViewView;

  setAcceptDrops(TRUE);
  ViewView = new KstViewListView(this, "ViewView");
  ViewView->addColumn(i18n("Name"));
  ViewView->addColumn(i18n("Type"));
  ViewView->setAllColumnsShowFocus(TRUE);
  ViewView->setShowSortIndicator(TRUE);
  ViewView->setRootIsDecorated(TRUE);
  ViewView->setAcceptDrops(FALSE);
  ViewView->viewport()->setAcceptDrops(TRUE);
  ViewView->setSelectionMode(QListView::Single);
  KstViewManagerLayout->addMultiCellWidget(ViewView, 0, 0, 0, 3);

  connect(Close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(ViewView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(edit_I()));
  connect(ViewView, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(currentChanged(QListViewItem *)));
  connect(ViewView, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(currentChanged(QListViewItem *)));
  connect(ViewView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(contextMenu(QListViewItem*, const QPoint&, int)));
}

KstViewManager::~KstViewManager() {
}

void KstViewManager::show_I() {
  show();
  raise();
  update();
}

void KstViewManager::updateContents() {
  update();
}

void KstViewManager::update() {
  KstApp *app = KstApp::inst();

  if (!isShown()) {
    return;
  }

  QListViewItem *currentItem = ViewView->selectedItem();
  QPtrStack<QListViewItem> trash;

  //
  // garbage collect first
  //

  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator it;

  windows = app->subWindowList( CreationOrder );

  for (QListViewItem *i = ViewView->firstChild(); i; i = i->nextSibling()) {
    bool found = false;
    KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);
    if (i->rtti() == RTTI_OBJ_WINDOW) {
      for (it = windows.constBegin(); it != windows.constEnd(); ++it)
        KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*it);
        if (viewWindow) {
          KstTopLevelViewPtr view = viewWindow->view();
          if (view) {
            if (view->tagName() == oi->tagName()) {
              found = true;
            }
          }
        }
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

  while (it->currentItem()) {
    for (it = windows.constBegin(); it != windows.constEnd(); ++it)
      KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*it);

      if (viewWindow) {
        bool found = false;

        for (QListViewItem *i = ViewView->firstChild(); i; i = i->nextSibling()) {
          KstViewObjectItem *oi = static_cast<KstViewObjectItem*>(i);

          if (oi->rtti() == RTTI_OBJ_WINDOW && oi->tagName() == viewWindow->tagName()) {
            oi->update(KstViewObjectPtr(viewWindow));

            found = true;

            break;
          }
        }

        if (!found) {
          new KstViewObjectItem(ViewView, viewWindow, this);
        }
      }
    }
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

void KstViewManager::edit_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);
  KstViewWindow *win;

  if (koi) {
    if (qi->rtti() == RTTI_OBJ_WINDOW) {
      win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(koi->tagName()));
      if (win) {
        win->view()->showDialog(win->view(), false);
      }
    } else if (qi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
      KstViewObjectPtr obj = koi->viewObject(&win);
      if (obj && win) {
        obj->showDialog(win->view(), false);
      }
    } else if (qi->rtti() == RTTI_OBJ_DATA_OBJECT) {
      Kst2DPlotPtr plot;
      KstDataObjectPtr obj = koi->dataObject(plot);
      if (obj) {
        obj->showDialog(false);
      }
    }
  } else {
    QMessageBox::warning(this, i18n("Kst"), i18n("An item must be selected to edit."));
  }
}

void KstViewManager::rename_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);
  KstViewWindow *win;

  if (koi) {
    if (qi->rtti() == RTTI_OBJ_WINDOW) {
      win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(koi->tagName()));
      if (win) {
        KstApp::inst()->renameWindow(win);
      }
    }
  } else {
    QMessageBox::warning(this, i18n("Kst"), i18n("An item must be selected to edit."));
  }
}

void KstViewManager::delete_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    if (koi->removable()) {
      if (koi->rtti() == RTTI_OBJ_WINDOW) {
        KstViewWindow *win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(koi->tagName()));
        if (win) {
          win->close(true);
          update();
        }
      } else if (koi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
        KstViewWindow *win;
        KstViewObjectPtr obj = koi->viewObject(&win);
        if (obj && win) {
          win->view()->removeChild(obj, true);
          win->view()->paint(KstPainter::P_PAINT);
          update();
        }
      } else if (koi->rtti() == RTTI_OBJ_DATA_OBJECT) {
        KstViewWindow *win;
        Kst2DPlotPtr plot;
        KstDataObjectPtr obj = koi->dataObject(plot);
        KstBaseCurvePtr curve = dynamic_cast<KstBaseCurve*>(obj.data());
        koi->viewObject(&win);
        if (curve && plot && win) {
          plot->removeCurve(curve);
          win->view()->paint(KstPainter::P_PAINT);
          update();
        }
      }
    }
  } else {
    QMessageBox::warning(this, i18n("Kst"), i18n("An item must be selected to delete."));
  }
}

void KstViewManager::activate_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    if (koi->rtti() == RTTI_OBJ_WINDOW) {
      KstViewWindow *win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(koi->tagName()));
      if (win) {
        win->activate();
      }
    }
  }
}

void KstViewManager::cleanupDefault_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    if (koi->rtti() == RTTI_OBJ_WINDOW) {
      KstViewWindow *win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(koi->tagName()));
      if (win) {
        win->view()->cleanupDefault();
      }
    }
  }
}

void KstViewManager::cleanupCustom_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    if (koi->rtti() == RTTI_OBJ_WINDOW) {
      KstViewWindow *win = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(koi->tagName()));
      if (win) {
        win->view()->cleanupCustom();
      }
    }
  }
}

void KstViewManager::select_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    if (koi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
      KstViewWindow *win;
      KstViewObjectPtr obj = koi->viewObject(&win);
      if (obj && win) {
        win->activate();
        obj->setSelected(true);
        win->view()->paint(KstPainter::P_PAINT);
        update();
      }
    }
  }
}

void KstViewManager::deselect_I() {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    if (koi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
      KstViewWindow *win;
      KstViewObjectPtr obj = koi->viewObject(&win);
      if (obj && win) {
        win->activate();
        obj->setSelected(false);
        win->view()->paint(KstPainter::P_PAINT);
        update();
      }
    }
  }
}

void KstViewManager::open() {
  QListViewItem *qi = ViewView->selectedItem();
  
  if (qi) {
    qi->setOpen(true);
  }
}

void KstViewManager::close() {
  QListViewItem *qi = ViewView->selectedItem();

  if (qi) {
    qi->setOpen(false);
  }
}

void KstViewManager::open(bool open) {
  QListViewItem *qi = ViewView->selectedItem();
  KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(qi);

  if (koi) {
    koi->openChildren(open);
    qi->setOpen(open);
  } else {
    for (qi = ViewView->firstChild(); qi; qi = qi->nextSibling()) {
      koi = static_cast<KstViewObjectItem*>(qi);

      koi->openChildren(open);
      qi->setOpen(open);
    }
  }
}

void KstViewManager::openAll() {
  open(true);
}

void KstViewManager::closeAll() {
  open(false);
}

void KstViewManager::contextMenu(QListViewItem *i, const QPoint& p, int col) {
  Q_UNUSED(col)

  if (i) {
    KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(i);
    KPopupMenu *menu = new KPopupMenu(this);
    int id;

    menu->insertTitle(koi->text(0));

    if (koi->rtti() == RTTI_OBJ_WINDOW) {
      id = menu->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
      id = menu->insertItem(i18n("&Rename..."), this, SLOT(rename_I()));
      id = menu->insertItem(i18n("&Close"), this, SLOT(delete_I()));
      id = menu->insertItem(i18n("&Activate"), this, SLOT(activate_I()));

      KPopupMenu *submenu = new KPopupMenu(menu);
      if (submenu) {
        menu->insertSeparator();
        submenu->insertItem("Default Tile", this, SLOT(cleanupDefault_I()));
        submenu->insertItem("Custom...", this, SLOT(cleanupCustom_I()));
        id = menu->insertItem(i18n("Cleanup Layout"), submenu);
      }
    } else if (koi->rtti() == RTTI_OBJ_VIEW_OBJECT) {
      id = menu->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
      id = menu->insertItem(i18n("&Delete"), this, SLOT(delete_I()));
      id = menu->insertItem(i18n("&Select"), this, SLOT(select_I()));
      menu->setItemEnabled(id, KstApp::inst()->currentViewMode() == KstTopLevelView::LayoutMode);
      id = menu->insertItem(i18n("Dese&lect"), this, SLOT(deselect_I()));
      menu->setItemEnabled(id, KstApp::inst()->currentViewMode() == KstTopLevelView::LayoutMode);
    } else if (koi->rtti() == RTTI_OBJ_DATA_OBJECT) {
      id = menu->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
      id = menu->insertItem(i18n("&Remove"), this, SLOT(delete_I()));
    }

    menu->insertSeparator();
    id = menu->insertItem(i18n("Expand"), this, SLOT(open()));
    menu->setItemEnabled(id, !koi->isOpen() && koi->firstChild());
    id = menu->insertItem(i18n("Collapse"), this, SLOT(close()));
    menu->setItemEnabled(id, koi->isOpen() && koi->firstChild());
    id = menu->insertItem(i18n("Expand All"), this, SLOT(openAll()));
    menu->setItemEnabled(id, koi->firstChild());
    id = menu->insertItem(i18n("Collapse All"), this, SLOT(closeAll()));
    menu->setItemEnabled(id, koi->firstChild());

    menu->popup(p);
  } else {
    KPopupMenu *menu = new KPopupMenu(this);
    int id;

    id = menu->insertItem(i18n("Expand All"), this, SLOT(openAll()));
    menu->setItemEnabled(id, ViewView->firstChild());
    id = menu->insertItem(i18n("Collapse All"), this, SLOT(closeAll()));
    menu->setItemEnabled(id, ViewView->firstChild());        

    menu->popup(p);
  }
}

void KstViewManager::doUpdates() {
  emit docChanged();
}

void KstViewManager::currentChanged(QListViewItem *i) {
  if (i) {
    KstViewObjectItem *koi = static_cast<KstViewObjectItem*>(i);
    koi->updateButtons();
  }
}

#include "kstviewmanager.moc"
