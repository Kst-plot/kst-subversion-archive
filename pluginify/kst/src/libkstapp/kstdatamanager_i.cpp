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
#include "ksdebug.h"
#include <klistview.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// application specific includes
#include "datasourcemetadatadialog.h"
#include "kst2dplot.h"
#include "kstcurvedialog_i.h"
#include "kstcsddialog_i.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
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

#define RTTI_OBJ_VECTOR          4201
#define RTTI_OBJ_OBJECT          4202
#define RTTI_OBJ_DATA_VECTOR     4203
#define RTTI_OBJ_STATIC_VECTOR   4204
#define RTTI_OBJ_MATRIX          4205
#define RTTI_OBJ_DATA_MATRIX     4206
#define RTTI_OBJ_STATIC_MATRIX   4207

KstObjectItem::KstObjectItem(QListView *parent, KstRVectorPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_DATA_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, i18n("Data Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListView *parent, KstSVectorPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_STATIC_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, i18n("Static Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListViewItem *parent, KstVectorPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_VECTOR), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, i18n("Slave Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListView *parent, KstDataObjectPtr x, KstDataManagerI *dm, int localUseCount)
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_OBJECT), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  for (KstVectorMap::Iterator i = x->outputVectors().begin();
      i != x->outputVectors().end();
      ++i) {
    KstObjectItem *item = new KstObjectItem(this, i.data(), _dm);
    connect(item, SIGNAL(updated()), this, SIGNAL(updated()));
  }
  for (KstMatrixMap::Iterator i = x->outputMatrices().begin();
       i != x->outputMatrices().end();
       ++i) {
    KstObjectItem *item = new KstObjectItem(this, i.data(), _dm);
    connect(item, SIGNAL(updated()), this, SIGNAL(updated()));       
  }
  x = 0L; // keep the counts in sync
  update(false, localUseCount);
}


KstObjectItem::KstObjectItem(QListView *parent, KstRMatrixPtr x, KstDataManagerI *dm, int localUseCount) 
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_DATA_MATRIX), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, i18n("Data Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListView *parent, KstSMatrixPtr x, KstDataManagerI *dm, int localUseCount) 
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_STATIC_MATRIX), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, i18n("Static Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QListViewItem *parent, KstMatrixPtr x, KstDataManagerI *dm, int localUseCount) 
: QObject(), QListViewItem(parent), _rtti(RTTI_OBJ_MATRIX), _name(x->tagName()), _dm(dm) {
  assert(x);
  _inUse = false;
  setText(0, x->tagName());
  setText(1, i18n("Slave Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::~KstObjectItem() {
}


KstDataObjectPtr KstObjectItem::dataObject() {
  return *KST::dataObjectList.findTag(_name);
}


void KstObjectItem::update(bool recursive, int localUseCount) {
  switch (_rtti) {
    case RTTI_OBJ_DATA_VECTOR:
    {
      KstRVectorPtr x = kst_cast<KstRVector>(*KST::vectorList.findTag(_name));
      if (x) {
        x->readLock();
        // getUsage: subtract 1 for KstRVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        QString field;
        if (inUse) {
          field = QString::number(x->length());
        } else {
          field = "-";
        }
        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("%3: %4 [%1..%2]").arg(x->reqStartFrame())
            .arg(x->reqStartFrame() + x->reqNumFrames())
            .arg(x->filename())
            .arg(x->field());
        if (text(4) != field) {
          setText(4, field);
        }
        _removable = x->getUsage() == 2;
        x->unlock();
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
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        QString field;
        if (inUse) {
          field = QString::number(x->length());
        } else {
          field = "-";
        }
        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("%1 to %2").arg(x->min()).arg(x->max());
        if (text(4) != field) {
          setText(4, field);
        }
        _removable = x->getUsage() == 2;
        x->unlock();
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
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        QString field = QString::number(x->length());
        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("[%1..%2]").arg(x->min()).arg(x->max());
        if (text(4) != field) {
          setText(4, field);
        }
        x->unlock();
        _removable = false;
      }
      break;
    }
    case RTTI_OBJ_OBJECT:
    {
      KstDataObjectPtr x = *KST::dataObjectList.findTag(_name);
      if (x) {
        x->readLock();
        QString field = x->typeString();
        if (text(1) != field) {
          setText(1, field);
        }
        // getUsage:
        //  subtract 1 for KstDataObjectPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        if (x->sampleCount() > 0) {
          field = QString::number(x->sampleCount());
          if (text(3) != field) {
            setText(3, field);
          }
        } else {
          if (text(3) != "-") {
            setText(3, "-");
          }          
        }
        field = x->propertyString();
        if (text(4) != field) {
          setText(4, field);
        }
        if (recursive) {
          QPtrStack<QListViewItem> trash;
          KstVectorMap vl = x->outputVectors();
          KstVectorMap::Iterator vlEnd = vl.end();

          for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
            KstObjectItem *oi = static_cast<KstObjectItem*>(i);
            if (vl.findTag(oi->tagName()) == vlEnd) {
              trash.push(i);
            }
          }
          trash.setAutoDelete(true);
          trash.clear();

          // get the output vectors
          for (KstVectorMap::Iterator p = vl.begin(); p != vlEnd; ++p) {
            bool found = false;
            QString tn = p.data()->tagName();
            for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
              KstObjectItem *oi = static_cast<KstObjectItem*>(i);
              if (oi->tagName() == tn) {
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
          
          KstMatrixMap ml = x->outputMatrices();
          KstMatrixMap::Iterator mlEnd = ml.end();
          // also get the output matrices
          for (KstMatrixMap::Iterator p = ml.begin(); p != mlEnd; ++p) {
            bool found = false;
            QString tn = p.data()->tagName();
            for (QListViewItem *i = firstChild(); i; i = i->nextSibling()) {
              KstObjectItem *oi = static_cast<KstObjectItem*>(i);
              if (oi->tagName() == tn) {
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
        x->unlock();
      }
      break;
    }
    case RTTI_OBJ_DATA_MATRIX:
    {
      KstRMatrixPtr x = kst_cast<KstRMatrix>(*KST::matrixList.findTag(_name));
      if (x) {
        x->readLock();
          // getUsage: subtract 1 for KstRMatrixPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        QString field = QString::number(x->sampleCount());
        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("%1: %2 (%3 by %4)").arg(x->filename()).arg(x->field())
                .arg(x->xNumSteps())
                .arg(x->yNumSteps()); 
        if (text(4) != field) {
          setText(4, field);
        }
        _removable = x->getUsage() == 2;
        x->unlock();
      } 
      break;
    }
    case RTTI_OBJ_STATIC_MATRIX:
    {
      KstSMatrixPtr x = kst_cast<KstSMatrix>(*KST::matrixList.findTag(_name));
      if (x) {
        x->readLock();
          // getUsage: subtract 1 for KstRMatrixPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        QString field = QString::number(x->sampleCount());
        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("%1 to %2").arg(x->gradZMin()).arg(x->gradZMax());
        if (text(4) != field) {
          setText(4, field);
        }
        _removable = x->getUsage() == 2;
        x->unlock();
      }
      break;
    }
    case RTTI_OBJ_MATRIX:
    {
      KstMatrixPtr x = *KST::matrixList.findTag(_name);
      if (x) {
        x->readLock();
          // getUsage:
          //  subtract 1 for KstVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        QString field = QString::number(x->sampleCount());
        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("[%1..%2]").arg(x->minValue()).arg(x->maxValue());
        if (text(4) != field) {
          setText(4, field);
        }
        x->unlock();
        _removable = false;
      }
      break;
    }
    default:
      assert(0);
  }
}


void KstObjectItem::updateButtons() {
  _dm->Edit->setEnabled(RTTI_OBJ_VECTOR != _rtti && RTTI_OBJ_MATRIX != _rtti);
  _dm->Delete->setEnabled(RTTI_OBJ_VECTOR != _rtti && RTTI_OBJ_MATRIX != _rtti);
}


void KstObjectItem::reload() {
  if (_rtti == RTTI_OBJ_DATA_VECTOR) {
    KstReadLocker ml(&KST::vectorList.lock());
    KstVectorList::Iterator v = KST::vectorList.findTag(_name);
    if (v != KST::vectorList.end()) {
      KstRVectorPtr r = kst_cast<KstRVector>(*v);
      if (r) {
        r->writeLock();
        r->reload();
        r->unlock();
      }
    }
  } else if (_rtti == RTTI_OBJ_DATA_MATRIX) {
    KstReadLocker ml(&KST::matrixList.lock());
    KstMatrixList::Iterator m = KST::matrixList.findTag(_name);
    if (m != KST::matrixList.end()) {
      KstRMatrixPtr r = kst_cast<KstRMatrix>(*m);
      if (r) {
        r->writeLock();
        r->reload();
        r->unlock();
      }
    }
  }
}


void KstObjectItem::makeCurve() {
  KstCurveDialogI::globalInstance()->show();
  KstCurveDialogI::globalInstance()->setVector(_name);
}

void KstObjectItem::makeCSD() {
  KstCsdDialogI::globalInstance()->show();
  KstCsdDialogI::globalInstance()->setVector(_name);
}

void KstObjectItem::makePSD() {
  KstPsdDialogI::globalInstance()->show();
  KstPsdDialogI::globalInstance()->setVector(_name);
}


void KstObjectItem::makeHistogram() {
  KstHsDialogI::globalInstance()->show();
  KstHsDialogI::globalInstance()->setVector(_name);
}


void KstObjectItem::makeImage() {
  KstImageDialogI::globalInstance()->show();
  KstImageDialogI::globalInstance()->setMatrix(_name);
}


void KstObjectItem::showMetadata() {
  if (_rtti == RTTI_OBJ_DATA_VECTOR) {
    DataSourceMetaDataDialog *dlg = new DataSourceMetaDataDialog(_dm, 0, false, WDestructiveClose);
    KstReadLocker vl(&KST::vectorList.lock());
    KstVectorList::Iterator m = KST::vectorList.findTag(_name);
    KstRVectorPtr r = kst_cast<KstRVector>(*m);
    KstDataSourcePtr dsp;
    if (r) {
      r->readLock();
      dsp = r->dataSource();
      r->unlock();
    }
    dlg->setDataSource(dsp);
    dlg->show();
  } else if (_rtti == RTTI_OBJ_DATA_MATRIX) {
    DataSourceMetaDataDialog *dlg = new DataSourceMetaDataDialog(_dm, 0, false, WDestructiveClose);
    KstReadLocker ml(&KST::matrixList.lock());
    KstMatrixList::Iterator m = KST::matrixList.findTag(_name);
    KstRMatrixPtr r = kst_cast<KstRMatrix>(*m);
    KstDataSourcePtr dsp;
    if (r) {
      r->readLock();
      dsp = r->dataSource();
      r->unlock();
    }
    dlg->setDataSource(dsp);
    dlg->show();
  }
}


void KstObjectItem::activateHint(int id) {
  KstDataObjectPtr d = dataObject();
  const KstCurveHintList* hints = d->curveHints();
  int cnt = 0;
  for (KstCurveHintList::ConstIterator i = hints->begin(); i != hints->end(); ++i) {
    if (cnt == id) {
      KstBaseCurvePtr c = (*i)->makeCurve(KST::suggestCurveName(d->tagName(), false), KstColorSequence::next());
      if (c) {
        KST::dataObjectList.lock().writeLock();
        KST::dataObjectList.append(c.data());
        KST::dataObjectList.lock().unlock();
        emit updated();
      } else {
        KMessageBox::sorry(KstApp::inst(), i18n("Unable to create quick curve."));
      }
      break;
    }
    ++cnt;
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


void KstObjectItem::paintPlot(Kst2DPlotPtr p) {
  KstApp *app = KstApp::inst();
  KMdiIterator<KMdiChildView*> *it = app->createIterator();
  while (it->currentItem()) {
    KstViewWindow *v = dynamic_cast<KstViewWindow*>(it->currentItem());
    if (v && v->view()->contains(kst_cast<KstViewObject>(p))) {
      v->view()->paint(KstPainter::P_PLOT);
      break;
    }
    it->next();
  }
  app->deleteIterator(it);
}


const QPixmap& KstDataManagerI::yesPixmap() const {
  return _yesPixmap;
}


KstDataManagerI::KstDataManagerI(KstDoc *in_doc, QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataManager(parent, name, modal, fl) {
  doc = in_doc;

  _yesPixmap = QPixmap(locate("data", "kst/pics/yes.png"));

  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(Purge, SIGNAL(clicked()), doc, SLOT(purge()));
  connect(DataView, SIGNAL(doubleClicked(QListViewItem *)),
      this, SLOT(edit_I()));
  connect(DataView, SIGNAL(currentChanged(QListViewItem *)),
      this, SLOT(currentChanged(QListViewItem *)));
  connect(DataView, SIGNAL(selectionChanged(QListViewItem *)),
      this, SLOT(currentChanged(QListViewItem *)));

  connect(NewCurve, SIGNAL(clicked()), KstCurveDialogI::globalInstance(), SLOT(show()));
  connect(NewHs, SIGNAL(clicked()), KstHsDialogI::globalInstance(), SLOT(show()));
  connect(NewEq, SIGNAL(clicked()), KstEqDialogI::globalInstance(), SLOT(show()));
  connect(NewPlugin, SIGNAL(clicked()), KstPluginDialogI::globalInstance(), SLOT(show()));
  connect(NewEvent, SIGNAL(clicked()), KstEventMonitorI::globalInstance(), SLOT(show()));
  connect(NewPSD, SIGNAL(clicked()), KstPsdDialogI::globalInstance(), SLOT(show()));
  connect(NewVector, SIGNAL(clicked()), KstVectorDialogI::globalInstance(), SLOT(show()));
  connect(OpenPlotDialog, SIGNAL(clicked()),
      KstApp::inst(), SLOT(showPlotDialog()));

  connect(NewImage, SIGNAL(clicked()), KstImageDialogI::globalInstance(), SLOT(show()));

  connect(NewMatrix, SIGNAL(clicked()), KstMatrixDialogI::globalInstance(), SLOT(show()));
  
  connect(NewCSD, SIGNAL(clicked()), KstCsdDialogI::globalInstance(), SLOT(show()));

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
  KST::matrixList.lock().writeLock();

  // garbage collect first
  for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
    KstObjectItem *oi = static_cast<KstObjectItem*>(i);
    if (i->rtti() == RTTI_OBJ_OBJECT) {
      if (KST::dataObjectList.findTag(oi->tagName()) == KST::dataObjectList.end()) {
        trash.push(i);
      }
    } else if (i->rtti() == RTTI_OBJ_DATA_MATRIX || 
               i->rtti() == RTTI_OBJ_MATRIX ||
               i->rtti() == RTTI_OBJ_STATIC_MATRIX) {
      if (KST::matrixList.findTag(oi->tagName()) == KST::matrixList.end()) {
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

  KST::matrixList.lock().unlock();
  KST::vectorList.lock().unlock();
  KST::dataObjectList.lock().unlock();

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

  // update the data matrices 
  KstRMatrixList rml = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);
  for (KstRMatrixList::iterator it = rml.begin(); it != rml.end(); ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_DATA_MATRIX && oi->tagName() == (*it)->tagName()) {
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

  // update the static matrices
  KstSMatrixList sml = kstObjectSubList<KstMatrix,KstSMatrix>(KST::matrixList);
  for (KstSMatrixList::iterator it = sml.begin(); it != sml.end(); ++it) {
    bool found = false;
    for (QListViewItem *i = DataView->firstChild(); i; i = i->nextSibling()) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(i);
      if (oi->rtti() == RTTI_OBJ_STATIC_MATRIX && oi->tagName() == (*it)->tagName()) {
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
  
  if (qi->rtti() == RTTI_OBJ_DATA_MATRIX) {
    emit editDataMatrix(qi->text(0));  
  }
  
  if (qi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
    emit editStaticMatrix(qi->text(0));  
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
      KST::vectorList.lock().unlock();
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_STATIC_VECTOR) {
      KST::vectorList.lock().writeLock();
      KST::vectorList.removeTag(koi->tagName());
      KST::vectorList.lock().unlock();
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_DATA_MATRIX) {
      KST::matrixList.lock().writeLock();
      KST::matrixList.removeTag(koi->tagName());
      KST::matrixList.lock().unlock();  
    } else if (qi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
      KST::matrixList.lock().writeLock();
      KST::matrixList.removeTag(koi->tagName());
      KST::matrixList.lock().unlock();  
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
          KST::vectorList.lock().unlock();
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
          KST::vectorList.lock().unlock();
          doUpdates();
        } else {
          KMessageBox::sorry(this, i18n("Unknown error deleting static vector."));
        }
      } else if (qi->rtti() == RTTI_OBJ_DATA_MATRIX) {
        KstRMatrixPtr x = kst_cast<KstRMatrix>(*KST::matrixList.findTag(koi->tagName()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::matrixList.lock().writeLock();
          KST::matrixList.removeTag(koi->tagName());
          KST::matrixList.lock().unlock();
          doUpdates();
        } else {
          KMessageBox::sorry(this, i18n("Unknown error deleting data matrix."));
        }
      } else if (qi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
        KstSMatrixPtr x = kst_cast<KstSMatrix>(*KST::matrixList.findTag(koi->tagName()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::matrixList.lock().writeLock();
          KST::matrixList.removeTag(koi->tagName());
          KST::matrixList.lock().unlock();
          doUpdates();
        } else {
          KMessageBox::sorry(this, i18n("Unknown error deleting static matrix."));
        }  
      }
      KstApp::inst()->paintAll(KstPainter::P_PLOT);
      update();
    } else {
      KMessageBox::sorry(this, i18n("Cannot delete objects with dependencies."));
    }
  }
}


// Menu IDs:
// 100->499 reserved for plots
// 500->999 reserved for filters

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
  if (koi->rtti() != RTTI_OBJ_VECTOR && koi->rtti() != RTTI_OBJ_MATRIX) {
    id = m->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
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
    id = m->insertItem(i18n("Make Cumulative &Spectral Decay..."), koi, SLOT(makeCSD()));
    id = m->insertItem(i18n("Make &Histogram..."), koi, SLOT(makeHistogram()));
    id = m->insertItem(i18n("&Reload"), koi, SLOT(reload()));
    id = m->insertItem(i18n("Meta &Data"), koi, SLOT(showMetadata()));
  } else if (koi->rtti() == RTTI_OBJ_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
    id = m->insertItem(i18n("Make &Power Spectrum..."), koi, SLOT(makePSD()));
    id = m->insertItem(i18n("Make Cumulative &Spectral Decay..."), koi, SLOT(makeCSD()));
    id = m->insertItem(i18n("Make &Histogram..."), koi, SLOT(makeHistogram()));
  } else if (koi->rtti() == RTTI_OBJ_DATA_MATRIX) {
    id = m->insertItem(i18n("Make &Image..."), koi, SLOT(makeImage()));  
    id = m->insertItem(i18n("&Reload"), koi, SLOT(reload()));
    id = m->insertItem(i18n("Meta &Data"), koi, SLOT(showMetadata()));
  } else if (koi->rtti() == RTTI_OBJ_MATRIX || koi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
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
  } 

  if (koi->rtti() != RTTI_OBJ_VECTOR && koi->rtti() != RTTI_OBJ_MATRIX) {
    // no slave vectors or matrices get this
    id = m->insertItem(i18n("&Delete"), this, SLOT(delete_I()));
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
