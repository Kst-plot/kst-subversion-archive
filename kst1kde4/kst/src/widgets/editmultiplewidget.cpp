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
}

EditMultipleWidget::~EditMultipleWidget( ) {
}

void EditMultipleWidget::selectAllObjects()
{
  _objectList->clearSelection();
  _objectList->invertSelection();
}

void EditMultipleWidget::applyFilter(const QString& filter)
{
  QRegExp re(filter, Qt::CaseInsensitive, QRegExp::Wildcard);
  uint c;
  uint i;

  _objectList->clearSelection();

  c = _objectList->count();
  for (i = 0; i < c; ++i) {
    if (re.exactMatch(_objectList->text(i))) {
      _objectList->setSelected(i, true);
    }
  }
}
