/***************************************************************************
                               kstcombobox.cpp
                             -------------------
    begin                : 12/14/06
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

#include <QCompleter>
#include <QLineEdit>
#include <QStringList>

#include "kstcombobox.h"

KstComboBox::KstComboBox(QWidget *parent, const char *name)
  : QComboBox(parent), _trueRW(false) {

  commonConstructor();
}


KstComboBox::KstComboBox(bool rw, QWidget *parent, const char *name)
  : QComboBox(parent), _trueRW(rw) {
  setEditable(false);
  commonConstructor();
}


KstComboBox::~KstComboBox() {
}


void KstComboBox::setEditable(bool rw) {
  _trueRW = rw;
}


void KstComboBox::commonConstructor() {
  QComboBox::setEditable( true );

  if (!_trueRW) {
    //
    // if not truly read write then go into psuedo mode for read only...
    //

    setInsertPolicy( NoInsert );

    //
    // don't handle the edit's returnpressed in qcombobox.cpp,
    // but rather handle it here...
    //

// xxx    lineEdit()->disconnect( SIGNAL(returnPressed()), this, SLOT(returnPressed()) );
// xxx    connect( this, SIGNAL(returnPressed()), this, SLOT(validate()) );
  }
}


void KstComboBox::focusInEvent(QFocusEvent *event) {
  //
  // if the list of items changes programmatically while the combo has focus
  //  this will fail! Unfortunately I see no way to check whether the list of 
  //  items changes programmatically other than to provide my own input methods 
  //  or poll with a timer. Neither is a good idea IMO...
  //

  if (!_trueRW) {
    QStringList strings;
    QCompleter *completer;
    int i;

    for (i = 0; i < count(); ++i) {
      strings.append(itemText(i));
    }
    completer = new QCompleter(strings, this);
    setCompleter(completer);
  }

  QComboBox::focusInEvent(event);
}


void KstComboBox::focusOutEvent(QFocusEvent *event) {
  validate(false);

  QComboBox::focusOutEvent(event);
}


void KstComboBox::validate(bool rp) {
  if (!_trueRW) {
    int match = -1;
    int i;

    for (i = 0; i < count(); ++i) {
      match = currentText() == itemText(i) ? i : match;
    }

    if (match < 0 && count()) {
      lineEdit()->blockSignals(true);
      lineEdit()->setText( itemText(currentIndex()) );
      lineEdit()->blockSignals(false);
    } else if (match != currentIndex() || rp) {
      setCurrentIndex(match);

      emit activated(match);
      emit activated(itemText(match));
    }
  }
}
