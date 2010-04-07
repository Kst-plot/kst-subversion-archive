/***************************************************************************
                       kstdatamanger.cpp  -  Part of KST
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

#include <QComboBox>
#include <QListWidget>
#include <QMessageBox>
#include <QStack>
#include <QStyleFactory>
#include <QStyle>
#include <QToolBox>
#include <QToolButton>

#include <kinputdialog.h>
#include <kstandarddirs.h>

#include "datasourcemetadatadialog.h"
#include "kst2dplot.h"
#include "kstcurvedialog.h"
#include "kstcsddialog.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstdatamanager.h"
#include "kstdoc.h"
#include "ksteqdialog.h"
#include "ksteventmonitor.h"
#include "ksthsdialog.h"
#include "kstimagedialog.h"
#include "kstmatrixdialog.h"
#include "kstplugindialog.h"
#include "kstpsddialog.h"
#include "kstvectordialog.h"
#include "kstvvdialog.h"
#include "kstviewwindow.h"
#include "matrixselector.h"
#include "vectorselector.h"
#include "plugincollection.h"

static QStyle *windowsStyle = 0;

static QMap<int,Kst2DPlotPtr> PlotMap;

#define RTTI_OBJ_VECTOR          4201
#define RTTI_OBJ_OBJECT          4202
#define RTTI_OBJ_DATA_VECTOR     4203
#define RTTI_OBJ_STATIC_VECTOR   4204
#define RTTI_OBJ_MATRIX          4205
#define RTTI_OBJ_DATA_MATRIX     4206
#define RTTI_OBJ_STATIC_MATRIX   4207
#define RTTI_OBJ_A_VECTOR        4208
#define RTTI_OBJ_A_MATRIX        4209

KstDataAction::KstDataAction(const QString &menuText, QKeySequence accel, QObject *parent, const char *name ) : QAction(menuText, parent) {
  setObjectName(name);
  setShortcut(accel);
}


void KstDataAction::addedTo(QWidget *actionWidget, QWidget *container) {
  Q_UNUSED(container)

  if (dynamic_cast<QToolButton*>(actionWidget) ) {
    if (!windowsStyle) {
      windowsStyle = QStyleFactory::create("windows");
    }

    actionWidget->setStyle(windowsStyle);
/* xxx
    ((QToolButton*)actionWidget)->setUsesTextLabel(true);
    ((QToolButton*)actionWidget)->setTextPosition(QToolButton::Right);

    actionWidget->setBackgroundMode(PaletteBase);
*/
  }
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstRVectorPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_DATA_VECTOR), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Data Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstSVectorPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_STATIC_VECTOR), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Static Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstAVectorPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_A_VECTOR), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Static Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidgetItem *parent, KstVectorPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_VECTOR), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Slave Vector"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstDataObjectPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_OBJECT), _tag(x->tag()), _dm(dm) {
  KstObjectItem *item;
  KstVectorMap::iterator vi;
  KstMatrixMap::Iterator mi;

  _inUse = false;
  setText(0, x->tag().tag());
  for (vi = x->outputVectors().begin(); vi != x->outputVectors().end(); ++vi) {
    item = new KstObjectItem(this, *vi, _dm);
    connect(item, SIGNAL(updated()), this, SIGNAL(updated()));
  }
  for (mi = x->outputMatrices().begin(); mi != x->outputMatrices().end(); ++mi) {
    item = new KstObjectItem(this, *mi, _dm);
    connect(item, SIGNAL(updated()), this, SIGNAL(updated()));       
  }
  x = 0L; // keep the counts in sync
  update(false, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstRMatrixPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_DATA_MATRIX), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Data Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstSMatrixPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_STATIC_MATRIX), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Static Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidget *parent, KstAMatrixPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_A_MATRIX), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Static Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::KstObjectItem(QTreeWidgetItem *parent, KstMatrixPtr x, KstDataManager *dm, int localUseCount) : QObject(), QTreeWidgetItem(parent), _rtti(RTTI_OBJ_MATRIX), _tag(x->tag()), _dm(dm) {
  _inUse = false;
  setText(0, x->tag().tag());
  setText(1, i18n("Slave Matrix"));
  x = 0L; // keep the counts in sync
  update(true, localUseCount);
}


KstObjectItem::~KstObjectItem() {
}


KstDataObjectPtr KstObjectItem::dataObject() {
  return *KST::dataObjectList.findTag(_tag.tag());
}


void KstObjectItem::update(bool recursive, int localUseCount) {
  QString field;

  switch (_rtti) {
    case RTTI_OBJ_DATA_VECTOR:
    {
      KstRVectorPtr x;

      KST::vectorList.lock().readLock();
      x = kst_cast<KstRVector>(*KST::vectorList.findTag(_tag));
      KST::vectorList.lock().unlock();

      if (x) {
        x->readLock();
        // getUsage: subtract 1 for KstRVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }

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
      KstSVectorPtr x;

      KST::vectorList.lock().readLock();
      x = kst_cast<KstSVector>(*KST::vectorList.findTag(_tag));
      KST::vectorList.lock().unlock();

      if (x) {
        x->readLock();
        // getUsage: subtract 1 for KstRVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
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

    case RTTI_OBJ_A_VECTOR:
    {
      KstAVectorPtr x;

      KST::vectorList.lock().readLock();
      x = kst_cast<KstAVector>(*KST::vectorList.findTag(_tag));
      KST::vectorList.lock().unlock();

      if (x) {
        x->readLock();
        // getUsage: subtract 1 for KstRVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }

        if (inUse) {
          field = QString::number(x->length());
        } else {
          field = "-";
        }

        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("[%1..%2]").arg(x->min()).arg(x->max());
        if (text(4) != field) {
          setText(4, field);
        }
        _removable = false;
        x->unlock();
      }
      // Hmmm what happens if this if() fails??  We become inconsistent?
      break;
    }

    case RTTI_OBJ_VECTOR:
    {
      KstVectorPtr x;

      KST::vectorList.lock().readLock();
      x = *KST::vectorList.findTag(_tag);
      KST::vectorList.lock().unlock();

      if (x) {
        x->readLock();
        // getUsage:
        //  subtract 1 for KstVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }
        field = QString::number(x->length());
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
      KstDataObjectPtr x;

      KST::dataObjectList.lock().readLock();
      x = *KST::dataObjectList.findTag(_tag.tag());
      KST::dataObjectList.lock().unlock();

      if (x) {
        x->readLock();

        field = x->typeString();
        if (text(1) != field) {
          setText(1, field);
        }
        // getUsage:
        //  subtract 1 for KstDataObjectPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
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
          QStack<QTreeWidgetItem*> trash;
          KstVectorMap vl = x->outputVectors();
          KstVectorMap::iterator vlEnd = vl.end();
          int i;

          for (i = 0; i < childCount(); ++i) {
            KstObjectItem *oi = static_cast<KstObjectItem*>(child(i));
/* xxx
            if (vl.findTag(oi->tag().tag()) == vlEnd) {
              trash.push(child(i));
            }
*/
          }

          while (!trash.isEmpty()) {
            delete trash.pop();
          }

          trash.clear();

          //
          // get the output vectors...
          //

          for (KstVectorMap::Iterator p = vl.begin(); p != vlEnd; ++p) {
            bool found = false;
            QString tn = (*p)->tag().tag();
            int i;

            for (i = 0; i < childCount(); ++i) {
              KstObjectItem *oi = static_cast<KstObjectItem*>(child(i));

              if (oi->tag().tag() == tn) {
                oi->update();
                found = true;

                break;
              }
            }

            if (!found) {
              KstObjectItem *item = new KstObjectItem(this, *p, _dm);

              connect(item, SIGNAL(updated()), this, SIGNAL(updated()));
            }
          }

          KstMatrixMap ml = x->outputMatrices();
          KstMatrixMap::Iterator mlEnd = ml.end();

          //
          // also get the output matrices...
          //

          for (KstMatrixMap::Iterator p = ml.begin(); p != mlEnd; ++p) {
            bool found = false;
            QString tn = (*p)->tag().tag();
            int i;

            for (i = 0; i < childCount(); ++i) {
              KstObjectItem *oi = static_cast<KstObjectItem*>(child(i));

              if (oi->tag().tag() == tn) {
                oi->update();
                found = true;

                break;
              }
            }

            if (!found) {
              KstObjectItem *item = new KstObjectItem(this, *p, _dm);

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
      KstRMatrixPtr x;

      KST::matrixList.lock().readLock();
      x = kst_cast<KstRMatrix>(*KST::matrixList.findTag(_tag));
      KST::matrixList.lock().unlock();

      if (x) {
        x->readLock();
          // getUsage: subtract 1 for KstRMatrixPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }

        if (inUse) {
          field = QString::number(x->sampleCount());
        } else {
          field = "-";
        }

        field = i18n("%1: %2 (%3 by %4)").arg(x->filename()).arg(x->field())
                .arg(x->xNumSteps()).arg(x->yNumSteps());
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
      KstSMatrixPtr x;

      KST::matrixList.lock().readLock();
      x = kst_cast<KstSMatrix>(*KST::matrixList.findTag(_tag));
      KST::matrixList.lock().unlock();

      if (x) {
        x->readLock();
          // getUsage: subtract 1 for KstRMatrixPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }

        if (inUse) {
          field = QString::number(x->sampleCount());
        } else {
          field = "-";
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

    case RTTI_OBJ_A_MATRIX:
    {
      KstAMatrixPtr x;

      KST::matrixList.lock().readLock();
      x = kst_cast<KstAMatrix>(*KST::matrixList.findTag(_tag));
      KST::matrixList.lock().unlock();

      if (x) {
        x->readLock();
          // getUsage: subtract 1 for KstRMatrixPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }

        if (inUse) {
          field = QString::number(x->sampleCount());
        } else {
          field = "-";
        }

        if (text(3) != field) {
          setText(3, field);
        }
        field = i18n("[%1..%2]").arg(x->minValue()).arg(x->maxValue());
        if (text(4) != field) {
          setText(4, field);
        }
        _removable = false;
        x->unlock();
      }
      break;
    }

    case RTTI_OBJ_MATRIX:
    {
      KstMatrixPtr x;

      KST::matrixList.lock().readLock();
      x = *KST::matrixList.findTag(_tag);
      KST::matrixList.lock().unlock();

      if (x) {
        x->readLock();
          // getUsage:
          //  subtract 1 for KstVectorPtr x
        bool inUse = (x->getUsage() - 1 - localUseCount) > 0;
        if (inUse != _inUse) {
          _inUse = inUse;
// xxx          setPixmap(2, inUse ? _dm->yesPixmap() : QPixmap());
        }

        if (inUse) {
          field = QString::number(x->sampleCount());
        } else {
          field = "-";
        }

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
    KstVectorList::Iterator v = KST::vectorList.findTag(_tag);
    if (v != KST::vectorList.end()) {
      KstRVectorPtr r;

      r = kst_cast<KstRVector>(*v);
      if (r) {
        r->writeLock();
        r->reload();
        r->unlock();
      }
    }
  } else if (_rtti == RTTI_OBJ_DATA_MATRIX) {
    KstReadLocker ml(&KST::matrixList.lock());
    KstMatrixList::Iterator m = KST::matrixList.findTag(_tag);
    if (m != KST::matrixList.end()) {
      KstRMatrixPtr r;

      r = kst_cast<KstRMatrix>(*m);
      if (r) {
        r->writeLock();
        r->reload();
        r->unlock();
      }
    }
  }
}


void KstObjectItem::makeCurve() {
  KstCurveDialog::globalInstance()->show();
  KstCurveDialog::globalInstance()->setVector(_tag.tagString());
}


void KstObjectItem::makeCSD() {
  KstCsdDialog::globalInstance()->show();
  KstCsdDialog::globalInstance()->setVector(_tag.tagString());
}


void KstObjectItem::makePSD() {
  KstPsdDialog::globalInstance()->show();
  KstPsdDialog::globalInstance()->setVector(_tag.tagString());
}


void KstObjectItem::makeHistogram() {
  KstHsDialog::globalInstance()->show();
  KstHsDialog::globalInstance()->setVector(_tag.tagString());
}


void KstObjectItem::makeImage() {
  KstImageDialog::globalInstance()->show();
  KstImageDialog::globalInstance()->setMatrix(_tag.tagString());
}


void KstObjectItem::viewVectorValues() {
  KstApp::inst()->showViewVectorsDialog(_tag.tagString());
}


void KstObjectItem::viewMatrixValues() {
  KstApp::inst()->showViewMatricesDialog(_tag.tagString());
}


void KstObjectItem::showMetadata() {
  if (_rtti == RTTI_OBJ_DATA_VECTOR) {
    DataSourceMetaDataDialog *dlg = new DataSourceMetaDataDialog(_dm, 0, false);
    KstRVectorPtr r;

    dlg->setAttribute(Qt::WA_DeleteOnClose);

    KstReadLocker vl(&KST::vectorList.lock());
    KstVectorList::iterator m = KST::vectorList.findTag(_tag);
    r = kst_cast<KstRVector>(*m);
    KstDataSourcePtr dsp;

    if (r) {
      r->readLock();
      dsp = r->dataSource();
      r->unlock();
    }
    dlg->setDataSource(dsp);
    dlg->show();
  } else if (_rtti == RTTI_OBJ_DATA_MATRIX) {
    DataSourceMetaDataDialog *dlg = new DataSourceMetaDataDialog(_dm, 0, false);
    KstRMatrixPtr r;

    dlg->setAttribute(Qt::WA_DeleteOnClose);

    KstReadLocker ml(&KST::matrixList.lock());
    KstMatrixList::iterator m = KST::matrixList.findTag(_tag);
    r = kst_cast<KstRMatrix>(*m);
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
  KstCurveHintList::ConstIterator i;
  const KstCurveHintList* hints = d->curveHints();
  int cnt = 0;

  for (i = hints->begin(); i != hints->end(); ++i) {
    if (cnt == id) {
      KstBaseCurvePtr c;

      c = (*i)->makeCurve(KST::suggestCurveName(d->tag(), false), KstColorSequence::next());
      if (c) {
        KST::dataObjectList.lock().writeLock();
        KST::dataObjectList.append(c);
        KST::dataObjectList.lock().unlock();
        emit updated();
      } else {
        QMessageBox::warning(KstApp::inst(), i18n("Kst"), i18n("Unable to create quick curve."));
      }
      break;
    }
    ++cnt;
  }
}


void KstObjectItem::addToPlot(int id) {
  Kst2DPlotPtr p;
  KstBaseCurvePtr c;

  p = PlotMap[id];
  c = kst_cast<KstBaseCurve>(dataObject());
  if (p && c) {
    p->addCurve(c);
    p->setDirty();
    paintPlot(p);
    emit updated();
  }
}


void KstObjectItem::removeFromPlot(int id) {
  Kst2DPlotPtr p;
  KstBaseCurvePtr c;

  p = PlotMap[id];
  c = kst_cast<KstBaseCurve>(dataObject());
  if (p && c) {
    p->removeCurve(c);
    p->setDirty();
    paintPlot(p);

    emit updated();
  }
}


void KstObjectItem::paintPlot(Kst2DPlotPtr p) {
  KstApp *app = KstApp::inst();
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);

    if (viewWindow) {
/* xxx
      if (viewWindow->view()->contains(kst_cast<KstViewObject>(p))) {
        viewWindow->view()->paint(KstPainter::P_PLOT);
        
        break;
      }
*/
    }
  }
}


const QPixmap& KstDataManager::yesPixmap() const {
  return _yesPixmap;
}

KstDataManager::KstDataManager(KstDoc *in_doc, QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl) {
  QList<int> cols;

  setupUi(this);
  doc = in_doc;

// xxx  _yesPixmap = QPixmap(locate("data", "kst/pics/yes.png"));

  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(Purge, SIGNAL(clicked()), doc, SLOT(purge()));
  connect(DataView, SIGNAL(doubleClicked(QTreeWidgetItem*)),
      this, SLOT(edit_I()));
  connect(DataView, SIGNAL(currentChanged(QTreWidgetItem*)), 
      this, SLOT(currentChanged(QTreeWidgetItem*)));
  connect(DataView, SIGNAL(selectionChanged()),
      this, SLOT(selectionChanged()));
  connect(DataView, SIGNAL(contextMenuRequested(QTreeWidgetItem*, const QPoint&, int)), 
      this, SLOT(contextMenu(QTreeWidgetItem*, const QPoint&, int)));

// xxx  _searchWidget = new KListViewSearchLineWidget(DataView, SearchBox);

  cols.append(0);
/* xxx
  _searchWidget->createSearchLine(DataView);
  _searchWidget->searchLine()->setSearchColumns(cols);

  QMainWindow *main = static_cast<QMainWindow*>(parent);
  main->setUsesTextLabel(true);

  _primitive = new QToolBar(i18n("Create Primitive"), main, this);
  _primitive->setFrameStyle(QFrame::NoFrame);
  _primitive->setOrientation(Qt::Vertical);
  _primitive->setBackgroundMode(PaletteBase);

  _data = new QToolBar(i18n("Create Data Object"), main, this);
  _data->setFrameStyle(QFrame::NoFrame);
  _data->setOrientation(Qt::Vertical);
  _data->setBackgroundMode(PaletteBase);

  _plugins = new QToolBar(i18n("Create Plugin"), main, this);
  _plugins->setFrameStyle(QFrame::NoFrame);
  _plugins->setOrientation(Qt::Vertical);
  _plugins->setBackgroundMode(PaletteBase);

  _fits = new QToolBar(i18n("Create Fit"), main, this);
  _fits->setFrameStyle(QFrame::NoFrame);
  _fits->setOrientation(Qt::Vertical);
  _fits->setBackgroundMode(PaletteBase);

  _filters = new QToolBar(i18n("Create Filter"), main, this);
  _filters->setFrameStyle(QFrame::NoFrame);
  _filters->setOrientation(Qt::Vertical);
  _filters->setBackgroundMode(PaletteBase);

  ToolBox->setUpdatesEnabled(false);

  _primitive->setUpdatesEnabled(false);
  _primitive->clear();

  _data->setUpdatesEnabled(false);
  _data->clear();

  _plugins->setUpdatesEnabled(false);
  _plugins->clear();

  _fits->setUpdatesEnabled(false);
  _fits->clear();

  _filters->setUpdatesEnabled(false);
  _filters->clear();
*/

  //
  // create canonical actions...
  //

//   createObjectAction(i18n("Scalar"), _primitive, KstScalarDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Vector"), _primitive, KstVectorDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Matrix"), _primitive, KstMatrixDialog::globalInstance(), SLOT(show()));
//   createObjectAction(i18n("String"), _primitive, KstStringDialog::globalInstance(), SLOT(show()));

  createObjectAction(i18n("Curve"), _data, KstCurveDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Equation"), _data, KstEqDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Histogram"), _data, KstHsDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Spectrum"), _data, KstPsdDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Event Monitor"), _data, KstEventMonitor::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Image"), _data, KstImageDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Spectrogram"), _data, KstCsdDialog::globalInstance(), SLOT(show()));
  createObjectAction(i18n("Vector View"), _data, KstVvDialog::globalInstance(), SLOT(show()));
  
  setupPluginActions();
/* xxx
  QWidget *priw = new QWidget(_primitive);
  priw->setBackgroundMode(PaletteBase);
  _primitive->setStretchableWidget(priw);

  QWidget *datw = new QWidget(_data);
  datw->setBackgroundMode(PaletteBase);
  _data->setStretchableWidget(datw);

  QWidget *pluginw = new QWidget(_plugins);
  pluginw->setBackgroundMode(PaletteBase);
  _plugins->setStretchableWidget(pluginw);

  QWidget *fitw = new QWidget(_fits);
  fitw->setBackgroundMode(PaletteBase);
  _fits->setStretchableWidget(fitw);

  QWidget *filw = new QWidget(_filters);
  filw->setBackgroundMode(PaletteBase);
  _filters->setStretchableWidget(filw);

  ToolBox->setUpdatesEnabled(true);

  _primitive->setUpdatesEnabled(true);
  _data->setUpdatesEnabled(true);
  _plugins->setUpdatesEnabled(true);
  _fits->setUpdatesEnabled(true);
  _filters->setUpdatesEnabled(true);

  ToolBox->addItem(_primitive, i18n("Create Primitive"));
  ToolBox->addItem(_data, i18n("Create Data Object"));
  ToolBox->addItem(_plugins, i18n("Create Plugin"));
  ToolBox->addItem(_fits, i18n("Create Fit"));
  ToolBox->addItem(_filters, i18n("Create Filter"));
*/
}


KstDataManager::~KstDataManager() {
}


void KstDataManager::createObjectAction(const QString &txt, QToolBar *bar,
                                         QObject *receiver, const char *slot) {
  KstDataAction *a;

  a = new KstDataAction(txt, QKeySequence(), bar);
/* xxx
  a->addTo(bar);
  if (receiver && slot) {
    connect(a, SIGNAL(activated()), receiver, slot);
  }
*/
}


void KstDataManager::setupPluginActions() {
  //
  // the new KstDataObject plugins...
  //

  {
    const KstPluginInfoList newPlugins = KstDataObject::pluginInfoList();
    KstPluginInfoList::const_iterator it = newPlugins.begin();

    for (; it != newPlugins.end(); ++it) {
      KstDataObjectPtr ptr;

      ptr = KstDataObject::plugin(it.key());
      if (!ptr) {
        continue;
      }

      switch(*it) {
        case KstDataObject::Generic:
          createObjectAction(it.key(), _data, ptr.data(), SLOT(showNewDialog()));
          break;

        case KstDataObject::KstPlugin:
          createObjectAction(it.key(), _plugins, ptr.data(), SLOT(showNewDialog()));
          break;

        case KstDataObject::Primitive:
          createObjectAction(it.key(), _primitive, ptr.data(), SLOT(showNewDialog()));
          break;

        case KstDataObject::Fit:
          createObjectAction(it.key(), _fits, ptr.data(), SLOT(showNewDialog()));
          break;

        case KstDataObject::Filter:
          createObjectAction(it.key(), _filters, ptr.data(), SLOT(showNewDialog()));
          break;

        default:
          break;
      }
    }
  }

  //
  // the old C style plugins...
  //   
  
  QStringList oldPlugins;
  const QMap<QString,QString> readable = PluginCollection::self()->readableNameList();
  QMap<QString,QString>::const_iterator it = readable.begin();
  
  for (; it != readable.end(); ++it) {
    oldPlugins << it.key();
  }

  {
    QStringList::const_iterator it = oldPlugins.begin();

    for (; it != oldPlugins.end(); ++it) {
      QExplicitlySharedDataPointer<Plugin> p;

      if (p = PluginCollection::self()->plugin(readable[*it])) {
        if (p->data()._isFit) {
          createObjectAction(*it, _fits, this, SLOT(showOldPlugin()));
        } else if (p->data()._isFilter) {
          createObjectAction(*it, _filters, this, SLOT(showOldPlugin()));
        } else {
          createObjectAction(*it, _plugins, this, SLOT(showOldPlugin()));
        }
      }
    }
  }
}


void KstDataManager::showOldPlugin() {
  if (QAction *a = dynamic_cast<QAction*>(sender())) {
    const QMap<QString,QString> readable =
      PluginCollection::self()->readableNameList();

    KstPluginDialog::globalInstance()->showNew(readable[a->text()]);
  }
}


void KstDataManager::doubleClicked(QTreeWidgetItem *i) {
  if (i && DataView->selectedItems().contains(i)) {
    edit_I();
  }
}

void KstDataManager::show_I() {
  show();
  raise();
  update();
}


void KstDataManager::updateContents() {
  if (!isVisible()) {
    return;
  }

  for (int i = 0; i < DataView->topLevelItemCount(); ++i) {
    KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));
    oi->update();
  }

// xxx  _searchWidget->searchLine()->updateSearch();
}


void KstDataManager::update() {
  if (!isVisible()) {
    return;
  }

  QTreeWidgetItem *currentItem = 0L;
  QStack<QTreeWidgetItem*> trash;

  if (DataView->selectedItems().count() > 0) {
    currentItem = DataView->selectedItems().front();
  }

  KST::dataObjectList.lock().writeLock();
  KST::vectorList.lock().writeLock();
  KST::matrixList.lock().writeLock();

  //
  // garbage collect first...
  //

  for (int i = 0; i < DataView->topLevelItemCount(); ++i) {
    KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

    if (oi->rtti() == RTTI_OBJ_OBJECT) {
      if (KST::dataObjectList.findTag(oi->tag().tagString()) == KST::dataObjectList.end()) {
        trash.push(DataView->topLevelItem(i));
      }
    } else if (oi->rtti() == RTTI_OBJ_DATA_MATRIX || 
               oi->rtti() == RTTI_OBJ_MATRIX ||
               oi->rtti() == RTTI_OBJ_STATIC_MATRIX ||
               oi->rtti() == RTTI_OBJ_A_MATRIX) {
      if (KST::matrixList.findTag(oi->tag().tagString()) == KST::matrixList.end()) {
        trash.push(DataView->topLevelItem(i));
      }
    } else {
      if (KST::vectorList.findTag(oi->tag().tagString()) == KST::vectorList.end()) {
        trash.push(DataView->topLevelItem(i));
      }
    }
  }

  DataView->blockSignals(true);

  while (!trash.isEmpty()) {
    delete trash.pop();
  }

  DataView->blockSignals(false);

  KstDataObjectList::iterator doit;

  for (doit = KST::dataObjectList.begin(); doit != KST::dataObjectList.end(); ++doit) {
    KstReadLocker dol((*doit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_OBJECT && oi->tag().tagString() == (*doit)->tag().tagString()) {
        oi->update();
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *doit, this);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  KST::dataObjectList.lock().unlock();

  //
  // update the data vectors...
  //

  KstRVectorList rvl;
  KstRVectorList::iterator rvlit;

  rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (rvlit = rvl.begin(); rvlit != rvl.end(); ++rvlit) {
    KstReadLocker vl((*rvlit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_DATA_VECTOR && oi->tag().tagString() == (*rvlit)->tag().tagString()) {
        oi->update(true, 1);
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *rvlit, this, 1);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  //
  // update the static vectors...
  //

  KstSVectorList svl;
  KstSVectorList::iterator svlit;

  svl = kstObjectSubList<KstVector,KstSVector>(KST::vectorList);
  for (svlit = svl.begin(); svlit != svl.end(); ++svlit) {
    KstReadLocker vl((*svlit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_STATIC_VECTOR && oi->tag().tagString() == (*svlit)->tag().tagString()) {
        oi->update(true, 1);
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *svlit, this, 1);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  //
  // update the a vectors...
  //

  KstAVectorList avl;
  KstAVectorList::iterator avlit;

  avl = kstObjectSubList<KstVector,KstAVector>(KST::vectorList);
  for (avlit = avl.begin(); avlit != avl.end(); ++avlit) {
    KstReadLocker vl((*avlit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_A_VECTOR && oi->tag().tagString() == (*avlit)->tag().tagString()) {
        oi->update(true, 1);
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *avlit, this, 1);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  KST::vectorList.lock().unlock();

  //
  // update the data matrices...
  //

  KstRMatrixList rml;
  KstRMatrixList::iterator rmlit;

  rml = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);
  for (rmlit = rml.begin(); rmlit != rml.end(); ++rmlit) {
    KstReadLocker ml((*rmlit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_DATA_MATRIX && oi->tag().tagString() == (*rmlit)->tag().tagString()) {
        oi->update(true, 1);
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *rmlit, this, 1);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  //
  // update the static matrices...
  //

  KstSMatrixList sml;
  KstSMatrixList::iterator smlit;

  sml = kstObjectSubList<KstMatrix,KstSMatrix>(KST::matrixList);
  for (smlit = sml.begin(); smlit != sml.end(); ++smlit) {
    KstReadLocker ml((*smlit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_STATIC_MATRIX && oi->tag().tagString() == (*smlit)->tag().tagString()) {
        oi->update(true, 1);
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *smlit, this, 1);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  //
  // update the a matrices...
  //

  KstAMatrixList aml;
  KstAMatrixList::iterator amlit;

  aml = kstObjectSubList<KstMatrix,KstAMatrix>(KST::matrixList);
  for (amlit = aml.begin(); amlit != aml.end(); ++amlit) {
    KstReadLocker ml((*amlit).data());
    bool found = false;
    int i;

    for (i = 0; i < DataView->topLevelItemCount(); ++i) {
      KstObjectItem *oi = static_cast<KstObjectItem*>(DataView->topLevelItem(i));

      if (oi->rtti() == RTTI_OBJ_A_MATRIX && oi->tag().tagString() == (*amlit)->tag().tagString()) {
        oi->update(true, 1);
        found = true;

        break;
      }
    }

    if (!found) {
      KstObjectItem *i = new KstObjectItem(DataView, *amlit, this, 1);

      connect(i, SIGNAL(updated()), this, SLOT(doUpdates()));
    }
  }

  KST::matrixList.lock().unlock();
/* xxx
  // is this really necessary?  I would think not...
  for (int i = 0; i < DataView->topLevelItemCount(); ++i) {
    if (topLevelItem(i) == currentItem) {
      DataView->setCurrentItem(topLevelItem(i));
      DataView->setSelected(i, true);

      break;
    }
  }
*/
  if (!DataView->selectedItems().isEmpty()) {
    static_cast<KstObjectItem*>(DataView->currentItem())->updateButtons();
  } else {
    Edit->setEnabled(false);
    Delete->setEnabled(false);
  }
}


void KstDataManager::edit_I() {
  QTreeWidgetItem *qi = 0L;

  if (!DataView->selectedItems().isEmpty()) {
    qi = DataView->selectedItems().at(0);
/* xxx
    switch (qi->rtti()) {
      case RTTI_OBJ_DATA_VECTOR:
        emit editDataVector(qi->text(0));
        break;
  
      case RTTI_OBJ_STATIC_VECTOR:
        emit editStaticVector(qi->text(0));
        break;
  
      case RTTI_OBJ_A_VECTOR:
        break;
  
      case RTTI_OBJ_OBJECT:
        static_cast<KstObjectItem*>(qi)->dataObject()->showDialog(false);
        break;
  
      case RTTI_OBJ_DATA_MATRIX:
        emit editDataMatrix(qi->text(0));  
        break;
  
      case RTTI_OBJ_STATIC_MATRIX:
        emit editStaticMatrix(qi->text(0));  
        break;
  
      case RTTI_OBJ_A_MATRIX:
        break;
  
      default:
        break;
    }
*/
  }
}


void KstDataManager::delete_I() {
  QTreeWidgetItem *qi = 0L;
  KstObjectItem *koi;
  
  if (!DataView->selectedItems().isEmpty()) {
    qi = DataView->selectedItems().at(0);
  }

  if (!qi) {
    return;
  }
/* xxx
  koi = static_cast<KstObjectItem*>(qi);
  if (koi->removable()) {
    if (qi->rtti() == RTTI_OBJ_OBJECT) {
      doc->removeDataObject(koi->tag().tagString());
    } else if (qi->rtti() == RTTI_OBJ_DATA_VECTOR) {
      KST::vectorList.lock().writeLock();
      KST::vectorList.removeTag(koi->tag().tagString());
      KST::vectorList.lock().unlock();
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_STATIC_VECTOR) {
      KST::vectorList.lock().writeLock();
      KST::vectorList.removeTag(koi->tag().tagString());
      KST::vectorList.lock().unlock();
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_A_VECTOR) {
      KST::vectorList.lock().writeLock();
      KST::vectorList.removeTag(koi->tag().tagString());
      KST::vectorList.lock().unlock();
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_DATA_MATRIX) {
      KST::matrixList.lock().writeLock();
      KST::matrixList.removeTag(koi->tag().tagString());
      KST::matrixList.lock().unlock();  
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
      KST::matrixList.lock().writeLock();
      KST::matrixList.removeTag(koi->tag().tagString());
      KST::matrixList.lock().unlock();  
      doUpdates();
    } else if (qi->rtti() == RTTI_OBJ_A_MATRIX) {
      KST::matrixList.lock().writeLock();
      KST::matrixList.removeTag(koi->tag().tagString());
      KST::matrixList.lock().unlock();  
      doUpdates();
    }

    update();
  } else {
    KstBaseCurvePtr bc;

    bc = kst_cast<KstBaseCurve>(koi->dataObject());
    if (bc || QMessageBox::warning(this, i18n("Kst"), i18n("There are other objects in memory that depend on %1.  Do you wish to delete them too?").arg(koi->tag().tag()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      if (qi->rtti() == RTTI_OBJ_OBJECT) {
        koi->dataObject()->deleteDependents();
        doc->removeDataObject(koi->tag().tagString());
      } else if (qi->rtti() == RTTI_OBJ_DATA_VECTOR) {
        KstRVectorPtr x;

        x = kst_cast<KstRVector>(*KST::vectorList.findTag(koi->tag().tagString()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::vectorList.lock().writeLock();
          KST::vectorList.removeTag(koi->tag().tagString());
          KST::vectorList.lock().unlock();
          doUpdates();
        } else {
          QMessageBox::warning(this, i18n("Kst"), i18n("Unknown error deleting data vector."));
        }
      } else if (qi->rtti() == RTTI_OBJ_STATIC_VECTOR) {
        KstSVectorPtr x;

        x = kst_cast<KstSVector>(*KST::vectorList.findTag(koi->tag().tagString()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::vectorList.lock().writeLock();
          KST::vectorList.removeTag(koi->tag().tagString());
          KST::vectorList.lock().unlock();
          doUpdates();
        } else {
          QMessageBox::warning(this, i18n("Kst"), i18n("Unknown error deleting static vector."));
        }
      } else if (qi->rtti() == RTTI_OBJ_A_VECTOR) {
        KstAVectorPtr x;

        x = kst_cast<KstAVector>(*KST::vectorList.findTag(koi->tag().tagString()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::vectorList.lock().writeLock();
          KST::vectorList.removeTag(koi->tag().tagString());
          KST::vectorList.lock().unlock();
          doUpdates();
        } else {
          QMessageBox::warning(this, i18n("Kst"), i18n("Unknown error deleting static vector."));
        }
      } else if (qi->rtti() == RTTI_OBJ_DATA_MATRIX) {
        KstRMatrixPtr x;
        
        x = kst_cast<KstRMatrix>(*KST::matrixList.findTag(koi->tag().tagString()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::matrixList.lock().writeLock();
          KST::matrixList.removeTag(koi->tag().tagString());
          KST::matrixList.lock().unlock();
          doUpdates();
        } else {
          QMessageBox::warning(this, i18n("Kst"), i18n("Unknown error deleting data matrix."));
        }
      } else if (qi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
        KstSMatrixPtr x;
        
        x = kst_cast<KstSMatrix>(*KST::matrixList.findTag(koi->tag().tagString()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::matrixList.lock().writeLock();
          KST::matrixList.removeTag(koi->tag().tagString());
          KST::matrixList.lock().unlock();
          doUpdates();
        } else {
          QMessageBox::warning(this, i18n("Kst"),i18n("Unknown error deleting static matrix."));
        }
      } else if (qi->rtti() == RTTI_OBJ_A_MATRIX) {
        KstAMatrixPtr x;

        x = kst_cast<KstAMatrix>(*KST::matrixList.findTag(koi->tag().tagString()));
        if (x) {
          x->deleteDependents();
          x = 0L;
          KST::matrixList.lock().writeLock();
          KST::matrixList.removeTag(koi->tag().tagString());
          KST::matrixList.lock().unlock();
          doUpdates();
        } else {
          QMessageBox::warning(this, i18n("Kst"), i18n("Unknown error deleting static matrix."));
        }
      }
      KstApp::inst()->paintAll(KstPainter::P_PLOT);
      update();
    } else {
      QMessageBox::warning(this, i18n("Kst"),i18n("Cannot delete objects with dependencies."));
    }
  }
*/
}


// Menu IDs:
// 100->499 reserved for plots
// 500->999 reserved for filters

void KstDataManager::contextMenu(QTreeWidgetItem *i, const QPoint& p, int col) {
  Q_UNUSED(col)

  if (!i) {
    return;
  }
/* xxx
  KstObjectItem *koi = static_cast<KstObjectItem*>(i);
  KstBaseCurvePtr c;
  KstImagePtr img;
  KPopupMenu *m = new KPopupMenu(this);
  int id;

  m->insertTitle(koi->text(0));

  if (koi->rtti() != RTTI_OBJ_VECTOR && koi->rtti() != RTTI_OBJ_MATRIX) {
    id = m->insertItem(i18n("&Edit..."), this, SLOT(edit_I()));
  }

  if (koi->dataObject()) {
    const KstCurveHintList* hints = koi->dataObject()->curveHints();

    if (!hints->isEmpty()) {
      KstCurveHintList::const_iterator i;
      KPopupMenu *hintMenu = new KPopupMenu(this);
      int cnt = 0;

      for (i = hints->begin(); i != hints->end(); ++i) {
        hintMenu->insertItem((*i)->curveName(), koi, SLOT(activateHint(int)), 0, cnt);
        cnt++;
      }
      id = m->insertItem(i18n("&Quick Curve"), hintMenu);
    }
  }

  if (koi->rtti() == RTTI_OBJ_DATA_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
    id = m->insertItem(i18n("Make Spect&rum..."), koi, SLOT(makePSD()));
    id = m->insertItem(i18n("Make &Spectrogram..."), koi, SLOT(makeCSD()));
    id = m->insertItem(i18n("Make &Histogram..."), koi, SLOT(makeHistogram()));
    id = m->insertItem(i18n("&Reload"), koi, SLOT(reload()));
    id = m->insertItem(i18n("Meta &Data"), koi, SLOT(showMetadata()));
    id = m->insertItem(i18n("&View Values..."), koi, SLOT(viewVectorValues()));
  } else if (koi->rtti() == RTTI_OBJ_VECTOR) {
    id = m->insertItem(i18n("&Make Curve..."), koi, SLOT(makeCurve()));
    id = m->insertItem(i18n("Make Spect&rum..."), koi, SLOT(makePSD()));
    id = m->insertItem(i18n("Make &Spectrogram..."), koi, SLOT(makeCSD()));
    id = m->insertItem(i18n("Make &Histogram..."), koi, SLOT(makeHistogram()));
    id = m->insertItem(i18n("&View Values..."), koi, SLOT(viewVectorValues()));
  } else if (koi->rtti() == RTTI_OBJ_DATA_MATRIX) {
    id = m->insertItem(i18n("Make &Image..."), koi, SLOT(makeImage()));  
    id = m->insertItem(i18n("&Reload"), koi, SLOT(reload()));
    id = m->insertItem(i18n("Meta &Data"), koi, SLOT(showMetadata()));
    id = m->insertItem(i18n("&View Values..."), koi, SLOT(viewMatrixValues()));
  } else if (koi->rtti() == RTTI_OBJ_MATRIX || koi->rtti() == RTTI_OBJ_STATIC_MATRIX) {
    id = m->insertItem(i18n("Make &Image..."), koi, SLOT(makeImage()));
    id = m->insertItem(i18n("&View Values..."), koi, SLOT(viewMatrixValues()));
  } else if ((c = kst_cast<KstBaseCurve>(koi->dataObject()))) {
    KPopupMenu *addMenu = new KPopupMenu(this);
    KPopupMenu *removeMenu = new KPopupMenu(this);
    PlotMap.clear();
    id = 300;
    bool haveAdd = false, haveRemove = false;

    KstApp *app = KstApp::inst();
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
  
    windows = app->subWindowList(QMdiArea::CreationOrder);
  
    for (i = windows.constBegin(); i != windows.constEnd(); ++i)
      KstViewWindow *viewWindow = dynamic_cast<KstViewWindow*>(*i);

      if (viewWindow) {
        Kst2DPlotList plots = viewWindow->view()->findChildrenType<Kst2DPlot>();
        Kst2DPlotList::iterator i;

        for (i = plots.begin(); i != plots.end(); ++i) {
          Kst2DPlotPtr plot = *i;

          if (!plot->Curves.contains(c)) {
            addMenu->insertItem(i18n("%1 - %2").arg(v->caption()).arg(plot->tag().tag()), koi, SLOT(addToPlot(int)), 0, id);
            haveAdd = true;
          } else {
            removeMenu->insertItem(i18n("%1 - %2").arg(v->caption()).arg(plot->tag().tag()), koi, SLOT(removeFromPlot(int)), 0, id);
            haveRemove = true;
          }
          PlotMap[id++] = plot;
        }
      }
    }

    id = m->insertItem(i18n("&Add to Plot"), addMenu);
    m->setItemEnabled(id, haveAdd);
    id = m->insertItem(i18n("&Remove From Plot"), removeMenu);
    m->setItemEnabled(id, haveRemove);
  } 

  if (koi->rtti() != RTTI_OBJ_VECTOR && koi->rtti() != RTTI_OBJ_MATRIX) {
    //
    // no slave vectors or matrices get this
    //

    id = m->insertItem(i18n("&Delete"), this, SLOT(delete_I()));
  }

  m->popup(p);
*/
}


void KstDataManager::doUpdates() {
//  doc->forceUpdate();
//  doc->setModified();
  emit docChanged();
}


void KstDataManager::currentChanged(QTreeWidgetItem *i) {
  if (i && !DataView->selectedItems().isEmpty()) {
    KstObjectItem *koi = static_cast<KstObjectItem*>(i);

    koi->updateButtons();
  } else {
    Edit->setEnabled(false);
    Delete->setEnabled(false);
  }
}


void KstDataManager::selectionChanged() {
  if (!DataView->selectedItems().isEmpty()) {
    KstObjectItem *koi = static_cast<KstObjectItem*>(DataView->selectedItems().first());

    koi->updateButtons();
  } else {
    Edit->setEnabled(false);
    Delete->setEnabled(false);
  }
}

#include "kstdatamanager.moc"
