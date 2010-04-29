/***************************************************************************
                      kstimagedialog.cpp  -  Part of KST
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

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>

#include "colorpalettewidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "kst2dplot.h"
#include "kstdataobjectcollection.h"
#include "kstimagedialog.h"
#include "kstuinames.h"
#include "kstviewwindow.h"
#include "matrixselector.h"

QPointer<KstImageDialog> KstImageDialog::_inst;

KstImageDialog *KstImageDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstImageDialog(KstApp::inst());
  }
  return _inst;
}


KstImageDialog::KstImageDialog(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) : KstDataDialog(parent) {
  _w = new Ui::ImageDialogWidget();
  _w->setupUi(_contents);

  setMultiple(true);
  connect(_w->_matrix, SIGNAL(newMatrixCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_autoThreshold, SIGNAL(clicked()), this, SLOT(calcAutoThreshold()));
  connect(_w->_smartThreshold, SIGNAL(clicked()), this, SLOT(calcSmartThreshold()));
  connect(_w->_colorOnly, SIGNAL(clicked()), this, SLOT(updateGroups()));
  connect(_w->_contourOnly, SIGNAL(clicked()), this, SLOT(updateGroups()));
  connect(_w->_colorAndContour, SIGNAL(clicked()), this, SLOT(updateGroups()));
  connect(_w->_useVariableWeight, SIGNAL(clicked()), this, SLOT(updateEnables()));
  connect(_w->_realTimeAutoThreshold, SIGNAL(clicked()), this, SLOT(updateEnables()));

  //
  // connections for multiple edit mode...
  //

  connect(_w->_colorOnly, SIGNAL(clicked()), this, SLOT(setColorOnlyDirty()));
  connect(_w->_contourOnly, SIGNAL(clicked()), this, SLOT(setContourOnlyDirty()));
  connect(_w->_colorAndContour, SIGNAL(clicked()), this, SLOT(setColorAndContourDirty()));
  connect(_w->_realTimeAutoThreshold, SIGNAL(clicked()), this, SLOT(setRealTimeAutoThresholdDirty()));
  connect(_w->_useVariableWeight, SIGNAL(clicked()), this, SLOT(setUseVariableWeightDirty()));
// xxx  connect(_w->_contourColor, SIGNAL(clicked()), this, SLOT(setContourColorDirty()));

  //
  // connections for apply button...
  //

  connect(_w->_matrix, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_matrix, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_colorOnly, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_contourOnly, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_colorAndContour, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_colorPalette->_palette, SIGNAL(highlighted(int)), this, SLOT(wasModifiedApply()));
  connect(_w->_lowerZ, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_upperZ, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_autoThreshold, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_smartThreshold, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_smartThresholdValue, SIGNAL(valueChanged(double)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_smartThresholdValue->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_realTimeAutoThreshold, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_numContourLines, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_numContourLines->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_contourColor, SIGNAL(changed(const QColor&)), this, SLOT(wasModifiedApply()));
  connect(_w->_contourWeight, SIGNAL(valueChanged(int)), this, SLOT(wasModifiedApply()));
// xxx  connect(_w->_contourWeight->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_useVariableWeight, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
}


KstImageDialog::~KstImageDialog() {
}


void KstImageDialog::updateWindow() {
  _w->_curvePlacement->update();
}


void KstImageDialog::fillFieldsForEdit() {
  fillFieldsForEditNoUpdate();

  KstImagePtr ip;

  ip = kst_cast<KstImage>(_dp);
  if (!ip) {
    return;
  }
  ip->readLock();

  // set the type of image
  _w->_colorOnly->setChecked(ip->hasColorMap() && !ip->hasContourMap());
  _w->_contourOnly->setChecked(ip->hasContourMap() && !ip->hasColorMap());
  _w->_colorAndContour->setChecked(ip->hasColorMap() && ip->hasContourMap());

  // set the matrix
  _w->_matrix->setSelection(ip->matrixTag());

  ip->unlock();

  //update the groups and enables
  // won't call fillFieldsForEditNoUpdate again because
  // fillFieldsForEdit is not called in _editMultipleMode
  updateGroups();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstImageDialog::fillFieldsForEditNoUpdate() {
  KstImagePtr ip;

  ip = kst_cast<KstImage>(_dp);
  if (ip) {
    KstImageList images;
    int tempWeight;

    images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);
  
    ip->readLock();

    _tagName->setText(ip->tagName());
    _w->_lowerZ->setText(QString::number(ip->lowerThreshold()));
    _w->_upperZ->setText(QString::number(ip->upperThreshold()));
    _w->_realTimeAutoThreshold->setChecked(ip->autoThreshold());
    _w->_colorPalette->refresh(ip->paletteName());
    _w->_numContourLines->setValue(ip->numContourLines());
// xxx    _w->_contourColor->setColor(ip->contourColor());
    tempWeight = ip->contourWeight();
    _w->_useVariableWeight->setChecked(tempWeight == -1);
    if (tempWeight >= 0) {
      _w->_contourWeight->setValue(tempWeight);
    }
  
    ip->unlock();

    //
    // don't place the image in edits...
    //

    _w->_curvePlacement->hide();
  
    updateEnables();
  }
}


void KstImageDialog::fillFieldsForNew() {
  KstImageList images;

  images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);

  _tagName->setText("<New_Image>");

  _w->_colorPalette->refresh();
  _w->_lowerZ->setText("0");
  _w->_upperZ->setText("100");
  _w->_realTimeAutoThreshold->setChecked(true);

  _w->_curvePlacement->update();

  //
  // for some reason the widgets need to be placed from bottom to top...
  //

  _w->_imageTypeGroup->hide();
  _w->_contourMapGroup->hide();
  _w->_colorMapGroup->hide();
  _w->_matrixGroup->hide();
  _w->_curvePlacement->show();
  _w->_contourMapGroup->show();
  _w->_colorMapGroup->show();
  _w->_imageTypeGroup->show();
  _w->_matrixGroup->show();

  //
  // use whatever setting was used last...
  //

  updateGroups();
  updateEnables();
  _w->_colorPalette->updatePalette(_w->_colorPalette->selectedPalette());
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstImageDialog::update() {
  _w->_matrix->update();
  _w->_curvePlacement->update();
}


bool KstImageDialog::newObject() {
  //if matrixCombo is empty then display an error message
  if (_w->_matrix->selectedMatrix().isEmpty()){
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Matrix is a 2D grid of numbers, used to create image", "New image not made: define matrix first."));

    return false;
  }

  KstMatrixPtr matrix;
  double lowerZDouble;
  double upperZDouble;

  if (!checkParameters(lowerZDouble, upperZDouble)) {
    return false;
  }

  KST::matrixList.lock().readLock();
  matrix = *KST::matrixList.findTag(_w->_matrix->selectedMatrix());
  KST::matrixList.lock().unlock();
  if (!matrix) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Matrix is a 2D grid of numbers, used to create image", "Could not find matrix."));

    return false;
  }
  KST::dataObjectList.lock().readLock();
  matrix->readLock();

  KstImagePtr image;
  QString tag_name = KST::suggestImageName(matrix->tag());

  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    matrix->unlock();
    KST::dataObjectList.lock().unlock();

    return false;
  }

  if (_w->_contourOnly->isChecked()) {
    QColor tempColor;

// xxx    tempColor = _w->_contourColor->color();
    image = new KstImage(tag_name, matrix, _w->_numContourLines->text().toInt(), tempColor,
                         _w->_useVariableWeight->isChecked() ? -1 : _w->_contourWeight->value());
  } else if (_w->_colorOnly->isChecked()) {
/* xxx
    KPalette* newPal = new KPalette(_w->_colorPalette->selectedPalette());

    image = new KstImage(tag_name, matrix, lowerZDouble, upperZDouble,
                         _w->_realTimeAutoThreshold->isChecked(), newPal);
*/
  } else {
    QColor tempColor;
/* xxx
    tempColor = _w->_contourColor->color();
    KPalette* newPal = new KPalette(_w->_colorPalette->selectedPalette());

    image = new KstImage(tag_name, matrix, lowerZDouble, upperZDouble,
                         _w->_realTimeAutoThreshold->isChecked(), newPal,
                         _w->_numContourLines->text().toInt(), tempColor,
                         _w->_useVariableWeight->isChecked() ? -1 : _w->_contourWeight->value());
*/
  }
  matrix->unlock();
  KST::dataObjectList.lock().unlock();
  placeInPlot(image);
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(image);
  KST::dataObjectList.lock().unlock();

  image = 0L; // drop the reference

  emit modified();

  return true;
}


bool KstImageDialog::editSingleObject(KstImagePtr imPtr) {
  KstMatrixPtr matrix;

  if (_matrixDirty) {
    KST::matrixList.lock().readLock();
    matrix = *KST::matrixList.findTag(_w->_matrix->selectedMatrix());
    KST::matrixList.lock().unlock();

    if (!matrix) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Could not find matrix."));

      return false;
    }
  } else {
    imPtr->readLock();
    matrix = imPtr->matrix();
    imPtr->unlock();
  }

  imPtr->writeLock();

  // if image type was changed, get all parameters from the dialog
  if (_contourOnlyDirty || _colorOnlyDirty || _colorAndContourDirty) {
    double lowerZDouble;
    double upperZDouble;

    if (!checkParameters(lowerZDouble, upperZDouble)) {
      //QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Image type was changed: Lower Z threshold cannot be higher than Upper Z threshold."));
      //pMatrix->unlock();
      imPtr->unlock();

      return false;
    }

    if (_w->_contourOnly->isChecked()) {
      QColor tempColor;

// xxx      tempColor = _w->_contourColor->color();
      imPtr->changeToContourOnly(imPtr->tagName(), matrix, _w->_numContourLines->text().toInt(), tempColor,
                              _w->_useVariableWeight->isChecked() ? -1 : _w->_contourWeight->value());
    } else if (_w->_colorOnly->isChecked()) {
/* xxx
      KPalette* newPal = new KPalette(_w->_colorPalette->selectedPalette());

      imPtr->changeToColorOnly(imPtr->tagName(), pMatrix, lowerZDouble, upperZDouble,
                            _w->_realTimeAutoThreshold->isChecked(), newPal);
*/
    } else {
      QColor tempColor;

// xxx      tempColor = _w->_contourColor->color();
/* xxx
      KPalette* newPal = new KPalette(_w->_colorPalette->selectedPalette());

      imPtr->changeToColorAndContour(imPtr->tagName(), pMatrix, lowerZDouble, upperZDouble,
                                  _w->_realTimeAutoThreshold->isChecked(), newPal,
                                  _w->_numContourLines->text().toInt(), tempColor,
                                  _w->_useVariableWeight->isChecked() ? -1 : _w->_contourWeight->value());
*/
    }
  } else {
    // get the current or new parameters as required
    QColor pContourColor;
    double pLowerZ, pUpperZ;
    int pNumContours, pContourWeight;
    bool pRealTimeAutoThreshold, pUseVariableWeight;

    if (_lowerZDirty) {
      pLowerZ = _w->_lowerZ->text().toDouble();
    } else {
      pLowerZ = imPtr->lowerThreshold();
    }

    if (_upperZDirty) {
      pUpperZ = _w->_upperZ->text().toDouble();
    } else {
      pUpperZ = imPtr->upperThreshold();
    }

    if (_realTimeAutoThresholdDirty) {
      pRealTimeAutoThreshold = _w->_realTimeAutoThreshold->isChecked();
    } else {
      pRealTimeAutoThreshold = imPtr->autoThreshold();
    }

    if (_numContourLinesDirty) {
      pNumContours = _w->_numContourLines->text().toInt();
    } else {
      pNumContours = imPtr->numContourLines();
    }

    if (_contourWeightDirty) {
      pContourWeight = _w->_contourWeight->value();
    } else {
      pContourWeight = imPtr->contourWeight();
    }

    if (_useVariableWeightDirty) {
      pUseVariableWeight = _w->_useVariableWeight->isChecked();
    } else {
      pUseVariableWeight = imPtr->contourWeight() == -1;
    }

    if (_contourColorDirty) {
// xxx      pContourColor = _w->_contourColor->color();
    } else {
      pContourColor = imPtr->contourColor();
    }

    // check parameters for color map
    if (imPtr->hasColorMap()) {
      if (pLowerZ > pUpperZ) {
        imPtr->unlock();

        return false;
      }
    }

    // don't change the image type, just change applicable settings for the
    // current image type
    if (imPtr->hasContourMap() && !imPtr->hasColorMap()) {
      imPtr->changeToContourOnly(imPtr->tagName(), matrix, pNumContours, pContourColor,
          pUseVariableWeight ? -1 : pContourWeight);
    } else {
/* xxx
      KPalette *palette;

      if (_paletteDirty) {
        palette = new KPalette(_w->_colorPalette->selectedPalette());
      } else {
        palette = imPtr->palette();
      }

      if (imPtr->hasColorMap() && !imPtr->hasContourMap()) {
        imPtr->changeToColorOnly(imPtr->tagName(), pMatrix, pLowerZ, pUpperZ,
            pRealTimeAutoThreshold, palette);
      } else {
        //
        // images always have at least one of color or contour maps...
        //

        imPtr->changeToColorAndContour(imPtr->tagName(), pMatrix, pLowerZ, pUpperZ,
            pRealTimeAutoThreshold, palette,
            pNumContours, pContourColor,
            pUseVariableWeight ? -1 : pContourWeight);
      }
*/
    }
  }


  imPtr->unlock();

  return true;
}

bool KstImageDialog::editObject() {
  KstImageList imList;

  imList = kstObjectSubList<KstDataObject,KstImage>(KST::dataObjectList);

  if (_editMultipleMode) {
    _numContourLinesDirty = _w->_numContourLines->text() != " ";
    _contourWeightDirty = _w->_contourWeight->text() != " ";
    _paletteDirty = _w->_colorPalette->currentPaletteIndex() != 0;
    _matrixDirty = _w->_matrix->_matrix->currentIndex() != 0;
    _lowerZDirty = !_w->_lowerZ->text().isEmpty();
    _upperZDirty = !_w->_upperZ->text().isEmpty();

    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); ++i) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstImageList::iterator imIter;
        KstImagePtr imPtr;

        imIter = imList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        if (imIter == imList.end()) {
          return false;
        }

        imPtr = *imIter;

        if (!editSingleObject(imPtr)) {
          return false;
        }
        didEdit = true;
      }
    }

    if (!didEdit) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Select one or more objects to edit."));

      return false;
    }
  } else {
    KstImagePtr ip;
    QString tag_name = _tagName->text();

    ip = kst_cast<KstImage>(_dp);
    if (!ip || (tag_name != ip->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();

      return false;
    }

    ip->writeLock();
    ip->setTagName(KstObjectTag(tag_name, ip->tag().context())); // FIXME: doesn't allow changing tag context
    ip->unlock();

    //
    // then edit the object...
    //

    _colorOnlyDirty = true;
    _contourOnlyDirty = true;
    _colorAndContourDirty = true;
    _paletteDirty = true;
    _lowerZDirty = true;
    _upperZDirty = true;
    _realTimeAutoThresholdDirty = true;
    _numContourLinesDirty = true;
    _contourWeightDirty = true;
    _useVariableWeightDirty = true;
    _contourColorDirty = true;
    if (!editSingleObject(ip)) {
      return false;
    }
  }

  emit modified();

  return true;
}


void KstImageDialog::calcAutoThreshold() {
 if (!_w->_matrix->selectedMatrix().isEmpty()){
    KstMatrixPtr matrix;

    KST::matrixList.lock().readLock();
    matrix = *KST::matrixList.findTag(_w->_matrix->selectedMatrix());
    KST::matrixList.lock().unlock();

    if (matrix) {
      matrix->readLock();
      _w->_lowerZ->setText(QString::number(matrix->minValue()));
      _w->_upperZ->setText(QString::number(matrix->maxValue()));
      matrix->unlock();
    }
  }
}


void KstImageDialog::calcSmartThreshold() {
  if (!_w->_matrix->selectedMatrix().isEmpty()){
    KstMatrixPtr matrix;

    KST::matrixList.lock().readLock();
    matrix = *KST::matrixList.findTag(_w->_matrix->selectedMatrix());
    KST::matrixList.lock().unlock();

    if (matrix) {
      double per = _w->_smartThresholdValue->value()/100.0;

      matrix->readLock();
      matrix->calcNoSpikeRange(per);
      _w->_lowerZ->setText(QString::number(matrix->minValueNoSpike()));
      _w->_upperZ->setText(QString::number(matrix->maxValueNoSpike()));
      matrix->unlock();
    }
  }
}


void KstImageDialog::placeInPlot(KstImagePtr image) {
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
        plot->setXScaleMode(AUTO);
        plot->setYScaleMode(AUTO);
        plot->addCurve(KstBaseCurvePtr(image));
        plot->generateDefaultLabels();
      }
    }
  }
}


void KstImageDialog::updateGroups() {
  _w->_colorMapGroup->setEnabled(_w->_colorOnly->isChecked() || _w->_colorAndContour->isChecked());
  _w->_contourMapGroup->setEnabled(_w->_contourOnly->isChecked() || _w->_colorAndContour->isChecked());

  // if editing multiple, also set some defaults for the newly enabled groups
  if (_editMultipleMode) {
    fillFieldsForEditNoUpdate();
  }
}


void KstImageDialog::updateEnables() {
  if (!_w->_useVariableWeight->isTristate()) {
    _w->_contourWeight->setEnabled(!_w->_useVariableWeight->isChecked());
  }

  if (!_w->_realTimeAutoThreshold->isTristate()) {
    if (_w->_realTimeAutoThreshold->isChecked()) {
      calcAutoThreshold();
    }
    _w->_lowerZ->setEnabled(!_w->_realTimeAutoThreshold->isChecked());
    _w->_upperZ->setEnabled(!_w->_realTimeAutoThreshold->isChecked());
    _w->_autoThreshold->setEnabled(!_w->_realTimeAutoThreshold->isChecked() && !_editMultipleMode);
    _w->_smartThreshold->setEnabled(!_w->_realTimeAutoThreshold->isChecked() && !_editMultipleMode);
    _w->_smartThresholdValue->setEnabled(!_w->_realTimeAutoThreshold->isChecked() && !_editMultipleMode);
  }
}


bool KstImageDialog::checkParameters(double& lowerZDouble, double& upperZDouble) {
  if (_w->_colorOnly->isChecked() || _w->_colorAndContour->isChecked()) {
    bool ok1, ok2;

    lowerZDouble = _w->_lowerZ->text().toDouble(&ok1);
    upperZDouble = _w->_upperZ->text().toDouble(&ok2);

    if (!(ok1 && ok2)) {
      if (ok1 || ok2) {
        if (ok1) {
          QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The upper threshold is not a valid decimal number."));
        } else {
          QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The lower threshold is not a valid decimal number."));
        }
      } else {
        QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The upper and lower thresholds are not valid decimal numbers."));
      }

      return false;
    }

    if (lowerZDouble >= upperZDouble) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The upper threshold must be greater than the lower threshold."));

      return false;
    }
  }

  return true;
}


void KstImageDialog::populateEditMultiple() {
  KstImageList imlist;

  imlist = kstObjectSubList<KstDataObject,KstImage>(KST::dataObjectList);
  _editMultipleWidget->_objectList->insertItems(0, imlist.tagNames());

  //
  // also intermediate state for multiple edit...
  //

  _w->_colorOnly->setChecked(false);
  _w->_contourOnly->setChecked(false);
  _w->_colorAndContour->setChecked(false);
  _w->_colorMapGroup->setEnabled(true);
  _w->_contourMapGroup->setEnabled(true);
  _w->_colorPalette->_palette->insertItem(0, "");
  _w->_colorPalette->_palette->setCurrentIndex(0);
  _w->_matrix->_matrix->insertItem(0, "");
  _w->_matrix->_matrix->setCurrentIndex(0);
  _w->_lowerZ->setText("");
  _w->_upperZ->setText("");
  _w->_realTimeAutoThreshold->setTristate(true);
  _w->_realTimeAutoThreshold->setChecked(Qt::PartiallyChecked);
  _w->_autoThreshold->setEnabled(false);
  _w->_numContourLines->setSpecialValueText(" ");
  _w->_numContourLines->setMinimum(_w->_numContourLines->minimum() - 1);
  _w->_numContourLines->setValue(_w->_numContourLines->minimum());
  _w->_contourWeight->setSpecialValueText(" ");
  _w->_contourWeight->setMinimum(_w->_contourWeight->minimum() - 1);
  _w->_contourWeight->setValue(_w->_contourWeight->minimum());
  _w->_useVariableWeight->setTristate(true);
  _w->_useVariableWeight->setChecked(Qt::PartiallyChecked);
// xxx  _w->_contourColor->setColor(QColor()); //default color

  _tagName->setText("");
  _tagName->setEnabled(false);

  _w->_lowerZ->setEnabled(true);
  _w->_upperZ->setEnabled(true);
  _w->_contourWeight->setEnabled(true);

  //
  // and clean all the fields...
  //

  _colorOnlyDirty = false;
  _contourOnlyDirty = false;
  _colorAndContourDirty = false;
  _paletteDirty = false;
  _lowerZDirty = false;
  _upperZDirty = false;
  _realTimeAutoThresholdDirty = false;
  _numContourLinesDirty = false;
  _contourWeightDirty = false;
  _useVariableWeightDirty = false;
  _contourColorDirty = false;
}


void KstImageDialog::setRealTimeAutoThresholdDirty() {
  _w->_realTimeAutoThreshold->setTristate(false);
  _realTimeAutoThresholdDirty = true;
  updateEnables();
}


void KstImageDialog::setUseVariableWeightDirty() {
  _w->_useVariableWeight->setTristate(false);
  _useVariableWeightDirty = true;
  updateEnables();
}


void KstImageDialog::cleanup() {
  if (_editMultipleMode) {
    _w->_numContourLines->setSpecialValueText(QString::null);
    _w->_numContourLines->setMinimum(_w->_numContourLines->minimum() + 1);
    _w->_contourWeight->setSpecialValueText(QString::null);
    _w->_contourWeight->setMinimum(_w->_contourWeight->minimum() + 1);
    _w->_autoThreshold->setEnabled(true);
  }
}


void KstImageDialog::setMatrix(const QString& name) {
  _w->_matrix->setSelection(name);
}

#include "kstimagedialog.moc"
