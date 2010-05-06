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

  tableVectors = new KstVectorTable(this, "tableVectors");
  tableVectors->setRowCount(0);
  tableVectors->setColumnCount(2);;
  tableVectors->setSelectionMode(QAbstractItemView::SingleSelection);
  if (tableVectors->verticalHeader()) {
    tableVectors->verticalHeader()->hide();
  }

  vboxLayout->insertWidget(1, tableVectors);

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(vectorSelector, SIGNAL(selectionChanged(const QString&)), this, SLOT(vectorChanged(const QString&)));
  connect(vectorSelector, SIGNAL(newVectorCreated(const QString&)), this, SLOT(vectorChanged(const QString&)));
/* xxx
  if (tableVectors->columnCount() != 2) {
    for (; 0 < tableVectors->columnCount(); ) {
      tableVectors->removeColumn(0);
    }

    //
    // insert two columns...
    //

    tableVectors->insertColumn(0);
    tableVectors->insertColumn(0);
  }
*/

  languageChange();
}


KstViewVectorsDialog::~KstViewVectorsDialog() {
}


bool KstViewVectorsDialog::hasContent() const {
  return !KST::vectorList.isEmpty();
}


void KstViewVectorsDialog::updateViewVectorsDialog() {
  QString vector;

  vectorSelector->update();
  vector = vectorSelector->selectedVector();
  tableVectors->setVector(vector);
  updateViewVectorsDialog(vector);
}


void KstViewVectorsDialog::updateViewVectorsDialog(const QString& vectorName) {
  KstVectorPtr vector;
  QRect rect;
  int needed = 0;

  KST::vectorList.lock().readLock();
  vector = *KST::vectorList.findTag(vectorName);
  KST::vectorList.lock().unlock();
  if (vector) {
    vector->readLock();
    needed = vector->length();
    vector->unlock();
  }

  if (needed != tableVectors->rowCount()) {
    tableVectors->setRowCount(needed);
  }
  rect = tableVectors->horizontalHeader()->rect();
  tableVectors->setColumnWidth(0, rect.width() / 5);
  tableVectors->setColumnWidth(1, rect.width() - (rect.width() / 5));
}


void KstViewVectorsDialog::showViewVectorsDialog() {
  updateViewVectorsDialog();
  updateDefaults(0);
  show();
  raise();
}


void KstViewVectorsDialog::showViewVectorsDialog(const QString &vectorName) {
  KstVectorPtr vector;

  updateViewVectorsDialog();

  KST::vectorList.lock().readLock();
  vector = *KST::vectorList.findTag(vectorName);
  KST::vectorList.lock().unlock();
  if (vector) {
    vectorSelector->setSelection(vector);
    updateViewVectorsDialog();
    show();
    raise();
  }
}


void KstViewVectorsDialog::vectorChanged(const QString& vector) {
  updateViewVectorsDialog(vector);
  tableVectors->setVector(vector);
  tableVectors->update();
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
