/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


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
    _string->clear();

	KST::stringList.lock().readLock();
    for (KstStringList::Iterator i = KST::stringList.begin(); i != KST::stringList.end(); ++i) {
		(*i)->readLock();
		QString tag = (*i)->tag().displayString();
		(*i)->unlock();
		_string->insertItem(tag);
		if (tag == prev) {
			found = true;
		}
	}
	KST::stringList.lock().unlock();

	if (found) {
		_string->setCurrentText(prev);
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
	emit newStringCreated();
	update();
	setSelection(s);
    }
    delete se;
}

void StringSelector::selectionWatcher( const QString & tag )
{
    QString label = "["+tag+"]";
    emit selectionChangedLabel(label);
}

void StringSelector::setSelection( const QString & tag )
{
    if (tag.isEmpty()) {
	    return;
    }
    blockSignals(true);
    _string->setCurrentText(tag);
    blockSignals(false);
}

void StringSelector::setSelection( KstStringPtr s )
{
    setSelection(s->tagName());
}

void StringSelector::init()
{
    update();
    connect(_string, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
}


QString StringSelector::selectedString()
{
    return _string->currentText();
}


void StringSelector::allowDirectEntry( bool allowed )
{
    _string->setEditable(allowed);
}
