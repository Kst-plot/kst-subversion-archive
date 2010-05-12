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
#include "kstvector.h"
#include "kstrvector.h"
#include "kstsvector.h"
#include "vectorselector.h"

VectorSelector::VectorSelector(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  update();

  _newVector->setIcon(QIcon(":/kst_vectornew.png"));
  _editVector->setIcon(QIcon(":/kst_vectoredit"));

  _provideNoneVector = false;

  connect(_vector, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
  connect(_newVector, SIGNAL(clicked()), this, SLOT(createNewVector()));
  connect(_editVector, SIGNAL(clicked()), this, SLOT(editVector()));
  connect(this, SIGNAL(selectionChanged(QString)), this, SLOT(selectionWatcher(QString)));
}


VectorSelector::~VectorSelector() {
}


void VectorSelector::allowNewVectors( bool allowed ) {
  _newVector->setEnabled(allowed);
}


QString VectorSelector::selectedVector() {
  KstVectorPtr ptr = *KST::vectorList.findTag(_vector->currentText());
  if (!ptr || (_provideNoneVector && _vector->currentIndex() == 0)) {
    return QString::null;
  } else {
    return _vector->currentText();
  }
}


void VectorSelector::update() {
  KstVectorList::const_iterator i;
  QStringList vectors;
  QString prev;
  QString tag;
  bool found = false;
  int index;
/* xxx
  if (_vector->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(update()));

    return;
  }
*/
  blockSignals(true);

  prev = _vector->currentText();

  _vector->clear();
  if (_provideNoneVector) {
    _vector->insertItem(0, QObject::tr("<None>"));
  }

  KST::vectorList.lock().readLock();
  for (i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
    (*i)->readLock();
    if (!(*i)->isScalarList()) {
      tag = (*i)->tag().displayString();
      vectors.append(tag);
      if (!found && tag == prev) {
        found = true;
      }
    }
    (*i)->unlock();
  }
  KST::vectorList.lock().unlock();

  qSort(vectors);
  _vector->addItems(vectors);

  if (found) {
    index = _vector->findText(prev);
    if (index != -1) {
      _vector->setCurrentIndex(index);
    }
  }

  blockSignals(false);

  setEdit(_vector->currentText());
}


void VectorSelector::createNewVector() {
  KstDialogs::self()->newVectorDialog(this, SLOT(newVectorCreated(KstVectorPtr)), SLOT(setSelection(KstVectorPtr)), SLOT(update()));
}


void VectorSelector::selectionWatcher( const QString & tag ) {
  QString label = "[" + tag + "]";

  emit selectionChangedLabel(label);

  setEdit(tag);
}


void VectorSelector::setSelection( const QString & tag ) {
  if (tag.isEmpty()) {
    if (_provideNoneVector) {
      blockSignals(true);
      _vector->setCurrentIndex(0);
      blockSignals(false);

      _editVector->setEnabled(false);
    }
  } else {
    int index;
  
    blockSignals(true);
    index = _vector->findText(tag);
    if (index != -1) {
      _vector->setCurrentIndex(index);
    }
    blockSignals(false);

    setEdit(tag);
  }
}


void VectorSelector::newVectorCreated( KstVectorPtr v ) {
  QString name;

  v->readLock();
  name = v->tagName();
  v->unlock();
  v = 0L; // deref

  emit newVectorCreated(name);
}


void VectorSelector::setSelection( KstVectorPtr v ) {
  if (v) {
    v->readLock();
    setSelection(v->tag().tagString());
    v->unlock();
  } else if (_provideNoneVector) {
    setSelection(tr("<None>"));
  }
}


void VectorSelector::provideNoneVector( bool provide ) {
  if (provide != _provideNoneVector) {
    _provideNoneVector = provide;
    update();
  }
}


void VectorSelector::editVector() {
  KstDataObjectPtr pro;

  KST::vectorList.lock().readLock();
  KstVectorPtr vec = *KST::vectorList.findTag(_vector->currentText());
  KST::vectorList.lock().unlock();

  if (vec) {
    pro = kst_cast<KstDataObject>(vec->provider());
  }
  if (pro) {
    pro->readLock();
    pro->showDialog(false);
    pro->unlock();
  } else {
    KstDialogs::self()->showVectorDialog(_vector->currentText(), true);
  }
}


void VectorSelector::setEdit( const QString& tag ) {
  KstVectorList::iterator it;
  KstVectorPtr vec;
  KstRVectorPtr rvp;
  KstSVectorPtr svp;
  KstDataObjectPtr pro;

  if (!tag.isEmpty()) {
    KST::vectorList.lock().readLock();
    it = KST::vectorList.findTag(tag);
    if (it != KST::vectorList.end()) {
      vec = *KST::vectorList.findTag(tag);
    }
    KST::vectorList.lock().unlock();
  }

  if (vec) {
    pro = kst_cast<KstDataObject>(vec->provider());
  }

  if (!pro) {
    rvp = kst_cast<KstRVector>(vec);
    svp = kst_cast<KstSVector>(vec);
  }

  _editVector->setEnabled(rvp||svp||pro);
}
