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

#include <qheader.h>
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
  listViewScalars = new KstScalarListView(this, &KST::scalarList);
  listViewScalars->setShowSortIndicator(false);
  listViewScalars->setSelectionMode(QListView::NoSelection);
  searchWidget = new KListViewSearchLineWidget(listViewScalars, this);
  QBoxLayout *box = dynamic_cast<QBoxLayout*>(layout());
  if (box) {
    box->insertWidget(0, searchWidget);
    box->insertWidget(1, listViewScalars);

    languageChange();
  }

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
}


KstViewScalarsDialogI::~KstViewScalarsDialogI() {
}


bool KstViewScalarsDialogI::hasContent() const {
  return !KST::scalarList.isEmpty();
}

void KstViewScalarsDialogI::updateViewScalarsDialog() {
  listViewScalars->update();
  searchWidget->searchLine()->updateSearch();

  // use whole width
  int c0Width = listViewScalars->columnWidth(0);
  int c1Width = listViewScalars->columnWidth(1);
  int totalWidth = listViewScalars->header()->rect().width();
  c0Width = totalWidth * c0Width/(c0Width + c1Width);
  c1Width = totalWidth - c0Width;
  listViewScalars->setColumnWidth(0, c0Width);
  listViewScalars->setColumnWidth(1, c1Width);
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
  listViewScalars->header()->setLabel(0, i18n("Scalar"));
  listViewScalars->header()->setLabel(1, i18n("Value"));
  KstViewScalarsDialog::languageChange();      
}


void KstViewScalarsDialogI::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewscalarsdialog_i.moc"
// vim: ts=2 sw=2 et
