/***************************************************************************
                    kstviewscalarsdialog_i.cpp  -  Part of KST
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

#include <klocale.h>

#include "kstdatacollection.h"
#include "kstviewscalarsdialog_i.h"

KstViewScalarsDialogI::KstViewScalarsDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl)
: KstViewScalarsDialog(parent, name, modal, fl) {
  tableScalars = new KstScalarTable(this, "tableScalars");
  tableScalars->setNumRows(0);
  tableScalars->setNumCols(2);
  tableScalars->setReadOnly(true);
  tableScalars->setSorting(false);
  tableScalars->setSelectionMode(QTable::Single);
  QBoxLayout *box = dynamic_cast<QBoxLayout*>(layout());
  if (box) {
    box->insertWidget(0, tableScalars);
    if (tableScalars->numCols() != 2) {
      for (; 0 < tableScalars->numCols(); ) {
        tableScalars->removeColumn(0);
      }
      tableScalars->insertColumns(0, 2);
    }

    tableScalars->setReadOnly(true);
    languageChange();
  }

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
}


KstViewScalarsDialogI::~KstViewScalarsDialogI() {
}


void KstViewScalarsDialogI::updateViewScalarsDialog() {
  KST::scalarList.lock().readLock();
  int needed = KST::scalarList.count();
  KST::scalarList.lock().unlock();

  tableScalars->setNumRows(needed);
  QRect rect = tableScalars->horizontalHeader()->rect();
  tableScalars->setColumnWidth(0, rect.width()/2);
  tableScalars->setColumnWidth(1, rect.width()/2);
}


void KstViewScalarsDialogI::showViewScalarsDialog() {
  updateViewScalarsDialog();
  updateDefaults(0);
  show();
  raise();
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewScalarsDialogI::languageChange() {
  setCaption(i18n("View Scalar Values"));
  tableScalars->horizontalHeader()->setLabel(0, i18n("Scalar"));
  tableScalars->horizontalHeader()->setLabel(1, i18n("Value"));
  KstViewScalarsDialog::languageChange();      
}


void KstViewScalarsDialogI::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewscalarsdialog_i.moc"
// vim: ts=2 sw=2 et
