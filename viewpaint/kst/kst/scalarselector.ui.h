/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void ScalarSelector::init()
{
    update();
    connect(_selectScalar, SIGNAL(clicked()), this, SLOT(selectScalar()));
    connect(_newScalar, SIGNAL(clicked()), this, SLOT(createNewScalar()));
    connect(_editScalar, SIGNAL(clicked()), this, SLOT(editScalar()));
    connect(_scalar, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
    connect(_scalar, SIGNAL(textChanged(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
    connect(this, SIGNAL(selectionChanged(const QString&)), this, SLOT(selectionWatcher(const QString&)));
}

void ScalarSelector::allowNewScalars( bool allowed )
{
    _newScalar->setEnabled(allowed);
}

void ScalarSelector::update()
{
    if (_scalar->listBox()->isVisible()) {
        QTimer::singleShot(250, this, SLOT(update()));
        return;
    }

    blockSignals(true);
    
    QString prev = _scalar->currentText();
    bool found = false;
    QStringList scalars;
    
    _scalar->clear();
    
    // FIXME: missing locking?
    for (KstScalarList::Iterator i = KST::scalarList.begin(); i != KST::scalarList.end(); ++i) {
        if ((*i)->displayable()) {
            scalars << (*i)->tagName();
        }
        if ((*i)->tagName() == prev) {
            found = true;
        }
    }
    
    qHeapSort(scalars);
    _scalar->insertStringList(scalars);
    if (found) {
        _scalar->setCurrentText(prev);
    } else {
        _scalar->insertItem("0");
        _scalar->setCurrentText("0");
        _editScalar->setEnabled(false);
    }
    
    blockSignals(false);
}

void ScalarSelector::createNewScalar()
{
    ScalarEditor *se = new ScalarEditor(this, "scalar editor");
    
    int rc = se->exec();
    if (rc == QDialog::Accepted) {
        bool ok = false;
        double val = se->_value->text().toFloat(&ok);
        
 	if (!ok) {
	    val = Equation::interpret(se->_value->text().latin1(), &ok);
	}

        if (ok) {
            KstScalarPtr s = new KstScalar(se->_name->text(), val);
            
            s->setOrphan(true);
            s->setEditable(true);
            emit newScalarCreated();
            update();
            setSelection(s);
            _editScalar->setEnabled(true);
        } else {
            KMessageBox::sorry(this, tr("Kst"), tr("Error saving your new scalar."));
        }
    }
    
    delete se;
}

void ScalarSelector::selectScalar()
{
    ComboBoxSelectionI *selection = new ComboBoxSelectionI(this, "scalar selector");
    int i;
    
    selection->reset();
    for (i=0; i<_scalar->count(); i++) {
        selection->addString(_scalar->text(i));
    }
    selection->sort();
    int rc = selection->exec();
    if (rc == QDialog::Accepted) {        
        _scalar->setCurrentText(selection->selected());   
    }
    
    delete selection;
}

void ScalarSelector::editScalar()
{
    ScalarEditor *se = new ScalarEditor(this, "scalar editor");
    
    KstScalarPtr pold = *KST::scalarList.findTag(_scalar->currentText());
    if (pold && pold->editable()) { 
        se->_value->setText(QString::number(pold->value()));
        se->_name->setText(pold->tagName()); 
        se->_value->selectAll();
        se->_value->setFocus();      
    }
    
    int rc = se->exec();
    if (rc == QDialog::Accepted) {
        bool ok = false;
        double val = se->_value->text().toFloat(&ok);
 
 	if (!ok) {
	    val = Equation::interpret(se->_value->text().latin1(), &ok);
	}

        if (ok) {
            KstScalarPtr p = *KST::scalarList.findTag(se->_name->text());
            if (p) {
                p->setValue(val);
                setSelection(p);
            } else {
                p = new KstScalar(se->_name->text(), val);
                
                p->setOrphan(true);
                p->setEditable(true);
                emit newScalarCreated();
                update();
                setSelection(p);
                _editScalar->setEnabled(true);
            }
        } else {
            KMessageBox::sorry(this, tr("Kst"), tr("Error saving your new scalar."));
        }
    }
    
    delete se;
}

void ScalarSelector::selectionWatcher( const QString & tag )
{   
    bool editable = false;

    QString label = "["+tag+"]";
    emit selectionChangedLabel(label);
    KST::scalarList.lock().readLock();
    KstScalarPtr p = *KST::scalarList.findTag(tag);
    if (p && p->editable()) {
      editable = true;
    }
    KST::scalarList.lock().readUnlock();
    _editScalar->setEnabled(editable);
}

void ScalarSelector::setSelection( const QString & tag )
{
    if (tag.isEmpty()) {
        return;
    }
    blockSignals(true);
    _scalar->setCurrentText(tag);
    selectionWatcher(tag);
    blockSignals(false);
}

void ScalarSelector::setSelection( KstScalarPtr s )
{
    setSelection(s->tagName());
}

QString ScalarSelector::selectedScalar()
{
    return _scalar->currentText();
}

void ScalarSelector::allowDirectEntry( bool allowed )
{
    _scalar->setEditable(allowed);
}
