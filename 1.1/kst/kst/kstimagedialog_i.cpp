/***************************************************************************
                      kstimagedialog_i.cpp  -  Part of KST
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

// include files for Qt
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>

// include files for KDE
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "curveplacementwidget.h"
#include "kstimagedialog_i.h"
#include "kstuinames.h"
#include "kstviewwindow.h"
#include "matrixselector.h"

#define DIALOGTYPE KstImageDialogI
#define DTYPE "Image"
#include "dataobjectdialog.h"

QGuardedPtr<KstImageDialogI> KstImageDialogI::_inst;

KstImageDialogI *KstImageDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstImageDialogI(KstApp::inst());
  }
  return _inst;
}


KstImageDialogI::KstImageDialogI(QWidget* parent,
                                 const char* name, bool modal, WFlags fl)
: KstImageDialog(parent, name, modal, fl) {
  Init();

  connect(_matrix, SIGNAL(newMatrixCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_autoThreshold, SIGNAL(clicked()),
          this, SLOT(calcAutoThreshold()));
  connect(_colorOnly, SIGNAL(clicked()),
          this, SLOT(updateGroups()));
  connect(_contourOnly, SIGNAL(clicked()),
          this, SLOT(updateGroups()));
  connect(_colorAndContour, SIGNAL(clicked()),
          this, SLOT(updateGroups()));
  connect(_useVariableWeight, SIGNAL(clicked()),
          this, SLOT(updateEnables()));
  connect(_realTimeAutoThreshold, SIGNAL(clicked()),
          this, SLOT(updateEnables()));
}

KstImageDialogI::~KstImageDialogI() {
  DP = 0L;
}

KstImagePtr KstImageDialogI::_getPtr(const QString &tagin) {
  KstImageList images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);
  return *(images.findTag(tagin));
}

void KstImageDialogI::updateWindow() {
  _curvePlacement->update();
}

void KstImageDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }

  KstImageList images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);

  DP->readLock();

  //fill in the tag name
  _tagName->setText(DP->tagName());

  //fill in the other parameters
  _lowerZ->setText(QString::number(DP->lowerThreshold()));
  _upperZ->setText(QString::number(DP->upperThreshold()));
  _realTimeAutoThreshold->setChecked(DP->autoThreshold());

  //get the list of installed palettes
  _palette->clear();
  QStringList palList = KPalette::getPaletteList();
  palList.sort();
  _palette->insertStringList(palList);
  int i;
  for (i = 0; i < _palette->count(); i++) {
    if (_palette->text(i) == DP->paletteName()) {
      break;
    }
  }
  _palette->setCurrentItem(i);

  _numContourLines->setValue(DP->numContourLines());
  _contourColor->setColor(DP->contourColor());
  int tempWeight = DP->contourWeight();
  _useVariableWeight->setChecked(tempWeight == -1);
  if (tempWeight >= 0) {
    _contourWeight->setValue(tempWeight);
  }


  //set the type of image
  _colorOnly->setChecked(DP->hasColorMap() && !DP->hasContourMap());
  _contourOnly->setChecked(DP->hasContourMap() && !DP->hasColorMap());
  _colorAndContour->setChecked(DP->hasColorMap() && DP->hasContourMap());

  //set the matrix
  _matrix->setSelection(DP->matrixTag());

  DP->readUnlock();

  //don't place the image in edits
  _curvePlacement->hide();
  //update the groups and enables
  updateGroups();
  updateEnables();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstImageDialogI::_fillFieldsForNew() {
  KstImageList images = kstObjectSubList<KstDataObject, KstImage>(KST::dataObjectList);

  /* set tag name */
  _tagName->setText("<New_Image>");

  //get the list of installed palettes
  _palette->clear();
  QStringList palList = KPalette::getPaletteList();
  palList.sort();
  _palette->insertStringList(palList);

  //some default values
  _lowerZ->setText("0");
  _upperZ->setText("100");
  _realTimeAutoThreshold->setChecked(false);

  //let the image be placed in plots
  _curvePlacement->update();

  //for some reason the widgets need to be placed from bottom to top
  _imageTypeGroup->hide();
  _contourMapGroup->hide();
  _colorMapGroup->hide();
  _matrixGroup->hide();
  _curvePlacement->show();
  _contourMapGroup->show();
  _colorMapGroup->show();
  _imageTypeGroup->show();
  _matrixGroup->show();

  //use whatever setting was used last
  updateGroups();
  updateEnables();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstImageDialogI::update() {
  _matrix->update();
}

bool KstImageDialogI::new_I() {
  //if matrixCombo is empty then display an error message
  if (_matrix->selectedMatrix().isEmpty()){
    KMessageBox::sorry(this, i18n("Matrix is a 2D grid of numbers, used to create image", "New image not made: define matrix first."));
    return false;
  }

  //do some checks on the inputs
  double lowerZDouble, upperZDouble;
  if (!checkParameters(lowerZDouble, upperZDouble)) {
    return false;
  }

  KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
  KstMatrixPtr matrix = *matrices.findTag(_matrix->selectedMatrix());
  if (!matrix) {
    KMessageBox::sorry(this, i18n("Matrix is a 2D grid of numbers, used to create image", "Could not find matrix."));
    return false;
  }
  KST::dataObjectList.lock().readLock();
  matrix->readLock();

  //create a unique name
  QString tag_name = KST::suggestImageName(matrix->tagName());
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    matrix->readUnlock();
    KST::dataObjectList.lock().readUnlock();
    return false;
  }

  KstImagePtr image;
  if (_contourOnly->isChecked()) {
    //need a contour map only
    QColor tempColor = _contourColor->color();
    image = new KstImage(tag_name, matrix, _numContourLines->text().toInt(), tempColor,
                         _useVariableWeight->isChecked() ? -1 : _contourWeight->value());
  } else if (_colorOnly->isChecked()) {
    //need a color map only
    KPalette* newPal = new KPalette(_palette->currentText());
    image = new KstImage(tag_name, matrix, lowerZDouble, upperZDouble,
                         _realTimeAutoThreshold->isChecked(), newPal);
  } else {
    //need both a contour map and colour map
    QColor tempColor = _contourColor->color();
    KPalette* newPal = new KPalette(_palette->currentText());
    image = new KstImage(tag_name, matrix, lowerZDouble, upperZDouble,
                         _realTimeAutoThreshold->isChecked(), newPal,
                         _numContourLines->text().toInt(), tempColor,
                         _useVariableWeight->isChecked() ? -1 : _contourWeight->value());
  }
  matrix->readUnlock();
  KST::dataObjectList.lock().readUnlock();
  placeInPlot(image);
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(image.data());
  KST::dataObjectList.lock().writeUnlock();
  image = 0L; // drop the reference
  emit modified();
  return true;
}

bool KstImageDialogI::edit_I() {
  /* verify that the image name is unique */
  if (_tagName->text() != DP->tagName() && KST::dataTagNameNotUnique(_tagName->text())) {
    return false;
  }
  //check parameters
  double lowerZDouble, upperZDouble;
  if (!checkParameters(lowerZDouble, upperZDouble)) {
    return false;
  }

  //find the matrix
  KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
  KstMatrixPtr matrix = *matrices.findTag(_matrix->selectedMatrix());

  if (!matrix) {
    KMessageBox::sorry(this, i18n("Matrix is a 2D grid of numbers, used to create image", "Could not find matrix."));
    return false;
  }

  matrix->readLock();

  DP->writeLock();
  if (_contourOnly->isChecked()) {
    //need a contour map only
    QColor tempColor = _contourColor->color();
    DP->changeToContourOnly(_tagName->text(), matrix, _numContourLines->text().toInt(), tempColor,
                            _useVariableWeight->isChecked() ? -1 : _contourWeight->value());
  } else if (_colorOnly->isChecked()) {
    //need a color map only
    KPalette* newPal = new KPalette(_palette->currentText());
    DP->changeToColorOnly(_tagName->text(), matrix, lowerZDouble, upperZDouble,
                          _realTimeAutoThreshold->isChecked(), newPal);
  } else {
    //need both a contour map and colour map
    QColor tempColor = _contourColor->color();
    KPalette* newPal = new KPalette(_palette->currentText());
    DP->changeToColorAndContour(_tagName->text(), matrix, lowerZDouble, upperZDouble,
        _realTimeAutoThreshold->isChecked(), newPal,
        _numContourLines->text().toInt(), tempColor,
        _useVariableWeight->isChecked() ? -1 : _contourWeight->value());
  }
  DP->writeUnlock();

  matrix->readUnlock();

  matrices.clear();
  emit modified();
  return true;
}

void KstImageDialogI::calcAutoThreshold() {
  //make sure an matrix is selected
  if (!_matrix->selectedMatrix().isEmpty()){
    KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
    KstMatrixPtr matrix = *matrices.findTag(_matrix->selectedMatrix());
    if (matrix) {
      matrix->readLock();
      _lowerZ->setText(QString::number(matrix->minZ()));
      _upperZ->setText(QString::number(matrix->maxZ()));
      matrix->readUnlock();
    }
  }
}

void KstImageDialogI::placeInPlot(KstImagePtr image) {
  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_curvePlacement->_plotWindow->currentText()));
  if (!w) {
    QString n = KstApp::inst()->newWindow(KST::suggestWinName());
    w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
  }
  if (w) {
    Kst2DPlotPtr plot;
    if (_curvePlacement->existingPlot()) {
      /* assign image to plot */
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(_curvePlacement->plotName()));
      if (plot) {
        plot->addImage(image.data());
      }
    }

    if (_curvePlacement->newPlot()) {
      /* assign image to plot */
      QString name = w->createPlot<Kst2DPlot>(KST::suggestPlotName());
      if (_curvePlacement->reGrid()) {
        w->view()->cleanup(_curvePlacement->columns());
      }
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
      if (plot) {
        _curvePlacement->update();
        _curvePlacement->setCurrentPlot(plot->tagName());
        plot->addImage(image.data());
        plot->GenerateDefaultLabels();
      }
    }
  }
}

void KstImageDialogI::updateGroups() {
  _colorMapGroup->setEnabled(_colorOnly->isChecked() || _colorAndContour->isChecked());
  _contourMapGroup->setEnabled(_contourOnly->isChecked() || _colorAndContour->isChecked());
}

void KstImageDialogI::updateEnables() {
  _contourWeight->setEnabled(!_useVariableWeight->isChecked());
  if (_realTimeAutoThreshold->isChecked()) {
    calcAutoThreshold();
  }
  _lowerZ->setEnabled(!_realTimeAutoThreshold->isChecked());
  _upperZ->setEnabled(!_realTimeAutoThreshold->isChecked());
  _autoThreshold->setEnabled(!_realTimeAutoThreshold->isChecked());
}

bool KstImageDialogI::checkParameters(double& lowerZDouble, double& upperZDouble) {
  if (_colorOnly->isChecked() || _colorAndContour->isChecked()) {
    bool ok1, ok2;
    lowerZDouble = _lowerZ->text().toDouble(&ok1);
    upperZDouble = _upperZ->text().toDouble(&ok2);
    if (!(ok1 && ok2)) {
      KMessageBox::sorry(this, i18n("The upper and lower thresholds are not valid.  Please enter valid decimal numbers for the thresholds."));
      return false;
    }
    if (lowerZDouble > upperZDouble) {
      KMessageBox::sorry(this, i18n("The lower threshold cannot be greater than the upper threshold."));
      return false;
    }
  }
  //for now there is nothing to check for contour maps
  return true;
}
#include "kstimagedialog_i.moc"
// vim: ts=2 sw=2 et
