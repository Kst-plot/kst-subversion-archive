/***************************************************************************
                      kstcurvedialog_i.cpp  -  Part of KST
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

#include <qcheckbox.h>
#include <qlistbox.h>
#include <QMessageBox>
#include <qspinbox.h>
#include <qvbox.h>
#include <qradiobutton.h>

#include <kcombobox.h>
#include <kcolorbutton.h>
#include "ksdebug.h"

#include "curveappearancewidget.h"
#include "curvedialogwidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "kst2dplot.h"
#include "kstchoosecolordialog_i.h"
#include "kstcurvedialog_i.h"
#include "kstdataobjectcollection.h"
#include "kstviewwindow.h"
#include "kstuinames.h"
#include "scalarselector.h"
#include "vectorselector.h"
#include "kstplotlabel.h"

const QString& KstCurveDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

QPointer<KstCurveDialogI> KstCurveDialog::_inst;

KstCurveDialog *KstCurveDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstCurveDialogI(KstApp::inst());
  }
  return _inst;
}


KstCurveDialog::KstCurveDialog(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new CurveDialogWidget(_contents);
  setMultiple(true);
  connect(_w->_xVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_yVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_xError, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_yError, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_xMinusError, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_yMinusError, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_checkBoxXMinusSameAsPlus, SIGNAL(clicked()), this, SLOT(toggledXErrorSame()));
  connect(_w->_checkBoxYMinusSameAsPlus, SIGNAL(clicked()), this, SLOT(toggledYErrorSame()));

  // multiple edit mode
  connect(_w->_checkBoxXMinusSameAsPlus, SIGNAL(clicked()), this, SLOT(setCheckBoxXMinusSameAsPlusDirty()));
  connect(_w->_checkBoxYMinusSameAsPlus, SIGNAL(clicked()), this, SLOT(setCheckBoxYMinusSameAsPlusDirty()));
  connect(_w->_curveAppearance->_color, SIGNAL(changed(const QColor&)), this, SLOT(setColorDirty()));
  connect(_w->_curveAppearance->_showPoints, SIGNAL(clicked()), this, SLOT(setShowPointsDirty()));
  connect(_w->_curveAppearance->_showLines, SIGNAL(clicked()), this, SLOT(setShowLinesDirty()));
  connect(_w->_curveAppearance->_showBars, SIGNAL(clicked()), this, SLOT(setShowBarsDirty()));
  connect(_w->_checkBoxIgnoreAutoscale, SIGNAL(clicked()), this, SLOT(setCheckBoxIgnoreAutoscaleDirty()));
  connect(_w->_checkBoxYVectorOffset, SIGNAL(clicked()), this, SLOT(setCheckBoxYVectorOffsetDirty()));

  // for apply button
  connect(_w->_xVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xError, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinusError, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yError, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinusError, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xError, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinusError, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yError, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinusError, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVector->_vector, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yVector->_vector, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xError->_vector, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xMinusError->_vector, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yError->_vector, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_yMinusError->_vector, SIGNAL(completion(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_interp, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_checkBoxXMinusSameAsPlus, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_checkBoxYMinusSameAsPlus, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_curveAppearance, SIGNAL(changed()), this, SLOT(wasModifiedApply()));
  connect(_w->_checkBoxIgnoreAutoscale, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_checkBoxYVectorOffset, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_scalarSelectorYVectorOffset, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));

  _w->_xError->provideNoneVector(true);
  _w->_yError->provideNoneVector(true);
  _w->_xMinusError->provideNoneVector(true);
  _w->_yMinusError->provideNoneVector(true);

  _w->_checkBoxXMinusSameAsPlus->setChecked(true);
  _w->_checkBoxYMinusSameAsPlus->setChecked(true);

  toggledXErrorSame();
  toggledYErrorSame();
}


KstCurveDialog::~KstCurveDialog() {
}


void KstCurveDialog::updateWindow() {
  _w->_curvePlacement->update();
}


void KstCurveDialog::toggledXErrorSame(bool on) {
  _w->textLabelXMinus->setEnabled(!on);
  _w->_xMinusError->setEnabled(!on);
}


void KstCurveDialog::toggledYErrorSame(bool on) {
  _w->textLabelYMinus->setEnabled(!on);
  _w->_yMinusError->setEnabled(!on);
}


void KstCurveDialog::toggledXErrorSame() {
  toggledXErrorSame(_w->_checkBoxXMinusSameAsPlus->isChecked());
}


void KstCurveDialog::toggledYErrorSame() {
  toggledYErrorSame(_w->_checkBoxYMinusSameAsPlus->isChecked());
}


void KstCurveDialog::fillFieldsForEdit() {
  KstVCurvePtr cp = kst_cast<KstVCurve>(_dp);
  if (!cp) {
    return; // shouldn't be needed
  }

  cp->readLock();

  _tagName->setText(cp->tagName());
  if (cp->legendText() == "") {
    _legendText->setText(defaultTag);
  } else {
    _legendText->setText(cp->legendText());
  }

  _w->_xVector->setSelection(cp->xVTag().displayString());
  _w->_yVector->setSelection(cp->yVTag().displayString());
  _w->_xError->setSelection(cp->xETag().displayString());
  _w->_yError->setSelection(cp->yETag().displayString());
  _w->_xMinusError->setSelection(cp->xEMinusTag().displayString());
  _w->_yMinusError->setSelection(cp->yEMinusTag().displayString());

  if (cp->xEMinusTag() == cp->xETag()) {
    _w->_checkBoxXMinusSameAsPlus->setChecked(true);
  } else {
    _w->_checkBoxXMinusSameAsPlus->setChecked(false);
  }
  if (cp->yEMinusTag() == cp->yETag()) {
    _w->_checkBoxYMinusSameAsPlus->setChecked(true);
  }  else {
    _w->_checkBoxYMinusSameAsPlus->setChecked(false);
  }
  toggledXErrorSame();
  toggledYErrorSame();

  _w->_curveAppearance->setValue(cp->hasLines(), cp->hasPoints(),
      cp->hasBars(), cp->color(),
      cp->pointStyle(), cp->lineWidth(),
      cp->lineStyle(), cp->barStyle(), cp->pointDensity());

  _w->_checkBoxIgnoreAutoscale->setChecked(cp->ignoreAutoScale());
  _w->_checkBoxYVectorOffset->setChecked(cp->yVectorOffset());
  if (cp->yVectorOffset()) {
    _w->_scalarSelectorYVectorOffset->setSelection(cp->yVectorOffsetTag().displayString());
  } else {
    //
    // if the user doesn't have the y-offset enabled then we set the scalar giving the mean
    //  of the y-vector as a reasonable default...
    //
    KstVectorPtr yVector = *cp->inputVectors().find("Y");
    if (yVector) {
      KstScalarPtr meanScalar;

      meanScalar = yVector->scalars().find("mean");
      if (meanScalar) {
        _w->_scalarSelectorYVectorOffset->setSelection(meanScalar->tag().displayString());
      }
    }
  }

  cp->unlock();

  _w->_curvePlacement->hide();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
  _w->_interp->setCurrentItem(cp->interp());
}


void KstCurveDialog::fillFieldsForNew() {
  KstVCurveList curves = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);
  _w->_curvePlacement->update();

  // set the X Axis Vector to the X axis vector of 
  // the last curve on the global curve list...
  if (curves.count() >0) {
    _w->_xVector->setSelection(curves.last()->xVTag().displayString());
  }

  // for some reason the lower widget needs to be shown first to prevent overlapping?
  _w->_curveAppearance->hide();
  _w->_curvePlacement->show();
  _w->_curveAppearance->show();
  _w->_curveAppearance->reset();
  _w->_interp->setCurrentItem(0);
  _w->_checkBoxIgnoreAutoscale->setChecked(false);
  _w->_checkBoxYVectorOffset->setChecked(false);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstCurveDialog::update() {
  _w->_curvePlacement->update();
  _w->_xVector->update();
  _w->_yVector->update();
  _w->_xError->update();
  _w->_yError->update();
  _w->_xMinusError->update();
  _w->_yMinusError->update();
  _w->_scalarSelectorYVectorOffset->update();
}


bool KstCurveDialog::newObject() {
  if (_w->_xVector->selectedVector().isEmpty() || _w->_yVector->selectedVector().isEmpty()) {
    QMessageBox::warning(this, i18n("Kst"), i18n("New curve not made: define vectors first."));
    return false;
  }

  KstVectorPtr EXMinus, EYMinus;

  // find VX and VY
  KST::vectorList.lock().readLock();
  KstVectorPtr VX = *KST::vectorList.findTag(_w->_xVector->selectedVector());
  KstVectorPtr VY = *KST::vectorList.findTag(_w->_yVector->selectedVector());
  KstVectorPtr EX = *KST::vectorList.findTag(_w->_xError->selectedVector());
  KstVectorPtr EY = *KST::vectorList.findTag(_w->_yError->selectedVector());
  if (_w->_checkBoxXMinusSameAsPlus->isChecked()) {
    EXMinus = EX;
  } else {
    EXMinus = *KST::vectorList.findTag(_w->_xMinusError->selectedVector());
  }
  if (_w->_checkBoxYMinusSameAsPlus->isChecked()) {
    EYMinus = EY;
  } else {
    EYMinus = *KST::vectorList.findTag(_w->_yMinusError->selectedVector());
  }
  KST::vectorList.lock().unlock();
  if (!VX || !VY) {
    kstdFatal() << "Bug in kst: the XVector field refers to "
                << "a non existent vector...." << endl;
  }

  // readlock the vectors, because when we update the curve, they get read
  VX->readLock();
  VY->readLock();

  QString tag_name = _tagName->text();
  if (tag_name == defaultTag) {
    tag_name = KST::suggestCurveName(VY->tag());
  }

  // verify that the curve name is unique
  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    VY->unlock();
    VX->unlock();
    return false;
  }

  if (EX) {
    EX->readLock();
  }
  if (EY) {
    EY->readLock();
  }
  if (EXMinus) {
    EXMinus->readLock();
  }
  if (EYMinus) {
    EYMinus->readLock();
  }

  QColor color = KstApp::inst()->chooseColorDlg()->getColorForCurve(VX, VY);
  if (!color.isValid()) {
    color = _w->_curveAppearance->color();
  }
  KstVCurvePtr curve = new KstVCurve(tag_name, VX, VY, EX, EY, EXMinus, EYMinus, color);

  if (EX) {
    EX->unlock();
  }
  if (EY) {
    EY->unlock();
  }
  if (EXMinus) {
    EXMinus->unlock();
  }
  if (EYMinus) {
    EYMinus->unlock();
  }

  VY->unlock();
  VX->unlock();

  QString legend_text = _legendText->text();
  if (legend_text == defaultTag) {
    curve->setLegendText(QString(""));
  } else {
    curve->setLegendText(legend_text);
  }

  curve->setHasPoints(_w->_curveAppearance->showPoints());
  curve->setHasLines(_w->_curveAppearance->showLines());
  curve->setHasBars(_w->_curveAppearance->showBars());
  curve->setLineWidth(_w->_curveAppearance->lineWidth());
  curve->setLineStyle(_w->_curveAppearance->lineStyle());
  curve->setPointStyle(_w->_curveAppearance->pointType());
  curve->setBarStyle(_w->_curveAppearance->barStyle());
  curve->setPointDensity(_w->_curveAppearance->pointDensity());
  curve->setIgnoreAutoScale(_w->_checkBoxIgnoreAutoscale->isChecked());
  curve->setYVectorOffset(_w->_checkBoxYVectorOffset->isChecked());

  curve->setInterp(KstVCurve::InterpType(_w->_interp->currentItem()));

  if (_w->_curvePlacement->existingPlot() || _w->_curvePlacement->newPlot()) {
    KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_w->_curvePlacement->_plotWindow->currentText()));
    if (!w) {
      QString n = KstApp::inst()->newWindow(KST::suggestWinName());
      w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
    }
    if (w) {
      Kst2DPlotPtr plot;
      if (_w->_curvePlacement->existingPlot()) {
        // assign curve to plot
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(_w->_curvePlacement->plotName()));
        if (plot) {
          plot->addCurve(curve.data());
        }
      }

      if (_w->_curvePlacement->newPlot()) {
        // assign curve to plot
        QString name = w->createPlot(KST::suggestPlotName());
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
        if (_w->_curvePlacement->reGrid()) {
          w->view()->cleanup(_w->_curvePlacement->columns());
        }
        if (plot) {
          _w->_curvePlacement->update();
          _w->_curvePlacement->setCurrentPlot(plot->tagName());
          plot->addCurve(curve.data());
          plot->generateDefaultLabels();
        }
      }
    }
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(curve.data());
  KST::dataObjectList.lock().unlock();

  curve = 0L; // drop the reference
  emit modified();
  return true;
}


bool KstCurveDialog::editSingleObject(KstVCurvePtr cvPtr) {

  { // leave this scope here to destroy the iterator
    KstReadLocker ml(&KST::vectorList.lock());
    KstVectorList::Iterator it;
    if (_xVectorDirty) {
      it = KST::vectorList.findTag(_w->_xVector->selectedVector());
      if (it != KST::vectorList.end()) {
        cvPtr->setXVector(*it);
      }
    }

    if (_yVectorDirty) {
      it = KST::vectorList.findTag(_w->_yVector->selectedVector());
      if (it != KST::vectorList.end()) {
        cvPtr->setYVector(*it);
      }
    }

    if (_xErrorDirty) {
      it = KST::vectorList.findTag(_w->_xError->selectedVector());
      cvPtr->setXError(*it);
    }

    if (_checkBoxXMinusSameAsPlusDirty && _w->_checkBoxXMinusSameAsPlus->isChecked()) {
      cvPtr->setXMinusError(_xErrorDirty ? *it : cvPtr->xErrorVector());
    } else if (_xMinusErrorDirty) {
      it = KST::vectorList.findTag(_w->_xMinusError->selectedVector());
      cvPtr->setXMinusError(*it);
    }

    if (_yErrorDirty) {
      it = KST::vectorList.findTag(_w->_yError->selectedVector());
      cvPtr->setYError(*it);
    }
    if (_checkBoxYMinusSameAsPlusDirty && _w->_checkBoxYMinusSameAsPlus->isChecked()) {
      cvPtr->setYMinusError(_yErrorDirty ? *it : cvPtr->yErrorVector());
    } else if (_yMinusErrorDirty) {
      it = KST::vectorList.findTag(_w->_yMinusError->selectedVector());
      cvPtr->setYMinusError(*it);
    }
  }

  cvPtr->writeLock();

  if (_colorDirty) {
    cvPtr->setColor(_w->_curveAppearance->color());
  }

  if (_showPointsDirty) {
    cvPtr->setHasPoints(_w->_curveAppearance->showPoints());
  }

  if (_showLinesDirty) {
    cvPtr->setHasLines(_w->_curveAppearance->showLines());
  }

  if (_showBarsDirty) {
    cvPtr->setHasBars(_w->_curveAppearance->showBars());
  }

  if (_spinBoxLineWidthDirty) {
    cvPtr->setLineWidth(_w->_curveAppearance->lineWidth());
  }

  if (_comboLineStyleDirty) {
    cvPtr->setLineStyle(_w->_curveAppearance->lineStyle());
  }

  if (_comboDirty) {
    cvPtr->setPointStyle(_w->_curveAppearance->pointType());
  }

  if (_barStyleDirty) {
    cvPtr->setBarStyle(_w->_curveAppearance->barStyle());
  }

  if (_comboPointDensityDirty) {
    cvPtr->setPointDensity(_w->_curveAppearance->pointDensity());
  }

  if (_checkBoxIgnoreAutoscaleDirty) {
    cvPtr->setIgnoreAutoScale(_w->_checkBoxIgnoreAutoscale->isChecked());
  }

  if (_checkBoxYVectorOffsetDirty) {
    cvPtr->setYVectorOffset(_w->_checkBoxYVectorOffset->isChecked());
    if (_scalarSelectorYVectorOffsetDirty) {
      cvPtr->setYVectorOffsetScalar(_w->_scalarSelectorYVectorOffset->selectedScalarPtr());
    }
  }

  if (_interpComboDirty) {
    cvPtr->setInterp(KstVCurve::InterpType(_w->_interp->currentItem()));
  }

  cvPtr->unlock();
  return true;
}


bool KstCurveDialog::editObject() {
  KstVCurveList cvList = kstObjectSubList<KstDataObject,KstVCurve>(KST::dataObjectList);

  // if editing multiple objects, edit each one
  if (_editMultipleMode) {
    // if the user selected no vector, treat it as non-dirty
    _xVectorDirty = _w->_xVector->_vector->currentItem() != 0;
    _yVectorDirty = _w->_yVector->_vector->currentItem() != 0;
    _xErrorDirty = _w->_xError->_vector->currentItem() != 0;
    _xMinusErrorDirty = _w->_xMinusError->_vector->currentItem() != 0;
    _yErrorDirty = _w->_yError->_vector->currentItem() != 0;
    _yMinusErrorDirty = _w->_yMinusError->_vector->currentItem() != 0;
    _spinBoxLineWidthDirty = _w->_curveAppearance->_spinBoxLineWidth->text() != " ";

    // the current selection may not have been set for the following
    _comboDirty = _w->_curveAppearance->_combo->currentItem() > 0;
    _comboLineStyleDirty = _w->_curveAppearance->_comboLineStyle->currentItem() > 0;
    _comboPointDensityDirty = _w->_curveAppearance->_comboPointDensity->currentItem() > 0;
    _barStyleDirty = _w->_curveAppearance->_barStyle->currentItem() > 0;
    _interpComboDirty = (_w->_interp->currentItem()<4);

    _scalarSelectorYVectorOffsetDirty = _w->_scalarSelectorYVectorOffset->_scalar->currentItem() != 0;

    bool didEdit = false;

    for (uint i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->isSelected(i)) {
        // get the pointer to the object
        KstVCurveList::Iterator cvIter = cvList.findTag(_editMultipleWidget->_objectList->text(i));
        if (cvIter == cvList.end()) {
          return false;
        }

        KstVCurvePtr cvPtr = *cvIter;

        if (!editSingleObject(cvPtr)) {
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
    KstVCurvePtr cp = kst_cast<KstVCurve>(_dp);
    // verify that the curve name is unique
    QString tag_name = _tagName->text();
    if (!cp || (tag_name != cp->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();
      return false;
    }

    cp->writeLock();
    cp->setTagName(KstObjectTag(tag_name, cp->tag().context())); // FIXME: doesn't allow changing tag context
    QString legend_text = _legendText->text();
    if (legend_text==defaultTag) {
      cp->setLegendText(QString(""));
    } else {
      cp->setLegendText(legend_text);
    }
    cp->unlock();

    // then edit the object
    _xVectorDirty = true;
    _yVectorDirty = true;
    _xErrorDirty = true;
    _xMinusErrorDirty = true;
    _yErrorDirty = true;
    _yMinusErrorDirty = true;
    _checkBoxXMinusSameAsPlusDirty = true;
    _checkBoxYMinusSameAsPlusDirty = true;
    _colorDirty = true;
    _showPointsDirty = true;
    _showLinesDirty = true;
    _showBarsDirty = true;
    _comboDirty = true;
    _comboPointDensityDirty = true;
    _comboLineStyleDirty = true;
    _spinBoxLineWidthDirty = true;
    _barStyleDirty = true;
    _interpComboDirty = true;
    _checkBoxIgnoreAutoscaleDirty = true;
    _checkBoxYVectorOffsetDirty = true;
    _scalarSelectorYVectorOffsetDirty = true;

    if (!editSingleObject(cp)) {
      return false;
    }
  }
  emit modified();
  return true;
}


void KstCurveDialog::populateEditMultiple() {
  KstVCurveList cvlist = kstObjectSubList<KstDataObject,KstVCurve>(KST::dataObjectList);
  _editMultipleWidget->_objectList->insertStringList(cvlist.tagNames());

  // also intermediate state for multiple edit
  _w->_xVector->_vector->insertItem("", 0);
  _w->_xVector->_vector->setCurrentItem(0);
  _w->_yVector->_vector->insertItem("", 0);
  _w->_yVector->_vector->setCurrentItem(0);
  _w->_xError->_vector->insertItem("", 0);
  _w->_xError->_vector->setCurrentItem(0);
  _w->_yError->_vector->insertItem("", 0);
  _w->_yError->_vector->setCurrentItem(0);
  _w->_xMinusError->_vector->insertItem("", 0);
  _w->_xMinusError->_vector->setCurrentItem(0);
  _w->_yMinusError->_vector->insertItem("", 0);
  _w->_yMinusError->_vector->setCurrentItem(0);
  _w->_scalarSelectorYVectorOffset->_scalar->insertItem("", 0);
  _w->_scalarSelectorYVectorOffset->_scalar->setCurrentItem(0);

  _w->_interp->insertItem("", 4);
  _w->_interp->setCurrentItem(4);

  // single blank characters to differentiate between QPixmaps
  _w->_curveAppearance->_combo->insertItem(" ", 0);
  _w->_curveAppearance->_combo->setCurrentItem(0);
  _w->_curveAppearance->_comboPointDensity->insertItem(" ", 0);
  _w->_curveAppearance->_comboPointDensity->setCurrentItem(0);
  _w->_curveAppearance->_comboLineStyle->insertItem(" ", 0);
  _w->_curveAppearance->_comboLineStyle->setCurrentItem(0);
  _w->_curveAppearance->_barStyle->insertItem(" ", 0);
  _w->_curveAppearance->_barStyle->setCurrentItem(0);

  _w->_curveAppearance->_spinBoxLineWidth->setMinValue(_w->_curveAppearance->_spinBoxLineWidth->minValue() - 1);
  _w->_curveAppearance->_spinBoxLineWidth->setSpecialValueText(" ");
  _w->_curveAppearance->_spinBoxLineWidth->setValue(_w->_curveAppearance->_spinBoxLineWidth->minValue());

  _w->_checkBoxXMinusSameAsPlus->setTristate(true);
  _w->_checkBoxXMinusSameAsPlus->setNoChange();
  _w->_checkBoxYMinusSameAsPlus->setTristate(true);
  _w->_checkBoxYMinusSameAsPlus->setNoChange();
  _w->_curveAppearance->_showPoints->setTristate(true);
  _w->_curveAppearance->_showPoints->setNoChange();
  _w->_curveAppearance->_showLines->setTristate(true);
  _w->_curveAppearance->_showLines->setNoChange();
  _w->_curveAppearance->_showBars->setTristate(true);
  _w->_curveAppearance->_showBars->setNoChange();
  _w->_checkBoxIgnoreAutoscale->setTristate(true);
  _w->_checkBoxIgnoreAutoscale->setNoChange();
  _w->_checkBoxYVectorOffset->setTristate(true);
  _w->_checkBoxYVectorOffset->setNoChange();

  toggledXErrorSame(false);
  toggledYErrorSame(false);

  _tagName->setText("");
  _tagName->setEnabled(false);
  _legendText->setText("");
  _legendText->setEnabled(false);

  // and clean all the fields
  _xVectorDirty = false;
  _yVectorDirty = false;
  _xErrorDirty = false;
  _xMinusErrorDirty = false;
  _yErrorDirty = false;
  _yMinusErrorDirty = false;
  _checkBoxXMinusSameAsPlusDirty = false;
  _checkBoxYMinusSameAsPlusDirty = false;
  _colorDirty = false;
  _showPointsDirty = false;
  _showLinesDirty = false;
  _showBarsDirty = false;
  _comboDirty = false;
  _comboPointDensityDirty = false;
  _comboLineStyleDirty = false;
  _spinBoxLineWidthDirty = false;
  _barStyleDirty = false;
  _checkBoxIgnoreAutoscaleDirty = false;
  _interpComboDirty = false;
  _checkBoxYVectorOffsetDirty = false;
  _scalarSelectorYVectorOffsetDirty = false;
}


void KstCurveDialog::setCheckBoxXMinusSameAsPlusDirty() {
  _w->_checkBoxXMinusSameAsPlus->setTristate(false);
  _checkBoxXMinusSameAsPlusDirty = true;
}


void KstCurveDialog::setCheckBoxYMinusSameAsPlusDirty() {
  _w->_checkBoxYMinusSameAsPlus->setTristate(false);
  _checkBoxYMinusSameAsPlusDirty = true;
}


void KstCurveDialog::setShowPointsDirty() {
  _w->_curveAppearance->_showPoints->setTristate(false);
  _showPointsDirty = true;
}


void KstCurveDialog::setShowLinesDirty() {
  _w->_curveAppearance->_showLines->setTristate(false);
  _showLinesDirty = true;
}


void KstCurveDialog::setShowBarsDirty() {
  _w->_curveAppearance->_showBars->setTristate(false);
  _showBarsDirty = true;
}


void KstCurveDialog::setCheckBoxIgnoreAutoscaleDirty() {
  _w->_checkBoxIgnoreAutoscale->setTristate(false);
  _checkBoxIgnoreAutoscaleDirty = true;
}


void KstCurveDialog::setCheckBoxYVectorOffsetDirty() {
  _w->_checkBoxYVectorOffset->setTristate(false);
  _checkBoxYVectorOffsetDirty = true;
}


void KstCurveDialog::cleanup() {
  // get rid of the blanks in _curveAppearance
  if (_editMultipleMode) {
    _w->_curveAppearance->_combo->removeItem(0);
    _w->_curveAppearance->_comboLineStyle->removeItem(0);
    _w->_curveAppearance->_comboPointDensity->removeItem(0);
    _w->_curveAppearance->_barStyle->removeItem(0);
    _w->_curveAppearance->_spinBoxLineWidth->setSpecialValueText(QString::null);
    _w->_curveAppearance->_spinBoxLineWidth->setMinValue(_w->_curveAppearance->_spinBoxLineWidth->minValue() + 1);
    _w->_interp->removeItem(4);
  }
}


void KstCurveDialog::setVector(const QString& name) {
  _w->_yVector->setSelection(name);
}

#include "kstcurvedialog.moc"
