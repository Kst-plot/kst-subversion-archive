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
#include "vectorselector.h"

VectorSelector::VectorSelector(QWidget *parent) : QWidget(parent) {
  setupUi(this);
  update();
// xxx  _newVector->setPixmap(BarIcon("kst_vectornew"));
// xxx  _editVector->setPixmap(BarIcon("kst_vectoredit"));
// xxx  connect(_selectVector, SIGNAL(clicked()), this, SLOT(selectVector()));
  connect(_newVector, SIGNAL(clicked()), this, SLOT(createNewVector()));
  connect(_editVector, SIGNAL(clicked()), this, SLOT(editVector()));
  connect(_vector, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
  connect(this, SIGNAL(selectionChanged(const QString&)), this, SLOT(selectionWatcher(const QString&)));
}

VectorSelector::~VectorSelector()
{
}

void VectorSelector::allowNewVectors( bool allowed )
{
  _newVector->setEnabled(allowed);
}


QString VectorSelector::selectedVector()
{
  /* xxx
  KstVectorPtr ptr = *KST::vectorList.findTag(_vector->currentText());
  if (!ptr || (_provideNoneVector && _vector->currentItem() == 0)) {
    return QString::null;
  } else {
    return _vector->currentText();
  } */
}


void VectorSelector::update()
{
  /* xxx if (_vector->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(update()));

    return;
  } */

  blockSignals(true);

  QString prev = _vector->currentText();
  bool found = false;

  _vector->clear();
/* xxx   if (_provideNoneVector) {
    _vector->insertItem(tr("<None>"));
  } */

  QStringList vectors;

  KST::vectorList.lock().readLock();
  for (KstVectorList::ConstIterator i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
    (*i)->readLock();
    if (!(*i)->isScalarList()){
      QString tag = (*i)->tag().displayString();
      vectors << tag;
      if (!found && tag == prev) {
        found = true;
      }
    }
    (*i)->unlock();
  }

  KST::vectorList.lock().unlock();

// xxx  qHeapSort(vectors);
  _vector->insertItems(0,vectors);
/* xxx  if (found) {
    _vector->setCurrentText(prev);
  }*/

  blockSignals(false);

// xxx  setEdit(_vector->currentText());
}


void VectorSelector::createNewVector()
{
  // xxx KstDialogs::self()->newVectorDialog(this, SLOT(newVectorCreated(KstVectorPtr)), SLOT(setSelection(KstVectorPtr)), SLOT(update()));
}


void VectorSelector::selectionWatcher( const QString & tag )
{   
  KstVectorPtr p;
  QString label = "["+tag+"]";
  bool editable = false;

// xxx emit selectionChangedLabel(label);

  KST::vectorList.lock().readLock();
  p = *KST::vectorList.findTag(tag);
  if (p && p->editable()) {
    editable = true;
  }
  KST::vectorList.lock().unlock();
   
 _editVector->setEnabled(editable);
}


void VectorSelector::setSelection( const QString & tag )
{
  if (!tag.isEmpty()) {
    if (_vector->currentText() != tag) {
      blockSignals(true);
// xxx   _string->setCurrentText(tag);
      selectionWatcher(tag);
      blockSignals(false);
    }
  }
}

/* xxx
void VectorSelector::newVectorCreated( KstVectorPtr v )
{
  v->readLock();
  QString name = v->tagName();
  v->unlock();
  v = 0L; // deref
// xxx  emit newVectorCreated(name);
}
*/

void VectorSelector::setSelection( KstVectorPtr v )
{
  setSelection(v->tagName());
}

/* xxx
void VectorSelector::provideNoneVector( bool provide )
{
  if (provide != _provideNoneVector) {
    _provideNoneVector = provide;
    update();
  }
}
*/

void VectorSelector::editVector()
{
  KST::vectorList.lock().readLock();
  KstVectorPtr vec = *KST::vectorList.findTag(_vector->currentText());
  KST::vectorList.lock().unlock();
  KstDataObjectPtr pro;
  pro = 0L;
  if (vec) {
    pro = kst_cast<KstDataObject>(vec->provider());
  }
  if (pro) {
    pro->readLock();
    pro->showDialog(false);
    pro->unlock();
  } else {
// xxx    KstDialogs::self()->showVectorDialog(_vector->currentText(), true);
  }
}

/* xxx
void VectorSelector::setEdit( const QString& tag )
{
  KST::vectorList.lock().readLock();
  KstVectorPtr vec = *KST::vectorList.findTag(tag);
  KST::vectorList.lock().unlock();
  KstRVectorPtr rvp = kst_cast<KstRVector>(vec);
  KstSVectorPtr svp = kst_cast<KstSVector>(vec);
  KstDataObjectPtr pro = 0L;
  if (vec) {
    pro = kst_cast<KstDataObject>(vec->provider());
  }
  _editVector->setEnabled(rvp||svp||pro);
}
*/
