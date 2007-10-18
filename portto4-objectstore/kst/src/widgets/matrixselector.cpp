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

#include "objectstore.h"

namespace Kst {

MatrixSelector::MatrixSelector(QWidget *parent, ObjectStore *store)
  : QWidget(parent), _store(store) {

  setupUi(this);

  int size = style()->pixelMetric(QStyle::PM_SmallIconSize);

  _newMatrix->setIcon(QPixmap(":kst_matrixnew.png"));
  _editMatrix->setIcon(QPixmap(":kst_matrixedit.png"));

  _newMatrix->setFixedSize(size + 8, size + 8);
  _editMatrix->setFixedSize(size + 8, size + 8);
}


MatrixSelector::~MatrixSelector() {
}


void MatrixSelector::setObjectStore(ObjectStore *store) {
  _store = store;
}


MatrixPtr MatrixSelector::selectedMatrix() const {
  return 0;
}


void MatrixSelector::setSelectedMatrix(MatrixPtr selectedMatrix) {
  Q_UNUSED(selectedMatrix);
}

}

// vim: ts=2 sw=2 et
