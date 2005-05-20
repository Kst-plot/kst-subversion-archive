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
#include <assert.h>

// include files for Qt
#include <qptrstack.h>

// include files for KDE
#include <kdebug.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// application specific includes
#include "kst2dplot.h"
#include "kstcurvedialog_i.h"
#include "kstdatacollection.h"
#include "kstdatamanager_i.h"
#include "kstdoc.h"
#include "ksteqdialog_i.h"
#include "ksteventmonitor_i.h"
#include "ksthsdialog_i.h"
#include "kstimagedialog_i.h"
#include "kstmatrixdialog_i.h"
#include "kstplugindialog_i.h"
#include "kstpsddialog_i.h"
#include "kstvectordialog_i.h"
#include "kstviewwindow.h"
#include "matrixselector.h"
#include "vectorselector.h"


static QMap<int,Kst2DPlotPtr> PlotMap;
static QMap<int,QString> FilterMap;

#define RTTI_OBJ_VECTOR          4201
#define RTTI_OBJ_OBJECT          4202
#define RTTI_OBJ_DATA_VECTOR     4203
#define RTTI_OBJ_STATIC_VECTOR   4204


KstObjectItem::KstObjectItem(QListView *parent, KstRVectorPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_DATA_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  setText(1, i18n("Data Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}

KstObjectItem::KstObjectItem(QListView *parent, KstSVectorPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_STATIC_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  setText(1, i18n("Static Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListViewItem *parent, KstVectorPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  setText(0, x->tagName());
  setText(1, i18n("Slave Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListView *parent, KstDataObjectPtr x, KstDataManagerI *dm, int localUseCount)
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
  update(false, localUseCount);
}


KstObjectItem::~KstObjectItem() {
}


void KstObjectItem::update(bool recursive, int localUseCount) {
  switch (_rtti) {
    case RTTI_OBJ_DATA_VECTOR:
      {
        KstRVectorPtr x = kst_cast<KstRVector>(*KST::vectorList.findTag(_name));
        if (x) {
          x->readLock();
          // getUsage: subtract 1 for KstRVectorPtr x
          setPixmap(2, (x->getUsage() - 1 - localUseCount) > 0 ? QPixmap(locate("data", "kst/pics/yes.png")) : QPixmap());
          //setText(2, QString::number(x->getUsage() - 1 - localUseCount));
          setText(3, QString::number(x->length()));
          setText(4, i18n("%3: %4 [%1..%2]").arg(x->reqStartFrame())
              .arg(x->reqStartFrame() + x->reqNumFrames())
              .arg(x->filename())
              .arg(x->field()));
          _removable = x->getUsage() == 2;
          x->readUnlock();
        }
        // Hmmm what happens if this if() fails??  We become inconsistent?
        break;
      }
    case RTTI_OBJ_STATIC_VECTOR:
      {
        KstSVectorPtr x = kst_cast<KstSVector>(*KST::vectorList.findTag(_name));
        if (x) {
          x->readLock();
          // getUsage: subtract 1 for KstRVectorPtr x
          setPixmap(2, (x->getUsage() - 1 - localUseCount) > 0 ? QPixmap(locate("data", "kst/pics/yes.png")) : QPixmap());
          setText(3, QString::number(x->length()));
          setText(4, i18n("%1 to %2").arg(x->min()).arg(x->max()));
          _removable = x->getUsage() == 2;
          x->readUnlock();
        }
        // Hmmm what happens if this if() fails??  We become inconsistent?
        break;
      }
    case RTTI_OBJ_VECTOR:
      {
        KstVectorPtr x = *KST::vectorList.findTag(_name);
        if (x) {
          x->readLock();
          // getUsage:
          //  subtract 1 for KstVectorPtr x
          setPixmap(2, (x->getUsage() - 1 - localUseCount) > 0 ? QPixmap(locate("data", "kst/pics/yes.png")) : QPixmap());
          //setText(2, QString::number(x->getUsage() - 1 - localUseCount));
          setText(3, QString::number(x->length()));
          setText(4, i18n("[%1..%2]").arg(x->min()).arg(x->max()));
          x->readUnlock();
          _removable = false;
        }
        break;
      }
    case RTTI_OBJ_OBJECT:
      {
        KstDataObjectPtr x = *KST::dataObjectList.findTag(_name);
        if (x) {
          x->readLock();
          setText(1, x->typeString());
          // getUsage:
          //  subtract 1 for KstDataObjectPtr x
          setPixmap(2, (x->getUsage() - 1 - localUseCount) > 0 ? QPixmap(locate("data", "kst/pics/yes.png")) : QPixmap());
          //setText(2, QString::number(x->getUsage() - 1 - localUseCount));
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
                p != x->outputVectors().end(); ++p) {
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
        }
        break;
      }
    default:
      assert(0);
  }
}


void KstObjectItem::updateButtons() {
  _dm->Edit->setEnabled(RTTI_OBJ_VECTOR != _rtti);
  _dm->Delete->setEnabled(RTTI_OBJ_VECTOR != _rtti);
}


void KstObjectItem::reload() {
  KstReadLocker ml(&KST::vectorList.lock());
  KstVectorList::Iterator v = KST::vectorList.findTag(_name);
  if (v != KST::vectorList.end()) {
    KstRVectorPtr r = kst_cast<KstRVector>(*v);
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


void KstObjectItem::makePSD() {
  KstPsdDialogI::globalInstance()->show_New();
  KstPsdDialogI::globalInstance()->_vector->setSelection(_name);
}


void KstObjectItem::makeHistogram() {
  KstHsDialogI::globalInstance()->show_New();
  KstHsDialogI::globalInstance()->_vector->setSelection(_name);
}


void KstObjectItem::makeMatrix() {
  KstMatrixDialogI::globalInstance()->show_New();
  KstMatrixDialogI::globalInstance()->_zVector->setSelection(_name);
}


void KstObjectItem::makeImage() {
  KstImageDialogI::globalInstance()->show_New();
  KstImageDialogI::globalInstance()->_matrix->setSelection(_name);
}


void KstObjectItem::applyFilter(int id) {
  /* Replace with new filtering logic
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
   */
}


void KstObjectItem::activateHint(int id) {
  KstDataObjectPtr d = dataObject();
  const KstCurveHintList* hints = d->curveHints();
  int cnt = 0;
  for (KstCurveHintList::ConstIterator i = hints->begin(); i != hints->end(); ++i) {
    if (cnt == id) {
      KstBaseCurvePtr c = (*i)->makeCurve(KST::suggestCurveName(d->tagName(), false),
          KstColorSequence::next());
      if (c) {
        KST::dataObjectList.lock().writeLock();
        KST::dataObjectList.append(c.data());
        KST::dataObjectList.lock().writeUnlock();
        emit updated();
      } else {
        KMessageBox::sorry(0L, i18n("Unable to create quick curve."));
      }
      break;
    }
    cnt++;
  }
}


void KstObjectItem::addToPlot(int id) {
  Kst2DPlotPtr p = PlotMap[id];
  KstBaseCurvePtr c = kst_cast<KstBaseCurve>(dataObject());
  if (p && c) {
    p->addCurve(c);
    p->setDirty();
    paintPlot(p);
    emit updated();
  }
}


void KstObjectItem::removeFromPlot(int id) {
  Kst2DPlotPtr p = PlotMap[id];
  KstBaseCurvePtr c = kst_cast<KstBaseCurve>(dataObject());
  if (p && c) {
    p->removeCurve(c);
    p->setDirty();
    paintPlot(p);
    emit updated();
  }
}


void KstObjectItem::addImageToPlot(int id) {
  Kst2DPlotPtr p = PlotMap[id];
  KstImagePtr img = kst_cast<KstImage>(dataObject());
  if (p && img) {
    p->addImage(img);
    p->setDirty();
    paintPlot(p);
    emit updated();
  }
}


void KstObjectItem::removeImageFromPlot(int id) {
  Kst2DPlotPtr p = PlotMap[id];
  KstImagePtr img = kst_cast<KstImage>(dataObject());
  if (p && img) {
    p->removeImage(img);
    p->setDirty();
    paintPlot(p);
    emit updated();
  }
}


void KstObjectItem::paintPlot(Kst2DPlotPtr p) {
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  while (it->currentItem()) {
    KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (v && v->view()->contains(kst_cast<KstViewObject>(p))) {
      v->view()->paint(P_PLOT);
      break;
    }
    it->next();
  }
  app->deleteIterator(it);
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
  connect(DataView, SIGNAL(selectionChanged(QListViewItem *)),
      this, SLOT(currentChanged(QListViewItem *)));

  connect(NewCurve, SIGNAL(clicked()),
      KstCurveDialogI::globalInstance(), SLOT(show_New()));
  connect(NewHs, SIGNAL(clicked()),
      KstHsDialogI::globalInstance(), SLOT(show_New()));
  connect(NewEq, SIGNAL(clicked()),
      KstEqDialogI::globalInstance(), SLOT(show_New()));
  connect(NewPlugin, SIGNAL(clicked()),
      KstPluginDialogI::globalInstance(), SLOT(show_New()));
  connect(NewEvent, SIGNAL(clicked()),
      KstEventMonitorI::globalInstance(), SLOT(show_New()));
  connect(NewPSD, SIGNAL(clicked()),
      KstPsdDialogI::globalInstance(), SLOT(show_New()));
  connect(NewVector, SIGNAL(clicked()),
      KstVectorDialogI::globalInstance(), SLOT(show_New()));
  connect(OpenPlotDialog, SIGNAL(clicked()),
      KstApp::inst(), SLOT(showPlotDialog()));

  connect(NewImage, SIGNAL(clicked()),
      KstImageDialogI::globalInstance(), SLOT(show_New()));

  connect(NewMatrix, SIGNAL(clicked()),
      KstMatrixDialogI::globalInstance(), SLOT(show_New()));

  // FIXME: Filters disabled for now...
  //  connect(Filter, SIGNAL(clicked()), this, SLOT(filter_I()));

  connect(DataView, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(contextMenu(QListViewItem*, const QPoint&, int)));
}


KstDataManagerI::~KstDataManagerI() {
}


void KstDataManagerI::show_I() {
  show();
  raise();
  update();
}


void KstDataManagerI::updateContents() {
  if (!isShown()) {
    return;
  }

  for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
    KstObjectItem *oi = static_cast<KstObjectItem*>(i);
    oi->update();
  }
}


void KstDataManagerI::update() {
  if (!isShown()) {
    return;
  }

  QListViewItem *currentItem = DataView->selectedItem();
  QPtrStack<QListViewItem> trash;

  KST::dataObjectList.lock().writeLock();
  KST::vectorList.lock().writeLock();

  // garbage collect first
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

  // update the data objects
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

  // update the data vectors
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (KstRVectorList::iterator it = rvl.begin(); it != rvl.end(); ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_DATA_VECTOR && oi->tagName() == (*it)->tagName()) {
        oi->update(true, 1);
        found = true;
        break;
      }
    }
    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *it, this, 1);
      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  // update the static vectors
  KstSVectorList svl = kstObjectSubList<KstVector,KstSVector>(KST::vectorList);
  for (KstSVectorList::iterator it = svl.begin(); it != svl.end(); ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_STATIC_VECTOR && oi->tagName() == (*it)->tagName()) {
        oi->update(true, 1);
        found = true;
        break;
      }
    }
    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *it, this, 1);
      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  // is this really necessary?  I would think not...
  for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
    if (i == currentItem) {
      DataView->setCurrentItem(i);
      DataView->setSelected(i, true);
      break;
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
//  static_cast<KstApp*>(doc->parent())->showFilterListEditor();
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

  if (qi->rtti() == RTTI_OBJ_STATIC_VECTOR) {
    emit editStaticVector(qi->text(0));
  }

  if (qi->rtti() == RTTI_OBJ_OBJECT) {
    static_cast<KstObjectItem*>(qi)->dataObject()->showDialog();
  }
}


void KstDataManagerI::delete_I() {
  QListViewItem *qi = DataView->selectedItems().at(0);
  KstObjectItem *koi = static_cast<KstObjectItem*>(qi);

  if (koi->removable()) {
    if (qi->rtti() == RTTI_OBJ_OBJECT) {
      doc->removeDataObject(koi->tagName());
    } else if (qi->rtti() == RTTI_OBJ_DATA_VECTOR) {
      KST::vectorList.lock().writeLock();
      KST::vectorList.removeTag(koi->tagName());
      KST::vectorList.lock().writeUnlock();
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_STATIC_VECTOR) {
      KST::vectorList.lock().writeLock();
      KST::vectorList.removeTag(koi->tagName());
      KST::vectorList.lock().writeUnlock();
      doUpdates();
    }
    update();
  } else {
    // Don't prompt for base curves
    KstBaseCurvePtr bc = kst_cast<KstBaseCurve>(koi->dataObject());
    if (bc || KMessageBox::warningYesNo(this, i18n("There are other objects in memory that depend on %1.  Do you wish to delete them too?").arg(koi->tagName())) == KMessageBox::Yes) {

      if (qi->rtti() == RTTI_OBJ_OBJECT) {
        koi->dataObject()->deleteDependents();
        doc->removeDataObject(koi->tagName());
      } else if (qi->rtti() == RTTI_OBJ_DATA_VECTOR) {
        KstRVectorPtr x = kst_cast<KstRVector>(*KST::vectorList.findTag(koi->tagName()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::vectorList.lock().writeLock();
          KST::vectorList.removeTag(koi->tagName());
          KST::vectorList.lock().writeUnlock();
          doUpdates();
        } else {
          KMessageBox::sorry(this, i18n("Unknown error deleting data vector."));
        }
      } else if (qi->rtti() == RTTI_OBJ_STATIC_VECTOR) {
        KstSVectorPtr x = kst_cast<KstSVector>(*KST::vectorList.findTag(koi->tagName()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::vectorList.lock().writeLock();
          KST::vectorList.removeTag(koi->tagName());
          KST::vectorList.lock().writeUnlock();
          doUpdates();
        } else {
          KMessageBox::sorry(this, i18n("Unknown error deleting data vector."));
        }
      }
      KstApp::inst()->paintAll(P_PLOT);
      update();
    } else {
      KMessageBox::sorry(this, i18n("Cannot delete objects with dependencies."));
    }
  }
}


// Menu IDs:
// 100->499 reserved for plots
// 500->999 reserver for filters

void KstDataManagerI::contextMenu(QListViewItem *i, const QPoint& p, int col) {
  Q_UNUSED(col)

  if (!i) {
    return;
  }

  KstObjectItem *koi = static_cast<KstObjectItem*>(i);
  KstBaseCurvePtr c;
  KstImagePtr img;

  KPopupMenu *m = new KPopupMenu(this);

  m->insertTitle(koi->text(0));

  int id;
  if (koi->rtti() != RTTI_OBJ_VECTOR) {
    id = m->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
  }

  if (koi->rtti() == RTTI_OBJ_DATA_VECTOR ||
      koi->rtti() == RTTI_OBJ_VECTOR) {
    KPopupMenu *fMenu = new KPopupMenu(this);
    id = 500;
    FilterMap.clear();
#if 0
    KST::filterSetList.lock().readLock();
    for (KstFilterSetList::Iterator f = KST::filterSetList.begin(); f != KST::filterSetList.end(); ++f) {
      fMenu->insertItem((*f)->name(), koi, SLOT(applyFilter(int)), 0, id);
      FilterMap[id] = (*f)->name();
      ++id;
    }
    KST::filterSetList.lock().readUnlock();
#endif
    id = m->insertItem(i18n("Apply &Filter"), fMenu);
    m->setItemEnabled(id, !FilterMap.isEmpty());
  }

  if (koi->dataObject()) {
    const KstCurveHintList* hints = koi->dataObject()->curveHints();
    if (!hints->isEmpty()) {
      KPopupMenu *hintMenu = new KPopupMenu(this);
      int cnt = 0;
      for (KstCurveHintList::ConstIterator i = hints->begin(); i != hints->end(); ++i) {
        hintMenu->insertItem((*i)->curveName(), koi, SLOT(activateHint(int)), 0, cnt);
        cnt++;
      }
      id = m->insertItem(i18n("&Quick Curve"), hintMenu);
    }
  }

  if (koi->rtti() == RTTI_OBJ_DATA_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
    id = m->insertItem(i18n("Make &Power Spectrum..."), koi, SLOT(makePSD()));
    id = m->insertItem(i18n("Make &Histogram..."), koi, SLOT(makeHistogram()));
    id = m->insertItem(i18n("Make M&atrix..."), koi, SLOT(makeMatrix()));
    id = m->insertItem(i18n("&Reload"), koi, SLOT(reload()));
  } else if (koi->rtti() == RTTI_OBJ_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
    id = m->insertItem(i18n("Make &Power Spectrum..."), koi, SLOT(makePSD()));
    id = m->insertItem(i18n("Make &Histogram..."), koi, SLOT(makeHistogram()));
    id = m->insertItem(i18n("Make M&atrix..."), koi, SLOT(makeMatrix()));
  } else if (kst_cast<KstMatrix>(koi->dataObject())) {
    id = m->insertItem(i18n("Make &Image..."), koi, SLOT(makeImage()));
  } else if ((c = kst_cast<KstBaseCurve>(koi->dataObject()))) {
    KPopupMenu *addMenu = new KPopupMenu(this);
    KPopupMenu *removeMenu = new KPopupMenu(this);
    PlotMap.clear();
    id = 300;
    bool haveAdd = false, haveRemove = false;

    KstApp *app = KstApp::inst();
    KMdiIterator<KMdiChildView*> *it = app->createIterator();
    while (it->currentItem()) {
      KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (v) {
        Kst2DPlotList plots = v->view()->findChildrenType<Kst2DPlot>();
        for (Kst2DPlotList::Iterator i = plots.begin(); i != plots.end(); ++i) {
          Kst2DPlotPtr plot = *i;
          if (!plot->Curves.contains(c)) {
            addMenu->insertItem(i18n("%1 - %2").arg(v->caption()).arg(plot->tagName()), koi, SLOT(addToPlot(int)), 0, id);
            haveAdd = true;
          } else {
            removeMenu->insertItem(i18n("%1 - %2").arg(v->caption()).arg(plot->tagName()), koi, SLOT(removeFromPlot(int)), 0, id);
            haveRemove = true;
          }
          PlotMap[id++] = plot;
        }
      }
      it->next();
    }

    app->deleteIterator(it);

    id = m->insertItem(i18n("&Add to Plot"), addMenu);
    m->setItemEnabled(id, haveAdd);
    id = m->insertItem(i18n("&Remove From Plot"), removeMenu);
    m->setItemEnabled(id, haveRemove);
  } else if ((img = kst_cast<KstImage>(koi->dataObject()))) {
    KPopupMenu *addMenu = new KPopupMenu(this);
    KPopupMenu *removeMenu = new KPopupMenu(this);
    PlotMap.clear();
    id = 100;
    bool haveAdd = false, haveRemove = false;

    KstApp *app = KstApp::inst();
    KMdiIterator<KMdiChildView*> *it = app->createIterator();
    while (it->currentItem()) {
      KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (v) {
        Kst2DPlotList plots = v->view()->findChildrenType<Kst2DPlot>();
        for (Kst2DPlotList::Iterator i = plots.begin(); i != plots.end(); ++i) {
          Kst2DPlotPtr plot = *i;
          if (!plot->hasImage(img)) {
            addMenu->insertItem(i18n("%1 - %2").arg(v->caption()).arg(plot->tagName()), koi, SLOT(addImageToPlot(int)), 0, id);
            haveAdd = true;
          } else {
            removeMenu->insertItem(i18n("%1 - %2").arg(v->caption()).arg(plot->tagName()), koi, SLOT(removeImageFromPlot(int)), 0, id);
            haveRemove = true;
          }
          PlotMap[id++] = plot;
        }
      }
      it->next();
    }

    app->deleteIterator(it);

    id = m->insertItem(i18n("&Add to Plot"), addMenu);
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
//  doc->forceUpdate();
//  doc->setModified();
  emit docChanged();
}


void KstDataManagerI::currentChanged(QListViewItem *i) {
  if (i) {
    KstObjectItem *koi = static_cast<KstObjectItem*>(i);
    koi->updateButtons();
  }
}

#include "kstdatamanager_i.moc"
// vim: ts=2 sw=2 et
