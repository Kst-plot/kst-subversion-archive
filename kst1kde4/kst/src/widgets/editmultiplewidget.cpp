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

#include <QRegExp>

#include "editmultiplewidget.h"

EditMultipleWidget::EditMultipleWidget(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  _objectList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(_selectAllBut, SIGNAL(clicked()), this, SLOT(selectAllObjects()));
  connect(_selectNoneBut, SIGNAL(clicked()), _objectList, SLOT(clearSelection()));
  connect(_filterEdit, SIGNAL(textChanged(QString)), this, SLOT(applyFilter(QString)));
}

EditMultipleWidget::~EditMultipleWidget( ) {
}

void EditMultipleWidget::selectAllObjects()
{
  _objectList->selectAll();
}

void EditMultipleWidget::applyFilter(const QString& filter)
{
  QListWidgetItem *item;
  QRegExp re(filter, Qt::CaseInsensitive, QRegExp::Wildcard);
  uint count;
  uint i;

  _objectList->clearSelection();

  count = _objectList->count();
  for (i = 0; i < count; ++i) {
    item = _objectList->item(i);
    if (re.exactMatch(item->text())) {
      item->setSelected(true);
    }
  }
}
