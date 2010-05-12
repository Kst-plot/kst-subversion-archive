/***************************************************************************
                    kstviewvectorsdialog.cpp  -  Part of KST
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

#include <QLayout>
#include <QPushButton>
#include <QTableWidget>

#include "kstviewvectorsdialog.h"
#include "vectorselector.h"

KstViewVectorsDialog::KstViewVectorsDialog(QWidget* parent, const char* name,
                                             bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  _model = 0L;

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(vectorSelector, SIGNAL(selectionChanged(const QString&)), this, SLOT(vectorChanged(const QString&)));
  connect(vectorSelector, SIGNAL(newVectorCreated(const QString&)), this, SLOT(vectorChanged(const QString&)));

  languageChange();
}


KstViewVectorsDialog::~KstViewVectorsDialog() {
  delete _model;
  _model = 0L;
}


bool KstViewVectorsDialog::hasContent() const {
  return !KST::vectorList.isEmpty();
}


void KstViewVectorsDialog::updateViewVectorsDialog() {
  vectorSelector->update();
  vectorChanged(QString());
}


void KstViewVectorsDialog::updateViewVectorsDialog(const QString& vectorName) {
  KstVectorPtr vector;

  KST::vectorList.lock().readLock();
  vector = *KST::vectorList.findTag(vectorName);
  KST::vectorList.lock().unlock();

  if (vector) {
    vectorSelector->setSelection(vector);
    vectorChanged(vectorName);
  }
}


void KstViewVectorsDialog::showViewVectorsDialog() {
  updateViewVectorsDialog();
  updateDefaults(0);
  show();
  raise();
}


void KstViewVectorsDialog::showViewVectorsDialog(const QString &vectorName) {
  KstVectorPtr vector;

  KST::vectorList.lock().readLock();
  vector = *KST::vectorList.findTag(vectorName);
  KST::vectorList.lock().unlock();

  if (vector) {
    vectorSelector->setSelection(vector);
    vectorChanged(vectorName);
    show();
    raise();
  }
}


void KstViewVectorsDialog::vectorChanged(const QString& vectorName) {
  KstVectorPtr vector;
  KstVectorList::iterator it;

  if (_model) {
    delete _model;
    _model = 0L;
  }

  KST::vectorList.lock().readLock();
  it = KST::vectorList.findTag(vectorName);
  if (it != KST::vectorList.end()) {
    vector = *it;
  }
  KST::vectorList.lock().unlock();

  if (vector) {
    _model = new ModelVector(vector);
    _table->setModel(_model);
  }
}


void KstViewVectorsDialog::languageChange() {
/* xxx
  setWindowTitle(QObject::tr("View Vector Values"));
  tableVectors->horizontalHeaderItem(0)->setText(QObject::tr("Index"));
  tableVectors->horizontalHeaderItem(1)->setText(QObject::tr("Value"));
  KstViewVectorsDialog::languageChange();
*/
}


void KstViewVectorsDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewvectorsdialog.moc"
