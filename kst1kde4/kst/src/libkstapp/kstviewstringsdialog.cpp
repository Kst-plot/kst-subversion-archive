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

#include <QLayout>
#include <QPushButton>

#include "kstdatacollection.h"
#include "kstviewstringsdialog.h"

KstViewStringsDialog::KstViewStringsDialog(QWidget* parent, const char* name,
                                           bool modal, Qt::WindowFlags fl)
: QDialog(parent, name, modal, fl) {
  QBoxLayout *box;

  setupUi(this);

  listViewStrings = new KstStringListView(this, &KST::stringList);
  listViewStrings->setShowSortIndicator(false);
  listViewStrings->setSelectionMode(QListView::NoSelection);
  searchWidget = new KListViewSearchLineWidget(listViewStrings, this);

  box = dynamic_cast<QBoxLayout*>(layout());
  if (box) {
    box->insertWidget(0, searchWidget);
    box->insertWidget(1, listViewStrings);

    languageChange();
  }

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
}


KstViewStringsDialog::~KstViewStringsDialog() {
}


bool KstViewStringsDialog::hasContent() const {
  return !KST::stringList.isEmpty();
}

void KstViewStringsDialog::updateViewStringsDialog() {
  listViewStrings->update();
  searchWidget->searchLine()->updateSearch();

  //
  // use whole width...
  //

  int c0Width = listViewStrings->columnWidth(0);
  int c1Width = listViewStrings->columnWidth(1);
  int totalWidth = listViewStrings->header()->rect().width();

  c0Width = totalWidth * c0Width/(c0Width + c1Width);
  c1Width = totalWidth - c0Width;
  listViewStrings->setColumnWidth(0, c0Width);
  listViewStrings->setColumnWidth(1, c1Width);
}


void KstViewStringsDialog::showViewStringsDialog() {
  updateViewStringsDialog();
  updateDefaults(0);
  show();
  raise();
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewStringsDialog::languageChange() {
  setCaption(QObject::tr("View String Values"));
  listViewStrings->header()->setLabel(0, QObject::tr("String"));
  listViewStrings->header()->setLabel(1, QObject::tr("Value"));
  KstViewStringsDialog::languageChange();      
}


void KstViewStringsDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewstringsdialog.moc"
