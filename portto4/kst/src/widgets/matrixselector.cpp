/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "matrixselector.h"

#include "dialoglauncher.h"
#include "datacollection.h"

namespace Kst {

MatrixSelector::MatrixSelector(QWidget *parent)
  : QWidget(parent) {

  setupUi(this);

  int size = style()->pixelMetric(QStyle::PM_SmallIconSize);

  _newMatrix->setIcon(QPixmap(":kst_matrixnew.png"));
  _editMatrix->setIcon(QPixmap(":kst_matrixedit.png"));

  _newMatrix->setFixedSize(size + 8, size + 8);
  _editMatrix->setFixedSize(size + 8, size + 8);

  fillMatrices();

  connect(_newMatrix, SIGNAL(pressed()), this, SLOT(newMatrix()));
  connect(_editMatrix, SIGNAL(pressed()), this, SLOT(editMatrix()));
}


MatrixSelector::~MatrixSelector() {
}


MatrixPtr MatrixSelector::selectedMatrix() const {
  return qVariantValue<Matrix*>(_matrix->itemData(_matrix->currentIndex()));
}


void MatrixSelector::setSelectedMatrix(MatrixPtr selectedMatrix) {
  int i = _matrix->findData(qVariantFromValue(selectedMatrix.data()));
  Q_ASSERT(i != -1);
  _matrix->setCurrentIndex(i);
}


void MatrixSelector::newMatrix() {
  DialogLauncher::self()->showMatrixDialog();
  fillMatrices();
}


void MatrixSelector::editMatrix() {
  DialogLauncher::self()->showMatrixDialog(ObjectPtr(selectedMatrix()));
}

void MatrixSelector::fillMatrices() {
  QHash<QString, MatrixPtr> matrices;

  matrixList.lock().readLock();

  MatrixList::ConstIterator it = matrixList.begin();
  for (; it != matrixList.end(); ++it) {
    MatrixPtr matrix = (*it);

    matrix->readLock();
    matrices.insert(matrix->tag().displayString(), matrix);
    matrix->unlock();
  }

  matrixList.lock().unlock();

  QStringList list = matrices.keys();

  qSort(list);

  MatrixPtr current = selectedMatrix();

  _matrix->clear();
  foreach (QString string, list) {
    MatrixPtr m = matrices.value(string);
    _matrix->addItem(string, qVariantFromValue(m.data()));
  }

  if (current)
    setSelectedMatrix(current);
}


}

// vim: ts=2 sw=2 et
