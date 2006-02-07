/***************************************************************************
                       kstdatamanger_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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
#include "kstdatamanager_i.h"

#include <assert.h>

#include <kdebug.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include <qmap.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qwidget.h>

#include "kstfilteredvector.h"
#include "kstdoc.h"
#include "kstplugin.h"
#include "kstrvector.h"
#include "kstdatacollection.h"
#include "kstpsddialog_i.h"
#include "ksteqdialog_i.h"
#include "ksthsdialog_i.h"
#include "kstcurvedialog_i.h"
#include "kstplugindialog_i.h"
#include "kstvectordialog_i.h"
#include "vectorselector.h"


static QMap<int,QString> PlotMap;
static QMap<int,QString> FilterMap;

#define RTTI_OBJ_VECTOR          4201
#define RTTI_OBJ_OBJECT          4202
#define RTTI_OBJ_DATA_VECTOR     4203
#define RTTI_OBJ_FILTERED_VECTOR 4204


KstObjectItem::KstObjectItem(QListView *parent, KstFilteredVectorPtr x, KstDataManagerI *dm)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_FILTERED_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  setText(1, i18n("Filtered Vector"));
  x = 0L; // keep the counts in sync
  update();
}

KstObjectItem::KstObjectItem(QListView *parent, KstRVectorPtr x, KstDataManagerI *dm)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_DATA_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  setText(1, i18n("Data Vector"));
  x = 0L; // keep the counts in sync
  update();
}

KstObjectItem::KstObjectItem(QListViewItem *parent, KstVectorPtr x, KstDataManagerI *dm)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  setText(1, i18n("Slave Vector"));
  x = 0L; // keep the counts in sync
  update();
}

KstObjectItem::KstObjectItem(QListView *parent, KstDataObjectPtr x, KstDataManagerI *dm)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_OBJECT), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  for (KstVectorMap::Iterator i = x->outputVectors().begin();
      i != x->outputVectors().end();
      ++i) {
    KstObjectItem *item = new KstObjectItem(this, i.data(), _dm);
    connect(item, SIGNAL(updated()), this, SIGNAL(updated()));
  }
  x = 0L; // keep the counts in sync
  update(false);
}

KstObjectItem::~KstObjectItem() {
}

void KstObjectItem::update(bool recursive) {
  switch (_rtti) {
    case RTTI_OBJ_DATA_VECTOR:
      {
        KstVectorPtr px = *KST::vectorList.findTag(_name);
        assert(px.data());
        assert(dynamic_cast<KstRVector*>(px.data()));
        KstRVectorPtr x = static_cast<KstRVector*>(px.data());
        x->readLock();
        setText(2, QString::number(x->getUsage() - 3)); // caller has a ptr
        setText(3, QString::number(x->sampleCount()));
        setText(4, i18n("%3: %4 [%1..%2]").arg(x->reqStartFrame())
            .arg(x->reqStartFrame() + x->reqNumFrames())
            .arg(x->filename())
            .arg(x->getField()));
        _removable = x->getUsage() == 3;
        x->readUnlock();
        break;
      }
    case RTTI_OBJ_FILTERED_VECTOR:
      {
        KstVectorPtr px = *KST::vectorList.findTag(_name);
        assert(px.data());
        assert(dynamic_cast<KstFilteredVector*>(px.data()));
        KstFilteredVectorPtr x = static_cast<KstFilteredVector*>(px.data());
        x->readLock();
        setText(2, QString::number(x->getUsage() - 3)); // caller has a ptr
        setText(3, QString::number(x->sampleCount()));
        setText(4, i18n("[%1..%2]").arg(x->min()).arg(x->max()));
        _removable = x->getUsage() == 3;
        x->readUnlock();
        break;
      }
    case RTTI_OBJ_VECTOR:
      {
        KstVectorPtr x = *KST::vectorList.findTag(_name);
        assert(x.data());
        x->readLock();
        setText(2, QString::number(x->getUsage() - 2)); //caller also points
        setText(3, QString::number(x->sampleCount()));
        setText(4, i18n("[%1..%2]").arg(x->min()).arg(x->max()));
        x->readUnlock();
        _removable = false;
        break;
      }
    case RTTI_OBJ_OBJECT:
      {
        KstDataObjectPtr x = *KST::dataObjectList.findTag(_name);
        assert(x.data());
        x->readLock();
        setText(1, x->typeString());
        setText(2, QString::number(x->getUsage() - 1)); // our pointer
        setText(3, QString::number(x->sampleCount()));
        setText(4, x->propertyString());
        if (recursive) {
          QPtrStack<QListViewItem> trash;

          for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
            KstObjectItem *oi = static_cast<KstObjectItem*>(i);
            if (x->outputVectors().findTag(oi->tagName()) == x->outputVectors().end()) {
              trash.push(i);
            }
          }
          trash.setAutoDelete(true);
          trash.clear();

          for (KstVectorMap::Iterator p = x->outputVectors().begin();
              p != x->outputVectors().end();
              ++p) {
            bool found = false;
            for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
              KstObjectItem *oi = static_cast<KstObjectItem*>(i);
              if (oi->tagName() == p.data()->tagName()) {
                oi->update();
                found = true;
                break;
              }
            }
            if (!found) {
              KstObjectItem *item = new KstObjectItem(this, p.data(), _dm);
              connect(item, SIGNAL(updated()), this, SIGNAL(updated()));
            }
          }
        }
        _removable = x->getUsage() == 1;
        x->readUnlock();
        break;
      }
    default:
      assert(0);
  }
}

void KstObjectItem::updateButtons() {
  _dm->Edit->setEnabled(RTTI_OBJ_VECTOR != _rtti && RTTI_OBJ_FILTERED_VECTOR != _rtti);
  _dm->Delete->setEnabled(_removable);
}

void KstObjectItem::reload() {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator v = KST::vectorList.findTag(_name);
  if (v != KST::vectorList.end()) {
    KstRVector *r = dynamic_cast<KstRVector*>((*v).data());
    if (r) {
      r->writeLock();
      r->reload();
      r->writeUnlock();
    }
  }
}

void KstObjectItem::makeCurve() {
  KstCurveDialogI::globalInstance()->show_New();
  KstCurveDialogI::globalInstance()->_yVector->setSelection(_name);
}

void KstObjectItem::applyFilter(int id) {
  KST::filterSetList.lock().readLock();
  KstFilterSetPtr p = KST::filterSetList.find(FilterMap[id]);
  KST::filterSetList.lock().readUnlock();
  KST::vectorList.lock().readLock();
  KstVectorPtr v = *KST::vectorList.findTag(_name);
  KST::vectorList.lock().readUnlock();
  if (p && v) {
    KstFilteredVectorPtr fv = new KstFilteredVector(v, p);
    emit updated();
  }
}

void KstObjectItem::addToPlot(int id) {
  KstPlot *p = KST::plotList.FindKstPlot(PlotMap[id]);
  KstBaseCurve *c = dynamic_cast<KstBaseCurve*>(dataObject().data());
  if (p && c) {
    p->addCurve(c);
    emit updated();
  }
}

void KstObjectItem::removeFromPlot(int id) {
  KstPlot *p = KST::plotList.FindKstPlot(PlotMap[id]);
  KstBaseCurve *c = dynamic_cast<KstBaseCurve*>(dataObject().data());
  if (p && c) {
    p->removeCurve(c);
    emit updated();
  }
}


KstDataManagerI::KstDataManagerI(KstDoc *in_doc, QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataManager(parent, name, modal, fl) {
    doc = in_doc;

    connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
    connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
    connect(Purge, SIGNAL(clicked()), doc, SLOT(purge()));
    connect(DataView, SIGNAL(doubleClicked(QListViewItem *)),
            this, SLOT(edit_I()));
    connect(DataView, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(currentChanged(QListViewItem *)));

    connect(NewCurve, SIGNAL(clicked()),
            KstCurveDialogI::globalInstance(), SLOT(show_New()));
    connect(NewHs, SIGNAL(clicked()),
            KstHsDialogI::globalInstance(), SLOT(show_New()));
    connect(NewEq, SIGNAL(clicked()),
            KstEqDialogI::globalInstance(), SLOT(show_New()));
    connect(NewPlugin, SIGNAL(clicked()),
            KstPluginDialogI::globalInstance(), SLOT(show_New()));
    connect(NewPSD, SIGNAL(clicked()),
            KstPsdDialogI::globalInstance(), SLOT(show_New()));
    connect(NewVector, SIGNAL(clicked()),
            KstVectorDialogI::globalInstance(), SLOT(show_New()));

    connect(Filter, SIGNAL(clicked()), this, SLOT(filter_I()));

    connect(DataView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(contextMenu(QListViewItem*, const QPoint&, int)));
}


KstDataManagerI::~KstDataManagerI() {
}


void KstDataManagerI::show_I() {
  show();
  raise();
  update();
}


void KstDataManagerI::update() {
if (!isShown()) {
  return;
}

QPtrStack<QListViewItem> trash;

  KST::dataObjectList.lock().writeLock();
  KST::vectorList.lock().writeLock();
  // Garbage collect first
  for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
    KstObjectItem *oi = static_cast<KstObjectItem*>(i);
    if (i->rtti() == RTTI_OBJ_OBJECT) {
      if (KST::dataObjectList.findTag(oi->tagName()) == KST::dataObjectList.end()) {
        trash.push(i);
      }
    } else {
      if (KST::vectorList.findTag(oi->tagName()) == KST::vectorList.end()) {
        trash.push(i);
      }
    }
  }

  trash.setAutoDelete(true);
  DataView->blockSignals(true);
  trash.clear();
  DataView->blockSignals(false);

  for (KstDataObjectList::iterator it = KST::dataObjectList.begin();
                                    it != KST::dataObjectList.end();
                                                               ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_OBJECT && oi->tagName() == (*it)->tagName()) {
        oi->update();
        found = true;
        break;
      }
    }
    if (!found) {
        KstObjectItem *i = new KstObjectItem(DataView, *it, this);
        connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }
  KST::vectorList.lock().writeUnlock();
  KST::dataObjectList.lock().writeUnlock();

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (KstRVectorList::iterator it = rvl.begin(); it != rvl.end(); ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_DATA_VECTOR && oi->tagName() == (*it)->tagName()) {
        oi->update();
        found = true;
        break;
      }
    }
    if (!found) {
        KstObjectItem *i = new KstObjectItem(DataView, *it, this);
        connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  KstFilteredVectorList fvl = kstObjectSubList<KstVector,KstFilteredVector>(KST::vectorList);
  for (KstFilteredVectorList::iterator it = fvl.begin(); it != fvl.end(); ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_FILTERED_VECTOR && oi->tagName() == (*it)->tagName()) {
        oi->update();
        found = true;
        break;
      }
    }
    if (!found) {
        KstObjectItem *i = new KstObjectItem(DataView, *it, this);
        connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  if (DataView->selectedItem()) {
    static_cast<KstObjectItem*>(DataView->currentItem())->updateButtons();
  } else {
    Edit->setEnabled(false);
    Delete->setEnabled(false);
  }
}


void KstDataManagerI::filter_I() {
  static_cast<KstApp*>(doc->parent())->showFilterListEditor();
}


void KstDataManagerI::edit_I() {
  QListViewItem *qi;

  if (DataView->selectedItems().count() > 0) {
    qi = DataView->selectedItems().at(0);
  } else {
    KMessageBox::sorry(this, i18n("A data item must be selected to edit."));
    return;
  }

  if (qi->rtti() == RTTI_OBJ_DATA_VECTOR) {
    emit editDataVector(qi->text(0));
  }

  if (qi->rtti() == RTTI_OBJ_OBJECT) {
    static_cast<KstObjectItem*>(qi)->dataObject()->showDialog();
  }
}


void KstDataManagerI::delete_I() {
  QListViewItem *qi = DataView->selectedItems().at(0);
  KstObjectItem *koi = static_cast<KstObjectItem*>(qi);

  KstWriteLocker ml(&KST::vectorList.lock());

  if (qi->rtti() == RTTI_OBJ_OBJECT) {
    doc->removeDataObject(koi->tagName());
  } else if (qi->rtti() == RTTI_OBJ_DATA_VECTOR) {
    KST::vectorList.removeTag(koi->tagName());
    doUpdates();
  } else if (qi->rtti() == RTTI_OBJ_FILTERED_VECTOR) {
    KST::vectorList.removeTag(koi->tagName());
    doUpdates();
  }
}


// Menu IDs:
// 100->499 reserved for plots
// 500->999 reserver for filters

void KstDataManagerI::contextMenu(QListViewItem *i, const QPoint& p, int col) {
  Q_UNUSED(col)
  KstObjectItem *koi = static_cast<KstObjectItem*>(i);
  KstBaseCurve *c;

  KPopupMenu *m = new KPopupMenu(this);

  m->setTitle(koi->text(0));

  int id;
  if (koi->rtti() != RTTI_OBJ_VECTOR && koi->rtti() != RTTI_OBJ_FILTERED_VECTOR) {
    id = m->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
  }

  if (koi->rtti() == RTTI_OBJ_DATA_VECTOR ||
      koi->rtti() == RTTI_OBJ_FILTERED_VECTOR ||
      koi->rtti() == RTTI_OBJ_VECTOR) {
    KPopupMenu *fMenu = new KPopupMenu(this);
    id = 500;
    FilterMap.clear();
    KST::filterSetList.lock().readLock();
    for (KstFilterSetList::Iterator f = KST::filterSetList.begin(); f != KST::filterSetList.end(); ++f) {
      fMenu->insertItem((*f)->name(), koi, SLOT(applyFilter(int)), 0, id);
      FilterMap[id] = (*f)->name();
      ++id;
    }
    KST::filterSetList.lock().readUnlock();
    id = m->insertItem(i18n("Apply &Filter"), fMenu);
    m->setItemEnabled(id, !FilterMap.isEmpty());
  }

  if (koi->rtti() == RTTI_OBJ_DATA_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
    id = m->insertItem(i18n("&Reload"), koi, SLOT(reload()));
  } else if (koi->rtti() == RTTI_OBJ_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
  } else if (koi->rtti() == RTTI_OBJ_FILTERED_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
  } else if ((c = dynamic_cast<KstBaseCurve*>(koi->dataObject().data()))) {
    KPopupMenu *addMenu = new KPopupMenu(this);
    KPopupMenu *removeMenu = new KPopupMenu(this);
    PlotMap.clear();
    id = 100;
    bool haveAdd = false, haveRemove = false;
    for (KstPlot *p = KST::plotList.first(); p; p = KST::plotList.next()) {
      if (!p->Curves.contains(c)) {
        addMenu->insertItem(p->tagName(), koi, SLOT(addToPlot(int)), 0, id);
        haveAdd = true;
      } else {
        removeMenu->insertItem(p->tagName(), koi, SLOT(removeFromPlot(int)), 0, id);
        haveRemove = true;
      }
      PlotMap[id++] = p->tagName();
    }
    id = m->insertItem(i18n("&Add To Plot"), addMenu);
    m->setItemEnabled(id, haveAdd);
    id = m->insertItem(i18n("&Remove From Plot"), removeMenu);
    m->setItemEnabled(id, haveRemove);
  }

  if (koi->rtti() != RTTI_OBJ_VECTOR) {
    // no slave vectors get this
    id = m->insertItem(i18n("&Delete"), this, SLOT(delete_I()));
    m->setItemEnabled(id, koi->removable());
  }

  m->popup(p);
}


void KstDataManagerI::doUpdates() {
  doc->forceUpdate();
  doc->setModified();
}


void KstDataManagerI::currentChanged(QListViewItem *i) {
  if (i) {
    KstObjectItem *koi = static_cast<KstObjectItem*>(i);
    koi->updateButtons();
  }
}

#include "kstdatamanager_i.moc"
// vim: ts=2 sw=2 et
