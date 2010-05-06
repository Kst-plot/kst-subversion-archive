/***************************************************************************
                    kstviewscalarsdialog.cpp  -  Part of KST
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

#include "kstdatacollection.h"
#include "kstviewscalarsdialog.h"

KstViewScalarsDialog::KstViewScalarsDialog(QWidget* parent, const char* name,
                                           bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);

  _listViewScalars = new KstScalarListView(this, &KST::scalarList);
  _listViewScalars->setShowSortIndicator(false);
  _searchWidget = new KListViewSearchLineWidget(listViewScalars, this);
  QBoxLayout *box = dynamic_cast<QBoxLayout*>(layout());
  if (box) {
    box->insertWidget(0, searchWidget);
    box->insertWidget(1, listViewScalars);

    languageChange();
  }

  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
}


KstViewScalarsDialog::~KstViewScalarsDialog() {
}


bool KstViewScalarsDialog::hasContent() const {
  return !KST::scalarList.isEmpty();
}

void KstViewScalarsDialog::updateViewScalarsDialog() {
  _listViewScalars->update();
  if (_searchWidget) {
    _searchWidget->searchLine()->updateSearch(); 
  }

  int c0Width = _listViewScalars->columnWidth(0);
  int c1Width = _listViewScalars->columnWidth(1);
  int totalWidth = _listViewScalars->header()->rect().width();

  c0Width = totalWidth * c0Width/(c0Width + c1Width);
  c1Width = totalWidth - c0Width;

  _listViewScalars->setColumnWidth(0, c0Width);
  _listViewScalars->setColumnWidth(1, c1Width);
}


void KstViewScalarsDialog::showViewScalarsDialog() {
  updateViewScalarsDialog();
  updateDefaults(0);
  show();
  raise();
}


void KstViewScalarsDialog::languageChange() {
  setCaption(QObject::tr("View Scalar Values"));
  _listViewScalars->header()->setLabel(0, QObject::tr("Scalar"));
  _listViewScalars->header()->setLabel(1, QObject::tr("Value"));
  KstViewScalarsDialog::languageChange();
}


void KstViewScalarsDialog::updateDefaults(int index) {
  Q_UNUSED(index)
}

#include "kstviewscalarsdialog.moc"
