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

#include <QTimer>

#include "stringeditor.h"
#include "stringselector.h"

StringSelector::StringSelector(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  update();

 _newString->setIcon(QIcon(":/kst_stringnew.png"));
 _editString->setIcon(QIcon(":/kst_stringedit.png"));

  connect(_selectString, SIGNAL(clicked()), this, SLOT(selectString()));
  connect(_newString, SIGNAL(clicked()), this, SLOT(createNewString()));
  connect(_editString, SIGNAL(clicked()), this, SLOT(editString()));
  connect(_string, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
  connect(this, SIGNAL(selectionChanged(const QString&)), this, SLOT(selectionWatcher(const QString&)));
}


StringSelector::~StringSelector() {
}


void StringSelector::allowNewStrings( bool allowed ) {
  _newString->setEnabled(allowed);
}


void StringSelector::update() {
  /* xxx if (_string->listBox()->isVisible()) {
    QTimer::singleShot(250, this, SLOT(update()));
    return;
  }*/

  blockSignals(true);
    
  KstStringList::Iterator i;
  QStringList strings;
  QString prev = _string->currentText();
  bool found = false;
    
  _string->clear();
    
	KST::stringList.lock().readLock();
  for (i = KST::stringList.begin(); i != KST::stringList.end(); ++i) {
		(*i)->readLock();
	  QString tag = (*i)->tag().displayString();
		strings << tag;
		(*i)->unlock();

    if (tag == prev) {
      found = true;
    }
  }
	KST::stringList.lock().unlock();
    
// xxx qHeapSort(strings);
  _string->insertItems(0, strings);

/* xxx 
  if (found) {
		if (_string->currentText() != prev) {
			_string->setCurrentText(prev);
		}
  }

	if (!_string->currentText().isNull()) {
		selectionWatcher(_string->currentText());
	}*/

    
  blockSignals(false);
}


void StringSelector::createNewString() {
  StringEditor *se = new StringEditor(this);
  se->setWindowTitle(tr("New String"));

  if (se) {
  	int rc = se->exec();
	  if (rc == QDialog::Accepted) {
          KstStringPtr s;
		  s = new KstString(KstObjectTag(se->_name->text(), KstObjectTag::globalTagContext), 0L, se->_value->text());

		  s->setOrphan(true);
		  s->setEditable(true);

		  emit newStringCreated();

		  update();
		  setSelection(s);
		  _editString->setEnabled(true);
	  }
  
	  delete se;
  }
}


void StringSelector::selectString() {
/* xxx
  ComboBoxSelectionI *selection = new ComboBoxSelectionI(this, "string selector");
  int i;
    
  selection->reset();
  for (i=0; i<_string->count(); i++) {
    selection->addString(_string->text(i));
  }
  selection->sort();
  int rc = selection->exec();
  if (rc == QDialog::Accepted && _string->currentText() != selection->selected()) {
    _string->setCurrentText(selection->selected());   
  }
    
  delete selection;*/

}


void StringSelector::editString() {
  StringEditor *se = new StringEditor(this);
  se->setWindowTitle(tr("Edit string"));
  if (se ) {    
    KstStringPtr pold = *KST::stringList.findTag(_string->currentText());
    if (pold && pold->editable()) { 
      se->_value->setText(pold->value());
      se->_name->setText(pold->tag().tagString()); 
      se->_value->selectAll();
      se->_value->setFocus();      
    }
      
    int rc = se->exec();
    if (rc == QDialog::Accepted) {
		  QString val = se->_value->text();
  
		  KstStringPtr p = *KST::stringList.findTag(se->_name->text());
		  if (p) {
			  p->setValue(val);
			  setSelection(p);
		  } else {
			  p = new KstString(KstObjectTag(se->_name->text(), KstObjectTag::globalTagContext), 0L, val);
  
			  p->setOrphan(true);
			  p->setEditable(true);
			  //emit newStringCreated();
			  update();
			  setSelection(p);
			  _editString->setEnabled(true);
		  }
	  }
      
    delete se;
  }
}


void StringSelector::selectionWatcher( const QString & tag ) {   
  KstStringPtr p;
  QString label = "["+tag+"]";
  bool editable = false;

// xxx emit selectionChangedLabel(label);

  KST::stringList.lock().readLock();
  p = *KST::stringList.findTag(tag);
  if (p && p->editable()) {
    editable = true;
  }
  KST::stringList.lock().unlock();
   
 _editString->setEnabled(editable);
}


void StringSelector::setSelection( const QString & tag ) {
  if (!tag.isEmpty()) {
    if (_string->currentText() != tag) {
      blockSignals(true);
// xxx   _string->setCurrentText(tag);
      selectionWatcher(tag);
      blockSignals(false);
    }
  }
}


void StringSelector::setSelection( KstStringPtr s ) {
  setSelection(s->tagName());
}


QString StringSelector::selectedString() {
  KstStringPtr ptr = *KST::stringList.findTag(_string->currentText());

  if (ptr) {
    return _string->currentText();
  } else {
    return QString::null;
  }
}


void StringSelector::allowDirectEntry( bool allowed ) {
  _string->setEditable(allowed);
}
