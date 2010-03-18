/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialoglauncher.h"
#include "kstdataobject.h"
#include "kstmatrix.h"
#include "matrixselector.h"

MatrixSelector::MatrixSelector(QWidget *parent) : QWidget(parent) {
  setupUi(this);

// xxx  _newMatrix->setPixmap(BarIcon("kst_matrixnew"));
// xxx  _editMatrix->setPixmap(BarIcon("kst_matrixedit"));

  _provideNoneMatrix = false;

  update();

  connect(_matrix, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
}

MatrixSelector::~MatrixSelector()
{
}

void MatrixSelector::allowNewMatrices( bool allowed )
{
  _newMatrix->setEnabled(allowed);
}


QString MatrixSelector::selectedMatrix()
{
  KstMatrixPtr ptr = *KST::matrixList.findTag(_matrix->currentText());
  if (!ptr || (_provideNoneMatrix && _matrix->currentIndex() == 0)) {
    return QString::null;
  } else {
    return _matrix->currentText();
  }
}


void MatrixSelector::update()
{
/* xxx
  if (_matrix->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(update()));

    return;
  }
*/
  blockSignals(true);

  QString prev = _matrix->currentText();
  bool found = false;
  int index;

  _matrix->clear();
  if (_provideNoneMatrix) {
    _matrix->insertItem(0, "<None>");
  }

  KstMatrixList matrices = KST::matrixList.list();

  KST::matrixList.lock().readLock();

  for (KstMatrixList::ConstIterator i = matrices.begin(); i != matrices.end(); ++i) {
    (*i)->readLock();
    QString tag = (*i)->tag().displayString();
    (*i)->unlock();
    _matrix->addItem(tag);
    if (!found && tag == prev) {
      found = true;
    }
  }

  KST::matrixList.lock().unlock();
  if (found) {
    index = _matrix->findText(prev);
    if (index != -1) {
      _matrix->setCurrentIndex(index);
    }
  }

  blockSignals(false);

  setEdit(_matrix->currentText());
}

void MatrixSelector::createNewMatrix()
{
// xxx  KstDialogs::self()->newMatrixDialog(this, SLOT(newMatrixCreated(KstMatrixPtr)), SLOT(setSelection(KstMatrixPtr)), SLOT(update()));
}

void MatrixSelector::selectionWatcher( const QString & tag )
{
  QString label = "[" + tag + "]";
// xxx  emit selectionChangedLabel(label);
  setEdit(tag);
}

void MatrixSelector::setSelection( const QString & tag )
{
  if (tag.isEmpty()) {
    if (_provideNoneMatrix) {
      blockSignals(true);
      _matrix->setCurrentIndex(0);
      blockSignals(false);
      _editMatrix->setEnabled(false);
    }
    return;
  } else {
    int index;

    blockSignals(true);
    index = _matrix->findText(tag);
    if (index != -1) {
      _matrix->setCurrentIndex(index);
    }
    blockSignals(false);

    setEdit(tag);
  }
}

void MatrixSelector::newMatrixCreated( KstMatrixPtr v )
{
  v->readLock();
  QString name = v->tagName();
  v->unlock();
  v = 0L; // deref
// xxx  emit newMatrixCreated(name);
}

void MatrixSelector::setSelection( KstMatrixPtr v )
{
  v->readLock();
  setSelection(v->tagName());
  v->unlock();
}

void MatrixSelector::provideNoneMatrix( bool provide )
{
  if (provide != _provideNoneMatrix) {
    _provideNoneMatrix = provide;
    update();
  }
}

void MatrixSelector::editMatrix()
{
  KstDataObjectPtr pro;
  KstMatrixPtr mat;
  
  KST::matrixList.lock().readLock();
  mat = *KST::matrixList.findTag(_matrix->currentText());
  KST::matrixList.lock().unlock();

  if (mat) {
    pro = kst_cast<KstDataObject>(mat->provider());
  }

  if (pro) {
    pro->readLock();
    pro->showDialog(false);
    pro->unlock();
  } else {
// xxx    KstDialogs::self()->showMatrixDialog(_matrix->currentText(), true);
  }
}

void MatrixSelector::setEdit(const QString& tag)
{
  KST::matrixList.lock().readLock();
  _editMatrix->setEnabled(KST::matrixList.findTag(tag) != KST::matrixList.end());
  KST::matrixList.lock().unlock();
}