/***************************************************************************
          variableditor.cpp  -  widget for editting lists of variables
                             -------------------
    begin                : Mon Jan 26 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "variableeditor.h"

#include <kdebug.h>

#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qobjectlist.h>


namespace {
class Helper : public QFrame {
  public:
    Helper(QWidget *parent, const QString& name, const QString& value)
      : QFrame(parent, name.latin1()) {
      QHBoxLayout *l = new QHBoxLayout(this);
      _name = new QLabel(this, (name + " label").latin1());
      _name->setText(name);
      _value = new QLineEdit(this, (name + " entry").latin1());
      _value->setText(value);

      l->addWidget(_name);
      l->addWidget(_value);

      l->setMargin(4);
      l->setSpacing(20);

      _name->setFocusProxy(_value);
    }

    virtual ~Helper() {}

    QLabel *_name;
    QLineEdit *_value;
};
}


VariableEditor::VariableEditor(QWidget *parent, const char *name)
: QScrollView(parent, name) {
  _layout = new QVBoxLayout(this);
  _layout->setMargin(0);
  _layout->setSpacing(0);

  _spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
  _layout->addItem(_spacer);

  setVScrollBarMode(QScrollView::Auto);
}


VariableEditor::~VariableEditor() {
}


bool VariableEditor::addVariable(const QString& var, const QString& val) {
  if (_variables.contains(var)) {
    return false;
  }

  Helper *h = new Helper(this, var, val);

  connect(h->_value, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));
  _layout->removeItem(_spacer);
  _layout->addWidget(h);
  _layout->addItem(_spacer);

  if (_variables.isEmpty()) {
    h->_value->setFocus();
  }

  _variables.append(var);

  return true;
}


bool VariableEditor::removeVariable(const QString& var) {
  if (!_variables.contains(var)) {
    return false;
  }

  QObject *o = child(var.latin1());

  _variables.remove(var);

  if (o && QString::fromLatin1(o->name()) == var) {
    delete o;
    return true;
  }

  return false;
}


QString VariableEditor::variable(const QString& var) const {
  const QObjectList *cl = children();
  QObjectListIterator it(*cl);
  const QObject *o;

  while ( (o = it.current()) != 0L) {
    if (QString::fromLatin1(o->name()) == var) {
      const Helper *h = dynamic_cast<const Helper*>(o);
      if (h) {
        return h->_value->text();
      }
    }
    ++it;
  }

  return QString::null;
}


void VariableEditor::changed() {
  const Helper *h = static_cast<const Helper*>(sender());
  emit variableModified(QString::fromLatin1(h->name()));
}

#include "variableeditor.moc"
// vim: ts=2 sw=2 et
