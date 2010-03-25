/***************************************************************************
                       kstvvdialog.cpp  -  Dialog for KstVectorView objects.
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by The University of British Columbia
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include "kstvvdialog.h"
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "kst2dplot.h"
#include "kstchoosecolordialog.h"
#include "kstdataobjectcollection.h"
#include "kstsettings.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "vectorselector.h"
#include "scalarselector.h"

const QString& KstVvDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

QPointer<KstVvDialog> KstVvDialog::_inst;

KstVvDialog *KstVvDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstVvDialog(KstApp::inst());
  }
  return _inst;
}


KstVvDialog::KstVvDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstDataDialog(parent) {
  _w = new Ui::VectorViewDialogWidget();
  _w->setupUi(this);

  setMultiple(true);

  connect(_w->_xVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_yVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_FlagVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_xMinScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_xMaxScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_yMinScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_yMaxScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));

  _w->_FlagVector->provideNoneVector(true);
  _w->_FlagVector->allowNewVectors(true);

  connect(_w->_xMinCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_w->_xMaxCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_w->_yMinCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_w->_yMaxCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));

  _w->_xMinScalar->allowDirectEntry(true);
  _w->_xMaxScalar->allowDirectEntry(true);
  _w->_yMinScalar->allowDirectEntry(true);
  _w->_yMaxScalar->allowDirectEntry(true);

  connect(_w->_realtimeButton, SIGNAL(clicked()), this, SLOT(realtimeClicked()));
  connect(_w->_currentButton, SIGNAL(clicked()), this, SLOT(currentClicked()));

  QColor qc = _w->_curveAppearance->color();
  _w->_curveAppearance->setValue(false, false, true, qc, 0, 0, 0, 1, 0);

  // for multiple edit mode
  connect(_w->_xMinCheckbox, SIGNAL(clicked()), this, SLOT(xMinCheckboxClicked()));
  connect(_w->_xMaxCheckbox, SIGNAL(clicked()), this, SLOT(xMaxCheckboxClicked()));
  connect(_w->_yMinCheckbox, SIGNAL(clicked()), this, SLOT(yMinCheckboxClicked()));
  connect(_w->_yMaxCheckbox, SIGNAL(clicked()), this, SLOT(yMaxCheckboxClicked()));

  // for apply button
  connect(_w->_xVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_interp, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinCheckbox, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxCheckbox, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinCheckbox, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxCheckbox, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_plotAxes, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_plotList, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_realtimeButton, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_currentButton, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_FlagVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_FlagVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_curveAppearance, SIGNAL(changed()), this, SLOT(wasModifiedApply()));
}


KstVvDialog::~KstVvDialog() {
}


void KstVvDialog::updateWindow() {
  _w->_curvePlacement->update();
}


void KstVvDialog::updatePlotList() {
    QStringList::const_iterator i;
    QStringList plots;
    QString old;

    if (_w->_plotList->count()) {
      old = _w->_plotList->currentText();
    }

    plots = KstData::self()->plotList();

    _w->_plotList->clear();
    for (i = plots.begin(); i != plots.end(); ++i) {
      _w->_plotList->insertItem(0, *i);
    }

    if (!old.isNull() && plots.contains(old)) {
// xxx      _w->_plotList->setCurrentText(old);
    }
}


void KstVvDialog::fillFieldsForEdit() {
  KstVectorViewPtr vp;

  vp = kst_cast<KstVectorView>(_dp);
  if (vp) {
    vp->readLock();
  
    _tagName->setText(vp->tagName());
  
    _w->_xVector->setSelection(vp->in_xVTag());
    _w->_yVector->setSelection(vp->in_yVTag());
  
    if (vp->hasFlag()) {
      _w->_FlagVector->setSelection(vp->FlagTag()); 
    }
  
    _w->_xMinCheckbox->setChecked(vp->useXmin());
    _w->_xMaxCheckbox->setChecked(vp->useXmax());
    _w->_yMinCheckbox->setChecked(vp->useYmin());
    _w->_yMaxCheckbox->setChecked(vp->useYmax());
  
    KstScalarPtr sc;
    QString str;
  
    sc = vp->xMinScalar();
    if (!sc) {
      _w->_xMinScalar->setSelection("0");
    } else if (*KST::scalarList.findTag(sc->tag().displayString())) {
      _w->_xMinScalar->setSelection(sc->tag().displayString());
    } else {
      // our scalar has been removed from the global list...
      // just put its current value into the scalar selector.
      // warning: after edit it won't be updated anymore!
      // the motivation for putting this in is to not lose scale information when a plot is deleted.
      _w->_xMinScalar->setSelection(QString::number(sc->value()));
    }
  
    sc = vp->xMaxScalar();
    if (!sc) {
      _w->_xMaxScalar->setSelection("0");
    } else if (*KST::scalarList.findTag(sc->tag().displayString())) {
      _w->_xMaxScalar->setSelection(sc->tag().displayString());
    } else {
      _w->_xMaxScalar->setSelection(QString::number(sc->value()));
    }
  
    sc = vp->yMinScalar();
    if (!sc) {
      _w->_yMinScalar->setSelection("0");
    } else if (*KST::scalarList.findTag(sc->tag().displayString())) {
      _w->_yMinScalar->setSelection(sc->tag().displayString());
    } else {
      _w->_yMinScalar->setSelection(QString::number(sc->value()));
    }
  
    sc = vp->yMaxScalar();
    if (!sc) {
      _w->_yMaxScalar->setSelection("0");
    } else if (*KST::scalarList.findTag(sc->tag().displayString())) {
      _w->_yMaxScalar->setSelection(sc->tag().displayString());
    } else {
      _w->_yMaxScalar->setSelection(QString::number(sc->value()));
    }
  
    vp->unlock();
    updateButtons();
  
    // can't edit curve props from here....
    _w->_curveAppearance->hide();
    _w->_curvePlacement->hide();
    _legendText->hide();
    _legendLabel->hide();
  
    adjustSize();
    resize(minimumSizeHint());
    setFixedHeight(height());
  }
}


void KstVvDialog::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);
  _legendText->show();
  _legendLabel->show();

  // set the curve placement window
  _w->_curvePlacement->update();

  // for some reason the lower widget needs to be shown first to prevent overlapping?
  _w->_curveAppearance->hide();
  _w->_curvePlacement->show();
  _w->_curveAppearance->show();
  _w->_curveAppearance->reset();

  QColor qc = _w->_curveAppearance->color();
  _w->_curveAppearance->setValue(true, false, false, qc, 0, 0, 0, 0, 0);

  updateButtons();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstVvDialog::update() {
  disconnect(_w->_xVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xVector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yVector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xMinScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xMinScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xMinScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xMaxScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xMaxScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_xMaxScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yMinScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yMinScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yMinScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yMaxScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yMaxScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_yMaxScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_FlagVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_FlagVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  disconnect(_w->_plotList, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));

  _w->_curvePlacement->update();
  _w->_xVector->update();
  _w->_yVector->update();

  _w->_FlagVector->update();

  _w->_xMinScalar->update();
  _w->_xMaxScalar->update();
  _w->_yMinScalar->update();
  _w->_yMaxScalar->update();

  updatePlotList();

  connect(_w->_xVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector->_vector, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMaxScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxScalar, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxScalar, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMaxScalar->_scalar, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_FlagVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_FlagVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_plotList, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
}


bool KstVvDialog::newObject() {
  KstVectorViewPtr vv;
  KstVectorPtr vx;
  KstVectorPtr vy;
  KstVectorPtr flag;
  QString tag_name = _tagName->text();

  if (tag_name == defaultTag) {
    tag_name = KST::suggestVectorViewName(KstObjectTag::fromString(_w->_xVector->selectedVector()));
  }

  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (_w->_xVector->selectedVector().isEmpty()) {
    QMessageBox::warning(this, i18n("Kst"), i18n("New VectorView not made: define X vector first."));
    return false;
  }

  if (_w->_yVector->selectedVector().isEmpty()) {
    QMessageBox::warning(this, i18n("Kst"), i18n("New VectorView not made: define Y vector first."));
    return false;
  }

  KST::vectorList.lock().readLock();
  vx = *KST::vectorList.findTag(_w->_xVector->selectedVector());
  vy = *KST::vectorList.findTag(_w->_yVector->selectedVector());
  flag = *KST::vectorList.findTag(_w->_FlagVector->selectedVector());
  KST::vectorList.lock().unlock();
  if (vx && vy) {
    KstVCurvePtr vc;
    QColor color;
    QString legend_text;

    vx->readLock();
    vy->readLock();
    if (flag) {
      flag->readLock();
    }
    
    vv = new KstVectorView(tag_name, vx, vy, 
                          KstVectorView::InterpType(_w->_interp->currentIndex()),
                          _w->_xMinCheckbox->isChecked(),
                          _w->_xMinScalar->selectedScalarPtr(),
                          _w->_xMaxCheckbox->isChecked(),
                          _w->_xMaxScalar->selectedScalarPtr(),
                          _w->_yMinCheckbox->isChecked(),
                          _w->_yMinScalar->selectedScalarPtr(),
                          _w->_yMaxCheckbox->isChecked(),
                          _w->_yMaxScalar->selectedScalarPtr(),
                          flag );
    vx->unlock();
    vy->unlock();
    if (flag) {
      flag->unlock();
    }
  
    color = KstApp::inst()->chooseColorDlg()->getColorForCurve(vv->vX(), vv->vY());
    if (!color.isValid()) {
      color = _w->_curveAppearance->color();
    }
    vc = new KstVCurve(KST::suggestCurveName(vv->tag(), true), vv->vX(), vv->vY(), KstVectorPtr(), KstVectorPtr(), KstVectorPtr(), KstVectorPtr(), color);
  
    vc->setHasPoints(_w->_curveAppearance->showPoints());
    vc->setHasLines(_w->_curveAppearance->showLines());
    vc->setHasBars(_w->_curveAppearance->showBars());
    vc->setPointStyle(_w->_curveAppearance->pointType());
    vc->setLineWidth(_w->_curveAppearance->lineWidth());
    vc->setLineStyle(_w->_curveAppearance->lineStyle());
    vc->setBarStyle(_w->_curveAppearance->barStyle());
    vc->setPointDensity(_w->_curveAppearance->pointDensity());
  
    legend_text = _legendText->text();
    if (legend_text == defaultTag) {
      vc->setLegendText(QString(""));
    } else {
      vc->setLegendText(legend_text);
    }
  
    KstViewWindow *w;
/* xxx
    w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_w->_curvePlacement->_plotWindow->currentText()));
    if (!w) {
      QString n = KstApp::inst()->newWindow(KST::suggestWinName());

      w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
    }
*/
    if (w) {
      Kst2DPlotPtr plot;

      if (_w->_curvePlacement->existingPlot()) {
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(_w->_curvePlacement->plotName()));
        if (plot) {
          plot->addCurve(vc);
        }
      }
  
      if (_w->_curvePlacement->newPlot()) {
        QString name = w->createPlot(KST::suggestPlotName());

        if (_w->_curvePlacement->reGrid()) {
          w->view()->cleanup(_w->_curvePlacement->columns());
        }
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
        if (plot) {
          _w->_curvePlacement->update();
          _w->_curvePlacement->setCurrentPlot(plot->tagName());
          plot->addCurve(vc);
          plot->generateDefaultLabels();
        }
      }
    }
  
    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.append(vv);
    KST::dataObjectList.append(vc);
    KST::dataObjectList.lock().unlock();
  
    vv = 0L;
    vc = 0L;

// xxx    emit modified();
  }

  return true;
}


bool KstVvDialog::editSingleObject(KstVectorViewPtr vvPtr) {
  if (_xVectorDirty) {
    KST::vectorList.lock().readLock();
    vvPtr->setXVector(*KST::vectorList.findTag(_w->_xVector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  if (_yVectorDirty) {
    KST::vectorList.lock().readLock();
    vvPtr->setYVector(*KST::vectorList.findTag(_w->_yVector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  if (_interpTypeDirty) {
    if (_editMultipleMode) {
      vvPtr->setInterp(KstVectorView::InterpType(_w->_interp->currentIndex()-1));
    } else {
      vvPtr->setInterp(KstVectorView::InterpType(_w->_interp->currentIndex()));
    }
  }

  if (_xMinScalarDirty) {
    KstScalarPtr s;

    s = _w->_xMinScalar->selectedScalarPtr();
    if (s) {
      vvPtr->setXminScalar(s);
    }
  }

  if (_xMaxScalarDirty) {
    KstScalarPtr s;

    s = _w->_xMaxScalar->selectedScalarPtr();
    if (s) {
      vvPtr->setXmaxScalar(s);
    }
  }

  if (_yMinScalarDirty) {
    KstScalarPtr s;
    
    s = _w->_yMinScalar->selectedScalarPtr();
    if (s) {
      vvPtr->setYminScalar(s);
    }
  }

  if (_yMaxScalarDirty) {
    KstScalarPtr s;

    s = _w->_yMaxScalar->selectedScalarPtr();
    if (s) {
      vvPtr->setYmaxScalar(s);
    }
  }

  if (_FlagVectorDirty) {
    KST::vectorList.lock().readLock();
    vvPtr->setFlagVector(*KST::vectorList.findTag(_w->_FlagVector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  if (_useXminDirty) {
    vvPtr->setUseXmin(_w->_xMinCheckbox->isChecked());
  }

  if (_useXmaxDirty) {
    vvPtr->setUseXmax(_w->_xMaxCheckbox->isChecked());
  }

  if (_useYminDirty) {
    vvPtr->setUseYmin(_w->_yMinCheckbox->isChecked());
  }

  if (_useYmaxDirty) {
    vvPtr->setUseYmax(_w->_yMaxCheckbox->isChecked());
  }

  vvPtr->writeLock();
  vvPtr->setRecursed(false);
  if (vvPtr->recursion()) {
    vvPtr->setRecursed(true);
    vvPtr->unlock();
    QMessageBox::critical(this, i18n("Kst"), i18n("There is a recursion resulting from the vector view you entered."));

    return false;
  }

  vvPtr->setDirty();
  vvPtr->unlock();

  return true;
}


bool KstVvDialog::editObject() {
  KstVectorViewList vvList;

  _FlagVectorDirty = _w->_FlagVector->_vector->currentIndex() != 0;
  _xVectorDirty = _w->_xVector->_vector->currentIndex() != 0;
  _yVectorDirty = _w->_yVector->_vector->currentIndex() != 0;
  _useXminDirty = _w->_xMinCheckbox->checkState() != Qt::PartiallyChecked;
  _useXmaxDirty = _w->_xMaxCheckbox->checkState() != Qt::PartiallyChecked;
  _useYminDirty = _w->_yMinCheckbox->checkState() != Qt::PartiallyChecked;
  _useYmaxDirty = _w->_yMaxCheckbox->checkState() != Qt::PartiallyChecked;
  _xMinScalarDirty = _w->_xMinScalar->_scalar->currentIndex() != 0;
  _xMaxScalarDirty = _w->_xMaxScalar->_scalar->currentIndex() != 0;
  _yMinScalarDirty = _w->_yMinScalar->_scalar->currentIndex() != 0;
  _yMaxScalarDirty = _w->_yMaxScalar->_scalar->currentIndex() != 0;
  _interpTypeDirty = _w->_interp->currentIndex() != 0;
// xxx  vvList = kstObjectSubList<KstDataObject,KstVectorView>(KST::dataObjectList);

  if (_editMultipleMode) {
    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstVectorViewList::iterator vvIter;
        KstVectorViewPtr vvPtr;

        vvIter = vvList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        if (vvIter == vvList.end()) {
          return false;
        }

        vvPtr = *vvIter;

        if (!editSingleObject(vvPtr)) {
          return false;
        }
        didEdit = true;
      }
    }
    if (!didEdit) {
      QMessageBox::warning(this, i18n("Kst"), i18n("Select one or more objects to edit."));
      return false;
    }
  } else {
    KstVectorViewPtr vp;
    QString tag_name = _tagName->text();

    vp = kst_cast<KstVectorView>(_dp);
    if (!vp || (tag_name != vp->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();
      return false;
    }

    vp->writeLock();
    vp->setTagName(tag_name);
    vp->unlock();

    _xVectorDirty = true;
    _yVectorDirty = true;
    _interpTypeDirty = true;
    _useXminDirty = true;
    _xMinScalarDirty = true;
    _useXmaxDirty = true;
    _xMaxScalarDirty = true;
    _useYminDirty = true;
    _yMinScalarDirty = true;
    _useYmaxDirty = true;
    _yMaxScalarDirty = true;
    _FlagVectorDirty = true;

    if (!editSingleObject(vp)) {
      return false;
    }
  }

// xxx  emit modified();

  return true;
}


void KstVvDialog::updateButtons() {
  _w->_xMinScalar->setEnabled(_w->_xMinCheckbox->checkState() != Qt::Unchecked);
  _w->_xMaxScalar->setEnabled(_w->_xMaxCheckbox->checkState() != Qt::Unchecked);
  _w->_yMinScalar->setEnabled(_w->_yMinCheckbox->checkState() != Qt::Unchecked);
  _w->_yMaxScalar->setEnabled(_w->_yMaxCheckbox->checkState() != Qt::Unchecked);
}


void KstVvDialog::realtimeClicked() {
  Kst2DPlotPtr plot;

  plot = Kst2DPlot::findPlotByName(_w->_plotList->currentText()); 

  //
  // the following assumes that the combo box entries are as follows:
  //  XY Axes, X Axis, Y Axis
  //

  if (_w->_plotAxes->currentIndex() == 0 || _w->_plotAxes->currentIndex() == 1) {
    _w->_xMinCheckbox->setChecked(true);
    _w->_xMinScalar->setSelection((plot->scalars())["xmin"]->tag().displayString());

    _w->_xMaxCheckbox->setChecked(true);
    _w->_xMaxScalar->setSelection((plot->scalars())["xmax"]->tag().displayString());
  }

  if (_w->_plotAxes->currentIndex() == 0 || _w->_plotAxes->currentIndex() == 2) {
    _w->_yMinCheckbox->setChecked(true);
    _w->_yMinScalar->setSelection((plot->scalars())["ymin"]->tag().displayString());

    _w->_yMaxCheckbox->setChecked(true);
    _w->_yMaxScalar->setSelection((plot->scalars())["ymax"]->tag().displayString());
  }

  updateButtons();
}


void KstVvDialog::currentClicked() {
    Kst2DPlotPtr plot = Kst2DPlot::findPlotByName(_w->_plotList->currentText());
    KstScalarPtr sp;
    double v;

    //
    // the following assumes that the combo box entries are as follows:
    //  XY Axes, X Axis, Y Axis
    //

    if (_w->_plotAxes->currentIndex() == 0 || _w->_plotAxes->currentIndex() == 1) {
      _w->_xMinCheckbox->setChecked(true);
      v = (plot->scalars())["xmin"]->value();
      _w->_xMinScalar->setSelection(QString::number(v));

      _w->_xMaxCheckbox->setChecked(true);
      v = (plot->scalars())["xmax"]->value();
      _w->_xMaxScalar->setSelection(QString::number(v));
    }

    if (_w->_plotAxes->currentIndex() == 0 || _w->_plotAxes->currentIndex() == 2) {
      _w->_yMinCheckbox->setChecked(true);
      v = (plot->scalars())["ymin"]->value();
      _w->_yMinScalar->setSelection(QString::number(v));

      _w->_yMaxCheckbox->setChecked(true);
      v = (plot->scalars())["ymax"]->value();
      _w->_yMaxScalar->setSelection(QString::number(v));
    }

    updateButtons();
}


void KstVvDialog::cleanup() {
  if (_editMultipleMode) {
    _w->_xMinCheckbox->setTristate(false);
    _w->_xMaxCheckbox->setTristate(false);
    _w->_yMinCheckbox->setTristate(false);
    _w->_yMaxCheckbox->setTristate(false);
    _w->_interp->removeItem(0);
  }
}


void KstVvDialog::populateEditMultiple() {
  KstVectorViewList vvlist;

// xxx  vvlist = kstObjectSubList<KstDataObject,KstVectorView>(KST::dataObjectList);
// xxx  _editMultipleWidget->_objectList->insertStringList(vvlist.tagNames());

  _w->_xVector->_vector->insertItem(0, "");
  _w->_xVector->_vector->setCurrentIndex(0);
  _w->_yVector->_vector->insertItem(0, "");
  _w->_yVector->_vector->setCurrentIndex(0);
  _w->_interp->insertItem(0, "");
  _w->_interp->setCurrentIndex(0);
  _w->_xMinCheckbox->setChecked(Qt::PartiallyChecked);
  _w->_xMinScalar->_scalar->insertItem(0, "");
  _w->_xMinScalar->_scalar->setCurrentIndex(0);
  _w->_xMaxCheckbox->setChecked(Qt::PartiallyChecked);
  _w->_xMaxScalar->_scalar->insertItem(0, "");
  _w->_xMaxScalar->_scalar->setCurrentIndex(0);
  _w->_yMinCheckbox->setChecked(Qt::PartiallyChecked);
  _w->_yMinScalar->_scalar->insertItem(0, "");
  _w->_yMinScalar->_scalar->setCurrentIndex(0);
  _w->_yMaxCheckbox->setChecked(Qt::PartiallyChecked);
  _w->_yMaxScalar->_scalar->insertItem(0, "");
  _w->_yMaxScalar->_scalar->setCurrentIndex(0);
  _w->_FlagVector->_vector->insertItem(0, "");
  _w->_FlagVector->_vector->setCurrentIndex(0);

  _tagName->setText("");
  _tagName->setEnabled(false);

  _xVectorDirty = false;
  _yVectorDirty = false;
  _interpTypeDirty = false;
  _useXminDirty = false;
  _useXmaxDirty = false;
  _useYminDirty = false;
  _useYmaxDirty = false;
  _xMinScalarDirty = false;
  _xMaxScalarDirty = false;
  _yMinScalarDirty = false;
  _yMaxScalarDirty = false;
  _useFlagVectorDirty = false;
  _FlagVectorDirty = false;

  updateButtons();
}


void KstVvDialog::setXVector(const QString& name) {
  _w->_xVector->setSelection(name);
}


void KstVvDialog::setYVector(const QString& name) {
  _w->_xVector->setSelection(name);
}


void KstVvDialog::xMinCheckboxClicked() {
  _w->_xMinCheckbox->setTristate(FALSE);
}


void KstVvDialog::xMaxCheckboxClicked() {
  _w->_xMaxCheckbox->setTristate(FALSE);
}


void KstVvDialog::yMinCheckboxClicked() {
  _w->_yMinCheckbox->setTristate(FALSE);
}


void KstVvDialog::yMaxCheckboxClicked() {
  _w->_yMaxCheckbox->setTristate(FALSE);
}

#include "kstvvdialog.moc"
