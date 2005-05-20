/***************************************************************************
                      kstmatrixdialog_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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
#include <qcheckbox.h>
#include <qspinbox.h>

// include files for KDE
#include <kcombobox.h>
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "kstmatrixdialog_i.h"
#include "kstviewwindow.h"
#include "vectorselector.h"

#define DIALOGTYPE KstMatrixDialogI
#define DTYPE "Matrix"
#include "dataobjectdialog.h"

QGuardedPtr<KstMatrixDialogI> KstMatrixDialogI::_inst;

KstMatrixDialogI *KstMatrixDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstMatrixDialogI(KstApp::inst());
  }
  return _inst;
}


KstMatrixDialogI::KstMatrixDialogI(QWidget* parent,
                                 const char* name, bool modal, WFlags fl)
: KstMatrixDialog(parent, name, modal, fl) {
  Init();

  connect(_zVector, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));

  //signals within the dialog
  connect(_useMaximum, SIGNAL(clicked()),
          this, SLOT(updateFields()));

  setFixedHeight(height());

}

KstMatrixDialogI::~KstMatrixDialogI() {
  DP = 0L;
}

KstMatrixPtr KstMatrixDialogI::_getPtr(const QString &tagin) {
  KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
  return *matrices.findTag(tagin);
}

void KstMatrixDialogI::updateWindow() {
}

void KstMatrixDialogI::_fillFieldsForEdit() {
  DP->readLock();
  //fill in the tag name
  _tagName->setText(DP->tagName());

  //fill in the other parameters
  _nX->setValue(DP->xNumSteps());
  _nY->setValue(DP->yNumSteps());
  _minX->setText(QString::number(DP->minX()));
  _minY->setText(QString::number(DP->minY()));
  _xStep->setText(QString::number(DP->xStepSize()));
  _yStep->setText(QString::number(DP->yStepSize()));
  _useMaximum->setChecked(DP->useMaxX());

  //set the z vector
  _zVector->setSelection(DP->zVectorTag());

  updateFields();
  DP->readUnlock();
}

void KstMatrixDialogI::_fillFieldsForNew() {
  /* set tag name */
  _tagName->setText("<New_Matrix>");

  //fill some default values for the grid parameters
  _minX->setText("0");
  _minY->setText("1");
  _xStep->setText("1");
  _yStep->setText("1");
  _useMaximum->setChecked(true);
  updateFields();
}

void KstMatrixDialogI::update() {
   _zVector->update();
}

bool KstMatrixDialogI::new_I() {

  KstVectorList::Iterator vector_iter;
  //check the parameters
  double minXDouble, minYDouble, xStepDouble, yStepDouble;
  if (!checkParameters(vector_iter, minXDouble, minYDouble, xStepDouble, yStepDouble)) {
    return false;
  }
  (*vector_iter)->readLock();

  //create a unique name
  QString tag_name = KST::suggestMatrixName((*vector_iter)->tagName());
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    (*vector_iter)->readUnlock();
    return false;
  }

  KstMatrixPtr matrix = new KstMatrix(tag_name, *vector_iter,
                                      _nX->cleanText().toUInt(),
                                      _nY->cleanText().toUInt(),
                                      minXDouble,
                                      minYDouble,
                                      xStepDouble,
                                      yStepDouble,
                                      _useMaximum->isChecked());
  (*vector_iter)->readUnlock();
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(matrix.data());
  KST::dataObjectList.lock().writeUnlock();
  emit matrixCreated(KstMatrixPtr(matrix));
  matrix = 0L; // drop the reference
  emit modified();

  return true;
}

bool KstMatrixDialogI::edit_I() {
  KstVectorList::Iterator vector_iter;

  /* verify that the matrix name is unique */
  if (_tagName->text() != DP->tagName() && KST::dataTagNameNotUnique(_tagName->text())) {
    return false;
  }
  double minXDouble, minYDouble, xStepDouble, yStepDouble;
  if (!checkParameters(vector_iter, minXDouble, minYDouble, xStepDouble, yStepDouble)) {
    return false;
  }
  (*vector_iter)->readLock();
  DP->writeLock();
  DP->changeParameters(_tagName->text(), *vector_iter,
                                      _nX->cleanText().toUInt(),
                                      _nY->cleanText().toUInt(),
                                      minXDouble,
                                      minYDouble,
                                      xStepDouble,
                                      yStepDouble,
                                      _useMaximum->isChecked());
  DP->writeUnlock();
  (*vector_iter)->readUnlock();

  emit modified();
  return true;
}

void KstMatrixDialogI::updateFields() {
  _nX->setEnabled(!_useMaximum->isChecked());
}

bool KstMatrixDialogI::checkParameters(KstVectorList::Iterator &vector_iter,
                                      double &minXDouble,
                                      double &minYDouble,
                                      double &xStepDouble,
                                      double &yStepDouble) {

  if (_zVector->selectedVector().isEmpty()){
    KMessageBox::sorry(this, i18n("Matrix is a 2D grid created from vector", "New matrix not made: define vectors first."));
    return false;
  }

  //find the z vector
  KST::vectorList.lock().readLock();
  vector_iter = KST::vectorList.findTag(_zVector->selectedVector());
  if (vector_iter == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the zVector field in matrixDialog refers to "
              << "a non existant vector...." << endl;
  }
  (*vector_iter)->readLock();

  //basic check to see if parameters are consistent with vector
  if (!_useMaximum->isChecked() && (*vector_iter)->length() < _nX->cleanText().toInt() * _nY->cleanText().toInt()) {
    KMessageBox::sorry(this, i18n("The number of elements in the grid is greater than the number of elements in the selected vector.  Please ensure the grid dimensions are correct."));
    (*vector_iter)->readUnlock();
    KST::vectorList.lock().readUnlock();
    return false;
  }

    //check parameters
  bool ok1, ok2, ok3, ok4;
  minXDouble = _minX->text().toDouble(&ok1);
  minYDouble = _minY->text().toDouble(&ok2);
  xStepDouble = _xStep->text().toDouble(&ok3);
  yStepDouble = _yStep->text().toDouble(&ok4);
  if (!(ok1 && ok2 && ok3 && ok4)) {
    KMessageBox::sorry(this, i18n("One or more grid parameters have invalid values.  Please ensure only decimal values are entered."));
    (*vector_iter)->readUnlock();
    KST::vectorList.lock().readUnlock();
    return false;
  }
  if (xStepDouble <= 0 || yStepDouble <= 0) {
    KMessageBox::sorry(this, i18n("Invalid step size entered.  Please ensure the step sizes are positive."));
    (*vector_iter)->readUnlock();
    KST::vectorList.lock().readUnlock();
    return false;
  }
  (*vector_iter)->readUnlock();
  KST::vectorList.lock().readUnlock();
  return true;
}

#include "kstmatrixdialog_i.moc"
// vim: ts=2 sw=2 et
