/**************************************************************************
        comboboxselection_i.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2005
    copyright            : (C) 2005 The University of British Columbia
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

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRegExp>
#include <QStringList>

#include "comboboxselection_i.h"

ComboBoxSelectionI::ComboBoxSelectionI(QWidget *parent, const char* name, bool modal, Qt::WindowFlags fl)
: QWidget(parent, fl) {
  setupUi((QDialog*)this);

  connect(OK, SIGNAL(clicked()), this, SLOT(ok()));
  connect(Cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(sort()));
  connect(_listBox, SIGNAL(selectionChanged()), this, SLOT(changed()));
  
  OK->setEnabled(false);
}


ComboBoxSelectionI::~ComboBoxSelectionI() {
}


void ComboBoxSelectionI::addString(const QString &str) {
  _strs.append(str);
}


void ComboBoxSelectionI::reset() {
  _listBox->clear();
}


void ComboBoxSelectionI::changed() {    
  QList<QListWidgetItem*> items; 
  
  items = _listBox->selectedItems();
  if (items.count() > 0) {
    OK->setEnabled(true);
  } else {
    OK->setEnabled(false);
  }
}


void ComboBoxSelectionI::sort() {
  QString search;
  int i;
  
  search = _lineEdit->text();
  if (search.isEmpty()) {
    search = "*";
  } else if (search.contains("*") == 0) {
    search.prepend("*");
    search.append("*");
  }

  QRegExp regexp(search, Qt::CaseInsensitive, QRegExp::Wildcard);

  OK->setEnabled(false);
  _listBox->clear();
  for (i=0; i<(int)_strs.count(); i++) {
    if (regexp.exactMatch(_strs[i])) {
      _listBox->insertItem(0, _strs[i]);
    }
  }
  _listBox->sortItems();
}


void ComboBoxSelectionI::ok() {
  QList<QListWidgetItem*> items; 

  items = _listBox->selectedItems();
  if (items.count() > 0) {
    _selected = items.first()->text();
  }
//  accept();
}


//#include "comboboxselection_i.moc"

