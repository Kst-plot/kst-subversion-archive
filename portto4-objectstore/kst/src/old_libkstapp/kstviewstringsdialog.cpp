/***************************************************************************
                    kstviewstringsdialog.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2006 The University of Toronto
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

#include <q3header.h>
#include <q3boxlayout.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "kstdatacollection.h"
#include "kstviewstringsdialog.h"

KstViewStringsDialogI::KstViewStringsDialogI(QWidget* parent, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);
  listViewStrings = new KstStringListView(this, &KST::stringList);
  listViewStrings->setShowSortIndicator(false);
  listViewStrings->setSelectionMode(Q3ListView::NoSelection);
  searchWidget = new K3ListViewSearchLineWidget(listViewStrings, this);
  Q3BoxLayout *box = dynamic_cast<Q3BoxLayout*>(layout());
  if (box) {
    box->insertWidget(0, searchWidget);
    box->insertWidget(1, listViewStrings);

    languageChange();
  }

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
}


KstViewStringsDialogI::~KstViewStringsDialogI() {
}


bool KstViewStringsDialogI::hasContent() const {
  return !KST::stringList.isEmpty();
}

void KstViewStringsDialogI::updateViewStringsDialog() {
  listViewStrings->update();
  searchWidget->searchLine()->updateSearch();

  // use whole width
  int c0Width = listViewStrings->columnWidth(0);
  int c1Width = listViewStrings->columnWidth(1);
  int totalWidth = listViewStrings->header()->rect().width();
  c0Width = totalWidth * c0Width/(c0Width + c1Width);
  c1Width = totalWidth - c0Width;
  listViewStrings->setColumnWidth(0, c0Width);
  listViewStrings->setColumnWidth(1, c1Width);
}


void KstViewStringsDialogI::showViewStringsDialog() {
  updateViewStringsDialog();
  updateDefaults(0);
  show();
  raise();
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewStringsDialogI::languageChange() {
  setWindowTitle(i18n("View String Values"));
  listViewStrings->header()->setLabel(0, i18n("String"));
  listViewStrings->header()->setLabel(1, i18n("Value"));
}


void KstViewStringsDialogI::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewstringsdialog.moc"
// vim: ts=2 sw=2 et
