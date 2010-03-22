/***************************************************************************
                    kstviewmatricesdialog.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2005 The University of British Columbia
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

// includes files for Qt
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtable.h>

// include files for KDE
#include <klocale.h>

// application specific includes
#include "kstmatrix.h"
#include "kstviewmatricesdialog.h"
#include "matrixselector.h"

KstViewMatricesDialog::KstViewMatricesDialog(QWidget* parent,
                                             const char* name,
                                             bool modal,
                                             WFlags fl)
: QDialog(parent, name, modal, fl) {
  setupUi(this);
  _tableMatrices = new KstMatrixTable(this, "tableMatrices");
  _tableMatrices->setNumRows(0);
  _tableMatrices->setNumCols(5);
  _tableMatrices->setReadOnly(true);
  _tableMatrices->setSorting(false);
  _tableMatrices->setSelectionMode(QTable::Single);
  layout2->insertWidget(1, _tableMatrices);

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(matrixSelector, SIGNAL(selectionChanged(const QString&)), this, SLOT(matrixChanged(const QString&)));
  connect(matrixSelector, SIGNAL(newMatrixCreated(const QString&)), this, SLOT(matrixChanged(const QString&)));

  languageChange();
}


KstViewMatricesDialog::~KstViewMatricesDialog() {
}


bool KstViewMatricesDialog::hasContent() const {
  return !KST::matrixList.isEmpty();
}


void KstViewMatricesDialog::updateViewMatricesDialog() {
  matrixSelector->update();
  QString matrix = matrixSelector->selectedMatrix();
  _tableMatrices->setMatrix(matrix);
  updateViewMatricesDialog(matrix);
}


void KstViewMatricesDialog::updateViewMatricesDialog(const QString& matrixName) {
  KST::matrixList.lock().readLock();
  KstMatrixPtr matrix = *KST::matrixList.findTag(matrixName);
  KST::matrixList.lock().unlock();
  if (matrix) {
    bool updated = false;
    int xneeded = 0;
    int yneeded = 0;

    matrix->readLock();
    xneeded = matrix->xNumSteps();
    yneeded = matrix->yNumSteps();
    matrix->unlock();

    if (xneeded != _tableMatrices->numCols()) {
      _tableMatrices->setNumCols(xneeded);
      updated = true;
    }

    if (yneeded != _tableMatrices->numRows()) {
      _tableMatrices->setNumRows(yneeded);
      updated = true;
    }

    if (!updated) {
      //
      // following two lines appear to be necessary to ensure a full update...
      //
      _tableMatrices->hide();
      _tableMatrices->show();

      _tableMatrices->update();
    }
  }
}


void KstViewMatricesDialog::showViewMatricesDialog() {
  updateViewMatricesDialog();
  updateDefaults(0);
  show();
  raise();
}


void KstViewMatricesDialog::showViewMatricesDialog(const QString &matrixName) {
  updateViewMatricesDialog();

  KST::matrixList.lock().readLock();
  KstMatrixPtr matrix = *KST::matrixList.findTag(matrixName);
  KST::matrixList.lock().unlock();
  if (matrix) {
    matrixSelector->setSelection(matrix);
    updateViewMatricesDialog();
    show();
    raise();
  }
}


void KstViewMatricesDialog::matrixChanged(const QString& matrix) {
  updateViewMatricesDialog(matrix);
  _tableMatrices->setMatrix(matrix);

  //
  // following two lines appear to be necessary to ensure a full update...
  //
  _tableMatrices->hide();
  _tableMatrices->show();

  _tableMatrices->update();
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewMatricesDialog::languageChange() {
  setCaption(i18n("View Matrix Values"));
  KstViewMatricesDialog::languageChange();
}


void KstViewMatricesDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewmatricesdialog.moc"
