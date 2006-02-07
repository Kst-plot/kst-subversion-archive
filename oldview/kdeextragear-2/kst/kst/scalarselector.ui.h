/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void ScalarSelector::allowNewScalars( bool allowed )
{
    _newScalar->setEnabled(allowed);
}

void ScalarSelector::update()
{
    blockSignals(true);
    QString prev = _scalar->currentText();
    bool found = false;
    _scalar->clear();
    for (KstScalarList::Iterator i = KST::scalarList.begin(); i != KST::scalarList.end(); ++i) {
	_scalar->insertItem((*i)->tagName());
	if (!found && (*i)->tagName() == prev) {
	    found = true;
	}
    }
    if (found) {
	_scalar->setCurrentText(prev);
    }
    blockSignals(false);
}

void ScalarSelector::createNewScalar()
{
    QDoubleValidator dv(this);
    ScalarEditor *se = new ScalarEditor(this, "scalar editor");
    se->_value->setValidator(&dv);
    int rc = se->exec();
    if (rc == QDialog::Accepted) {
	bool ok = false;
	double val = se->_value->text().toFloat(&ok);
	if (ok) {
	    KstScalarPtr s = new KstScalar(se->_name->text(), val);
	    s->setOrphan(true);
	    emit newScalarCreated();
	    update();
	    setSelection(s);
	} else {
	    QMessageBox::warning(0L, tr("Kst"), tr("Error saving your new scalar."));
	}
    }
    delete se;
}

void ScalarSelector::selectionWatcher( const QString & tag )
{
    QString label = "["+tag+"]";
    emit selectionChangedLabel(label);
}

void ScalarSelector::setSelection( const QString & tag )
{
    if (tag.isEmpty()) {
	return;
    }
    blockSignals(true);
    _scalar->setCurrentText(tag);
    blockSignals(false);
}

void ScalarSelector::setSelection( KstScalarPtr s )
{
    setSelection(s->tagName());
}

void ScalarSelector::init()
{
    update();
    connect(_scalar, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
}


QString ScalarSelector::selectedScalar()
{
    return _scalar->currentText();
}
