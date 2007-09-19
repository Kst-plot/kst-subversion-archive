/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void StringSelector::init()
{
    update();
    _newString->setPixmap(BarIcon("kst_stringnew"));
    _editString->setPixmap(BarIcon("kst_stringedit"));
    connect(_selectString, SIGNAL(clicked()), this, SLOT(selectString()));
    connect(_newString, SIGNAL(clicked()), this, SLOT(createNewString()));
    connect(_editString, SIGNAL(clicked()), this, SLOT(editString()));
    connect(_string, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
    connect(this, SIGNAL(selectionChanged(const QString&)), this, SLOT(selectionWatcher(const QString&)));
}

void StringSelector::allowNewStrings( bool allowed )
{
    _newString->setEnabled(allowed);
}

void StringSelector::update()
{
    if (_string->listBox()->isVisible()) {
        QTimer::singleShot(250, this, SLOT(update()));
        return;
    }

    blockSignals(true);
    
    QString prev = _string->currentText();
    bool found = false;
    QStringList strings;
    
    _string->clear();
    
	KST::stringList.lock().readLock();
    for (KstStringList::Iterator i = KST::stringList.begin(); i != KST::stringList.end(); ++i) {
		(*i)->readLock();
	    QString tag = (*i)->tag().displayString();
		strings << tag;
		(*i)->unlock();

        if (tag == prev) {
            found = true;
        }
    }
	KST::stringList.lock().unlock();
    
    qHeapSort(strings);
    _string->insertStringList(strings);
    if (found) {
		if (_string->currentText() != prev) {
			_string->setCurrentText(prev);
		}
    }

	if (!_string->currentText().isNull()) {
		selectionWatcher(_string->currentText());
	}
    
    blockSignals(false);
}

void StringSelector::createNewString()
{
    StringEditor *se = new StringEditor(this, "string editor");
    
	int rc = se->exec();
	if (rc == QDialog::Accepted) {
		KstStringPtr s = new KstString(KstObjectTag(se->_name->text(), KstObjectTag::globalTagContext), 0L, se->_value->text());

		s->setOrphan(true);
		s->setEditable(true);
		emit newStringCreated();
		update();
		setSelection(s);
		_editString->setEnabled(true);
	}

	delete se;
}

void StringSelector::selectString()
{
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
    
    delete selection;
}

void StringSelector::editString()
{
    StringEditor *se = new StringEditor(this, "string editor");
    
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
			emit newStringCreated();
			update();
			setSelection(p);
			_editString->setEnabled(true);
		}
	}
    
    delete se;
}

void StringSelector::selectionWatcher( const QString & tag )
{   
    bool editable = false;

    QString label = "["+tag+"]";
    emit selectionChangedLabel(label);
    KST::stringList.lock().readLock();
    KstStringPtr p = *KST::stringList.findTag(tag);
    if (p && p->editable()) {
      editable = true;
    }
    KST::stringList.lock().unlock();
    _editString->setEnabled(editable);
}

void StringSelector::setSelection( const QString & tag )
{
    if (tag.isEmpty() || _string->currentText() == tag) {
        return;
    }
    blockSignals(true);
    _string->setCurrentText(tag);
    selectionWatcher(tag);
    blockSignals(false);
}

void StringSelector::setSelection( KstStringPtr s )
{
    setSelection(s->tagName());
}

QString StringSelector::selectedString()
{
    KstStringPtr ptr = *KST::stringList.findTag(_string->currentText());
    if (ptr)
        return _string->currentText();
    else
        return QString::null;
}

void StringSelector::allowDirectEntry( bool allowed )
{
    _string->setEditable(allowed);
}
