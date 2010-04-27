/***************************************************************************
                       kstcsddialog.cpp  -  Part of KST
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

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QSpinBox>

#include "fftoptionswidget.h"
#include "colorpalettewidget.h"
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "kst2dplot.h"
#include "kstcsddialog.h"
#include "kstdataobjectcollection.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "vectorselector.h"

const QString& KstCsdDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

QPointer<KstCsdDialog> KstCsdDialog::_inst = 0L;

KstCsdDialog *KstCsdDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstCsdDialog(KstApp::inst());
  }
  return _inst;
}


KstCsdDialog::KstCsdDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstDataDialog(parent) {
  _w = new Ui::CSDDialogWidget();
  _w->setupUi(parent);

  setMultiple(true);
  connect(_w->_vector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));

  //for multiple edit mode
  connect(_w->_kstFFTOptions->Apodize, SIGNAL(clicked()), this, SLOT(setApodizeDirty()));
  connect(_w->_kstFFTOptions->RemoveMean, SIGNAL(clicked()), this, SLOT(setRemoveMeanDirty()));
  connect(_w->_kstFFTOptions->Interleaved, SIGNAL(clicked()), this, SLOT(setInterleavedDirty()));
  connect(_w->_kstFFTOptions->InterpolateHoles, SIGNAL(clicked()), this, SLOT(setInterpolateHolesDirty()));

  // for apply button
  connect(_w->_vector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_vector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_windowSize, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_windowSize->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->ApodizeFxn, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->FFTLen, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_kstFFTOptions->FFTLen->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->Sigma, SIGNAL(valueChanged(double)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_kstFFTOptions->Sigma->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->Apodize, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->RemoveMean, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->Interleaved, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->InterpolateHoles, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->SampRate, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->VectorUnits, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->RateUnits, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_kstFFTOptions->Output, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
}


KstCsdDialog::~KstCsdDialog() {
}


void KstCsdDialog::updateWindow() {
}


void KstCsdDialog::fillFieldsForEdit() {
  KstCSDPtr cp;

  cp = kst_cast<KstCSD>(_dp);
  if (!cp) {
    return; // shouldn't be needed
  }

  cp->readLock();

  _tagName->setText(cp->tagName());

  _w->_vector->setSelection(cp->vTag());

  // set sample rate, Units, FFT len, and vector units
  _w->_kstFFTOptions->FFTLen->setValue(cp->length());
  _w->_kstFFTOptions->SampRate->setText(QString::number(cp->freq()));
  _w->_kstFFTOptions->VectorUnits->setText(cp->vectorUnits());
  _w->_kstFFTOptions->RateUnits->setText(cp->rateUnits());
  _w->_kstFFTOptions->Apodize->setChecked(cp->apodize());
  _w->_kstFFTOptions->ApodizeFxn->setCurrentIndex(cp->apodizeFxn());
  _w->_kstFFTOptions->Sigma->setValue(cp->gaussianSigma());
  _w->_kstFFTOptions->RemoveMean->setChecked(cp->removeMean());
  _w->_kstFFTOptions->Interleaved->setChecked(cp->average());
  _w->_kstFFTOptions->Output->setCurrentIndex(cp->output());
  _w->_kstFFTOptions->InterpolateHoles->setChecked(cp->interpolateHoles());
  _w->_windowSize->setValue(cp->windowSize());
  _w->_kstFFTOptions->synch();

  cp->unlock();

  _w->_imageOptionsGroup->hide();
  _w->_curvePlacement->hide();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstCsdDialog::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _w->_kstFFTOptions->update();
  _w->_colorPalette->refresh();
  _w->_curvePlacement->update();
  _w->_imageOptionsGroup->show();
  _w->_curvePlacement->show();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstCsdDialog::update() {
  _w->_vector->update();
  _w->_curvePlacement->update();
}


bool KstCsdDialog::newObject() {
  QString tag_name = _tagName->text();
  if (tag_name == defaultTag) {
    tag_name = KST::suggestCSDName(KstObjectTag::fromString(_w->_vector->selectedVector()));
  }

  //
  // verify that the curve name is unique...
  //

  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (_w->_vector->selectedVector().isEmpty()) {
    QMessageBox::warning(this, i18n("Kst"), i18n("New spectrogram not made: define vectors first."));
    return false;
  }

  KST::vectorList.lock().readLock();
  KstVectorPtr p = *KST::vectorList.findTag(_w->_vector->selectedVector());
  KST::vectorList.lock().unlock();

  if (p) {
    ApodizeFunction apodizeFxn = ApodizeFunction(_w->_kstFFTOptions->ApodizeFxn->currentIndex());
    bool apodize = _w->_kstFFTOptions->Apodize->isChecked();
    double gaussianSigma = _w->_kstFFTOptions->Sigma->value();
    bool removeMean = _w->_kstFFTOptions->RemoveMean->isChecked();
    bool average = _w->_kstFFTOptions->Interleaved->isChecked();
    int windowSize = _w->_windowSize->value();
    int length = _w->_kstFFTOptions->FFTLen->value();
    double freq = _w->_kstFFTOptions->SampRate->text().toDouble();
    PSDType output = PSDType(_w->_kstFFTOptions->Output->currentIndex());
    QString vectorUnits = _w->_kstFFTOptions->VectorUnits->text();
    QString rateUnits = _w->_kstFFTOptions->RateUnits->text();
    _w->_kstFFTOptions->synch();
  
    KstCSDPtr csd;
    KstImagePtr image;

    csd = new KstCSD(tag_name, p, freq, average, removeMean,apodize, 
                                apodizeFxn, windowSize, length, gaussianSigma, output,
                                vectorUnits, rateUnits);
    image = createImage(csd);
  
    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.append(csd);
    KST::dataObjectList.append(image);
    KST::dataObjectList.lock().unlock();
  
    csd = 0L;
// xxx    emit modified();
  }

  return true;
}


KstImagePtr KstCsdDialog::createImage(KstCSDPtr csd) {
  KstImagePtr image;
/* xxx
  KPalette* newPal = new KPalette(_w->_colorPalette->selectedPalette());

  csd->readLock();
  image = new KstImage(csd->tagName()+"-I", csd->outputMatrix(), 0, 1, true, newPal);
  csd->unlock();

  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_w->_curvePlacement->_plotWindow->currentText()));
  if (!w) {
    QString n = KstApp::inst()->newWindow(KST::suggestWinName());
    w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
  }
  if (w) {
    Kst2DPlotPtr plot;

    if (_w->_curvePlacement->existingPlot()) {
      // assign image to plot
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(_w->_curvePlacement->plotName()));
      if (plot) {
        plot->addCurve(KstBaseCurvePtr(image));
      }
    }

    if (_w->_curvePlacement->newPlot()) {
      // assign image to plot
      QString name = w->createPlot(KST::suggestPlotName());

      if (_w->_curvePlacement->reGrid()) {
        w->view()->cleanup(_w->_curvePlacement->columns());
      }
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
      if (plot) {
        _w->_curvePlacement->update();
        _w->_curvePlacement->setCurrentPlot(plot->tagName());
        plot->addCurve(KstBaseCurvePtr(image));
        plot->generateDefaultLabels();
      }
    }
  }
*/
  return image;
}


bool KstCsdDialog::editSingleObject(KstCSDPtr csPtr) {
  KstVectorPtr v;

  csPtr->writeLock();

  KST::vectorList.lock().readLock();
  v = *KST::vectorList.findTag(_w->_vector->selectedVector());
  KST::vectorList.lock().unlock();

  if (v) { // Can be null if edit multiple and it wasn't changed
    csPtr->setVector(v);
  }

  // get the values that need to be checked for consistency
  double sampRate;
  int fFTLen;

  if (_sampRateDirty) {
    sampRate = _w->_kstFFTOptions->SampRate->text().toDouble();
  } else {
    sampRate = csPtr->freq();
  }

  if (_fFTLenDirty) {
    fFTLen = _w->_kstFFTOptions->FFTLen->text().toInt();
  } else {
    fFTLen = csPtr->length();
  }

  if (!_w->_kstFFTOptions->checkGivenValues(sampRate, fFTLen)) {
    csPtr->unlock();
    return false;
  }

  if (_sampRateDirty) {
    csPtr->setFreq(_w->_kstFFTOptions->SampRate->text().toDouble());
  }

  if (_fFTLenDirty) {
    csPtr->setLength(_w->_kstFFTOptions->FFTLen->text().toInt());
  }

  if (_apodizeDirty) {
    csPtr->setApodize(_w->_kstFFTOptions->Apodize->isChecked());
  }

  if (_apodizeFxnDirty) {
    if (_editMultipleMode) {
      csPtr->setApodizeFxn(ApodizeFunction(_w->_kstFFTOptions->ApodizeFxn->currentIndex()-1));
    } else {
      csPtr->setApodizeFxn(ApodizeFunction(_w->_kstFFTOptions->ApodizeFxn->currentIndex()));
    }
  }

  if (_gaussianSigmaDirty) {
    csPtr->setGaussianSigma(_editMultipleMode ? _w->_kstFFTOptions->Sigma->value() - 1.0 :
                                                _w->_kstFFTOptions->Sigma->value());
  }

  if (_removeMeanDirty) {
    csPtr->setRemoveMean(_w->_kstFFTOptions->RemoveMean->isChecked());
  }

  if (_interleavedDirty) {
    csPtr->setAverage(_w->_kstFFTOptions->Interleaved->isChecked());
  }

  if (_windowSizeDirty) {
    csPtr->setWindowSize(_w->_windowSize->value());
  }

  if (_rateUnitsDirty) {
    csPtr->setRateUnits(_w->_kstFFTOptions->RateUnits->text());
  }

  if (_vectorUnitsDirty) {
    csPtr->setVectorUnits(_w->_kstFFTOptions->VectorUnits->text());
  }

  if (_outputDirty) {
    if (_editMultipleMode) {
      csPtr->setOutput(PSDType(_w->_kstFFTOptions->Output->currentIndex()-1));
    } else {
      csPtr->setOutput(PSDType(_w->_kstFFTOptions->Output->currentIndex()));
    }
  }

  if (_interpolateHolesDirty) {
    csPtr->setInterpolateHoles(_w->_kstFFTOptions->InterpolateHoles->isChecked());
  }

  csPtr->setRecursed(false);
  if (csPtr->recursion()) {
    csPtr->setRecursed(true);
    csPtr->unlock();
    QMessageBox::warning(this, i18n("Kst"), i18n("There is a recursion resulting from the spectrogram you entered."));
    return false;
  }

  csPtr->unlock();
  return true;
}



bool KstCsdDialog::editObject() {
  //
  // if the user selected no vector, treat it as non-dirty
  //

  _vectorDirty = _w->_vector->_vector->currentIndex() != 0;
  _fFTLenDirty = _w->_kstFFTOptions->FFTLen->text() != " ";
  _sampRateDirty = !_w->_kstFFTOptions->SampRate->text().isEmpty();
  _vectorUnitsDirty = !_w->_kstFFTOptions->VectorUnits->text().isEmpty();
  _rateUnitsDirty = !_w->_kstFFTOptions->RateUnits->text().isEmpty();
  _windowSizeDirty = _w->_windowSize->text() != " ";
  _apodizeFxnDirty = _w->_kstFFTOptions->ApodizeFxn->currentIndex() != 0;
  _gaussianSigmaDirty = _w->_kstFFTOptions->Sigma->value() != _w->_kstFFTOptions->Sigma->minimum();
  _outputDirty =  _w->_kstFFTOptions->Output->currentIndex() != 0;

  KstCSDList csList;

  csList = kstObjectSubList<KstDataObject,KstCSD>(KST::dataObjectList);

  //
  // if editing multiple objects, edit each one
  //

  if (_editMultipleMode) {
    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstCSDList::iterator csIter;
        KstCSDPtr csPtr;

        //
        // get the pointer to the object...
        //

        csIter = csList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        if (csIter == csList.end()) {
          return false;
        }

        csPtr = *csIter;

        if (!editSingleObject(csPtr)) {
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
    KstCSDPtr cp;
    QString tag_name = _tagName->text();

    //
    // verify that the name is unique...
    //

    cp = kst_cast<KstCSD>(_dp);
    if (!cp || (tag_name != cp->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();
      return false;
    }

    cp->writeLock();
    cp->setTagName(tag_name);
    cp->unlock();

    // then edit the object
    _vectorDirty = true;
    _apodizeDirty = true;
    _apodizeFxnDirty = true;
    _gaussianSigmaDirty = true;
    _removeMeanDirty = true;
    _interleavedDirty = true;
    _sampRateDirty = true;
    _vectorUnitsDirty = true;
    _rateUnitsDirty = true;
    _fFTLenDirty = true;
    _windowSizeDirty = true;
    _outputDirty = true;
    _interpolateHolesDirty = true;
    if (!editSingleObject(cp)) {
      return false;
    }
  }
// xxx  emit modified();

  return true;
}


void KstCsdDialog::populateEditMultiple() {
  KstCSDList csList;

  csList = kstObjectSubList<KstDataObject,KstCSD>(KST::dataObjectList);
  _editMultipleWidget->_objectList->insertItems(0, csList.tagNames());

  //
  // also intermediate state for multiple edit
  //

  _w->_vector->_vector->insertItem(0, "");
  _w->_vector->_vector->setCurrentIndex(0);
  _w->_kstFFTOptions->Apodize->setChecked(Qt::PartiallyChecked);
  _w->_kstFFTOptions->ApodizeFxn->insertItem(0, "");
  _w->_kstFFTOptions->ApodizeFxn->setCurrentIndex(0);
  _w->_kstFFTOptions->Sigma->setMinimum(_w->_kstFFTOptions->Sigma->minimum() - 0.01);
  _w->_kstFFTOptions->Sigma->setSpecialValueText(" ");
  _w->_kstFFTOptions->Sigma->setValue(_w->_kstFFTOptions->Sigma->minimum());
  _w->_kstFFTOptions->RemoveMean->setChecked(Qt::PartiallyChecked);
  _w->_kstFFTOptions->Interleaved->setChecked(Qt::PartiallyChecked);
  _w->_kstFFTOptions->InterpolateHoles->setChecked(Qt::PartiallyChecked);
  _w->_kstFFTOptions->SampRate->setText("");
  _w->_kstFFTOptions->VectorUnits->setText("");
  _w->_kstFFTOptions->RateUnits->setText("");
  _w->_kstFFTOptions->FFTLen->setMinimum(_w->_kstFFTOptions->FFTLen->minimum() - 1);
  _w->_kstFFTOptions->FFTLen->setSpecialValueText(" ");
  _w->_kstFFTOptions->FFTLen->setValue(_w->_kstFFTOptions->FFTLen->minimum());
  _w->_kstFFTOptions->Output->insertItem(0, "");
  _w->_kstFFTOptions->Output->setCurrentIndex(0);
  _w->_windowSize->setMinimum(_w->_windowSize->minimum() - 1);
  _w->_windowSize->setSpecialValueText(" ");
  _w->_windowSize->setValue(_w->_windowSize->minimum());

  _tagName->setText("");
  _tagName->setEnabled(false);

  // and clean all the fields
  _vectorDirty = false;
  _apodizeDirty = false;
  _apodizeFxnDirty = false;
  _gaussianSigmaDirty = false;
  _removeMeanDirty = false;
  _interleavedDirty = false;
  _sampRateDirty = false;
  _vectorUnitsDirty = false;
  _rateUnitsDirty = false;
  _fFTLenDirty = false;
  _outputDirty = false;
  _windowSizeDirty = false;
  _interpolateHolesDirty = false;
}


void KstCsdDialog::cleanup() {
  if (_editMultipleMode) {
     _w->_kstFFTOptions->FFTLen->setMinimum(_w->_kstFFTOptions->FFTLen->minimum() + 1);
     _w->_kstFFTOptions->FFTLen->setSpecialValueText(QString::null);
     _w->_kstFFTOptions->Sigma->setMinimum(_w->_kstFFTOptions->Sigma->minimum() + 0.01);
     _w->_kstFFTOptions->Sigma->setSpecialValueText(QString::null);
     _w->_kstFFTOptions->ApodizeFxn->removeItem(0);
     _w->_kstFFTOptions->Output->removeItem(0);
  }
}


void KstCsdDialog::setApodizeDirty() {
  _apodizeDirty = true;
  _w->_kstFFTOptions->Apodize->setTristate(false);
}


void KstCsdDialog::setRemoveMeanDirty() {
  _removeMeanDirty = true;
  _w->_kstFFTOptions->RemoveMean->setTristate(false);
}


void KstCsdDialog::setInterpolateHolesDirty() {
  _w->_kstFFTOptions->InterpolateHoles->setTristate(false);
  _interpolateHolesDirty = true;
}


void KstCsdDialog::setInterleavedDirty() {
  _w->_kstFFTOptions->Interleaved->setTristate(false);
  _interleavedDirty = true;
  // also set the FFTLen to be dirty, as presumably the user will think it has been edited
  _fFTLenDirty = true;
}


void KstCsdDialog::setVector(const QString& name) {
  _w->_vector->setSelection(name);
}

#include "kstcsddialog.moc"
