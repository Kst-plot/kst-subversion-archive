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

#include <qlayout.h>
#include <qpushbutton.h>
#include <qtable.h>

#include <klocale.h>

#include "kstviewvectorsdialog.h"
#include "vectorselector.h"

KstViewVectorsDialog::KstViewVectorsDialog(QWidget* parent,
                                             const char* name,
                                             bool modal,
                                             WFlags fl)
: QDialog(parent, name, modal, fl) {
  setupUi(this);
  tableVectors = new KstVectorTable(this, "tableVectors");
  tableVectors->setNumRows(0);
  tableVectors->setNumCols(2);
  tableVectors->setReadOnly(true);
  tableVectors->setSorting(false);
  tableVectors->setLeftMargin(0);
  tableVectors->setSelectionMode(QTable::Single);
  if (tableVectors->verticalHeader()) {
    tableVectors->verticalHeader()->hide();
  }
  layout2->insertWidget(1, tableVectors);

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(vectorSelector, SIGNAL(selectionChanged(const QString&)), this, SLOT(vectorChanged(const QString&)));
  connect(vectorSelector, SIGNAL(newVectorCreated(const QString&)), this, SLOT(vectorChanged(const QString&)));

  if (tableVectors->numCols() != 2) {
    for (; 0 < tableVectors->numCols(); ) {
      tableVectors->removeColumn(0);
    }
    tableVectors->insertColumns(0, 2);
  }

  tableVectors->setReadOnly(true);
  languageChange();
}


KstViewVectorsDialog::~KstViewVectorsDialog() {
}


bool KstViewVectorsDialog::hasContent() const {
  return !KST::vectorList.isEmpty();
}


void KstViewVectorsDialog::updateViewVectorsDialog() {
  vectorSelector->update();
  QString vector = vectorSelector->selectedVector();
  tableVectors->setVector(vector);
  updateViewVectorsDialog(vector);
}


void KstViewVectorsDialog::updateViewVectorsDialog(const QString& vectorName) {
  int needed = 0;
  KST::vectorList.lock().readLock();
  KstVectorPtr vector = *KST::vectorList.findTag(vectorName);
  KST::vectorList.lock().unlock();
  if (vector) {
    vector->readLock();
    needed = vector->length();
    vector->unlock();
  }

  if (needed != tableVectors->numRows()) {
    tableVectors->setNumRows(needed);
  }
  QRect rect = tableVectors->horizontalHeader()->rect();
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
  updateViewVectorsDialog();

  KST::vectorList.lock().readLock();
  KstVectorPtr vector = *KST::vectorList.findTag(vectorName);
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


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewVectorsDialog::languageChange() {
  setCaption(i18n("View Vector Values"));
  tableVectors->horizontalHeader()->setLabel(0, i18n("Index"));
  tableVectors->horizontalHeader()->setLabel(1, i18n("Value"));
  KstViewVectorsDialog::languageChange();
}


void KstViewVectorsDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewvectorsdialog.moc"
