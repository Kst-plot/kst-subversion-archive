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

#include <QLayout>
#include <QPushButton>
#include <QTableWidget>

#include "kstmatrix.h"
#include "kstviewmatricesdialog.h"
#include "matrixselector.h"

KstViewMatricesDialog::KstViewMatricesDialog(QWidget* parent, const char* name,
                                             bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  _tableMatrices = new KstMatrixTable(this, "tableMatrices");
  _tableMatrices->setRowCount(0);
  _tableMatrices->setColumnCount(5);
  _tableMatrices->setSelectionMode(QAbstractItemView::SingleSelection);
  vboxLayout->insertWidget(1, _tableMatrices);

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
  QString matrix;

  matrixSelector->update();
  matrix = matrixSelector->selectedMatrix();
  _tableMatrices->setMatrix(matrix);
  updateViewMatricesDialog(matrix);
}


void KstViewMatricesDialog::updateViewMatricesDialog(const QString& matrixName) {
  KstMatrixPtr matrix;

  KST::matrixList.lock().readLock();
  matrix = *KST::matrixList.findTag(matrixName);
  KST::matrixList.lock().unlock();
  if (matrix) {
    bool updated = false;
    int xneeded = 0;
    int yneeded = 0;

    matrix->readLock();
    xneeded = matrix->xNumSteps();
    yneeded = matrix->yNumSteps();
    matrix->unlock();

    if (xneeded != _tableMatrices->columnCount()) {
      _tableMatrices->setColumnCount(xneeded);
      updated = true;
    }

    if (yneeded != _tableMatrices->rowCount()) {
      _tableMatrices->setRowCount(yneeded);
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
  KstMatrixPtr matrix;

  updateViewMatricesDialog();

  KST::matrixList.lock().readLock();
  matrix = *KST::matrixList.findTag(matrixName);
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


void KstViewMatricesDialog::languageChange() {
/* xxx
  setWindowTitle(QObject::tr("View Matrix Values"));
  KstViewMatricesDialog::languageChange();
*/
}


void KstViewMatricesDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewmatricesdialog.moc"
