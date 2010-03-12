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
#include "kstdataobject.h"
#include "matrixselector.h"
MatrixSelector::MatrixSelector(QWidget *parent) : QWidget(parent) {
  setupUi(this);
  update();
// xxx  _newMatrix->setPixmap(BarIcon("kst_matrixnew"));
// xxx  _editMatrix->setPixmap(BarIcon("kst_matrixedit"));
// xxx  _provideNoneMatrix = false;
  update();

// xxx  connect(_selectMatrix, SIGNAL(clicked()), this, SLOT(selectMatrix()));
  connect(_newMatrix, SIGNAL(clicked()), this, SLOT(createNewMatrix()));
  connect(_editMatrix, SIGNAL(clicked()), this, SLOT(editMatrix()));
  connect(_matrix, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
  connect(this, SIGNAL(selectionChanged(const QString&)), this, SLOT(selectionWatcher(const QString&)));
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

  if (ptr) {
    return _matrix->currentText();
  } else {
    return QString::null;
  }
}

void MatrixSelector::update()
{
/* xxx  if (_matrix->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(update()));

    return;
  } */

  blockSignals(true);

  QString prev = _matrix->currentText();
  bool found = false;

  _matrix->clear();
/* xxx  if (_provideNoneMatrix) {
    _matrix->insertItem("<None>");
  } */

  KstMatrixList matrices = KST::matrixList.list();

  KST::matrixList.lock().readLock();

  for (KstMatrixList::ConstIterator i = matrices.begin(); i != matrices.end(); ++i) {
    (*i)->readLock();
    QString tag = (*i)->tag().displayString();
    (*i)->unlock();
// xxx    _matrix->insertItem(tag);
    if (!found && tag == prev) {
      found = true;
    }
  }

  KST::matrixList.lock().unlock();
  if (found) {
// xxx    _matrix->setCurrentText(prev);
  }

  blockSignals(false);

// xxx  setEdit(_matrix->currentText());
}





void MatrixSelector::createNewMatrix()
{
// xxx  KstDialogs::self()->newMatrixDialog(this, SLOT(newMatrixCreated(KstMatrixPtr)), SLOT(setSelection(KstMatrixPtr)), SLOT(update()));
}


void MatrixSelector::selectionWatcher( const QString & tag )
{
  bool editable = false;

  QString label = "["+tag+"]";
// xxx  emit selectionChangedLabel(label);
  KST::matrixList.lock().readLock();
  KstMatrixPtr p = *KST::matrixList.findTag(tag);
  if (p && p->editable()) {
    editable = true;
  }
  KST::matrixList.lock().unlock();
  _editMatrix->setEnabled(editable);
}


void MatrixSelector::setSelection( const QString & tag )
{
  if (!tag.isEmpty()) {
    blockSignals(true);
// xxx    _matrix->setCurrentText(tag);
    selectionWatcher(tag);
    blockSignals(false);
  }
}
/* xxx
void MatrixSelector::newMatrixCreated( KstMatrixPtr v )
{
  v->readLock();
  QString name = v->tagName();
  v->unlock();
  v = 0L; // deref
  emit newMatrixCreated(name);
}
*/

void MatrixSelector::setSelection( KstMatrixPtr v )
{
  v->readLock();
  setSelection(v->tagName());
  v->unlock();
}

/* xxx
void MatrixSelector::provideNoneMatrix( bool provide )
{
  if (provide != _provideNoneMatrix) {
    _provideNoneMatrix = provide;
    update();
  }
}
*/

void MatrixSelector::editMatrix()
{
  KST::matrixList.lock().readLock();
  KstMatrixPtr mat = *KST::matrixList.findTag(_matrix->currentText());
  KST::matrixList.lock().unlock();
  KstDataObjectPtr pro;
  pro = 0L;
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

/* xxx
void MatrixSelector::setEdit(const QString& tag)
{
  KST::matrixList.lock().readLock();
  _editMatrix->setEnabled(KST::matrixList.findTag(tag) != KST::matrixList.end());
  KST::matrixList.lock().unlock();
}
*/
