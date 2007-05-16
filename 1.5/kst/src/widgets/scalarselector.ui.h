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
    _newScalar->setPixmap(BarIcon("kst_scalarnew"));
    _editScalar->setPixmap(BarIcon("kst_scalaredit"));
    connect(_selectScalar, SIGNAL(clicked()), this, SLOT(selectScalar()));
    connect(_newScalar, SIGNAL(clicked()), this, SLOT(createNewScalar()));
    connect(_editScalar, SIGNAL(clicked()), this, SLOT(editScalar()));
    connect(_scalar, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
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
    
	KST::scalarList.lock().readLock();
    for (KstScalarList::Iterator i = KST::scalarList.begin(); i != KST::scalarList.end(); ++i) {
		(*i)->readLock();
	    QString tag = (*i)->tag().displayString();
        if ((*i)->displayable()) {
            scalars << tag;
        }
		(*i)->unlock();

        if (tag == prev) {
            found = true;
        }
    }
	KST::scalarList.lock().unlock();
    
    qHeapSort(scalars);
    _scalar->insertStringList(scalars);

    bool ok;
    prev.toDouble(&ok);
    ok = ok && _scalar->editable(); //if directentry is allowed, and if prev is a number, then we can put it back into the combobox.

    if (found || ok) {
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
            KstScalarPtr s = new KstScalar(KstObjectTag(se->_name->text(), KstObjectTag::globalTagContext), 0L, val);
            
            s->setOrphan(true);
            s->setEditable(true);
            emit newScalarCreated();
            update();
            setSelection(s);
            _editScalar->setEnabled(true);
        } else {
            KMessageBox::sorry(this, tr("Error saving your new scalar."), tr("Kst"));
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
                p = new KstScalar(KstObjectTag(se->_name->text(), KstObjectTag::globalTagContext), 0L, val);

                p->setOrphan(true);
                p->setEditable(true);
                emit newScalarCreated();
                update();
                setSelection(p);
                _editScalar->setEnabled(true);
            }
        } else {
            KMessageBox::sorry(this, tr("Error saving your new scalar."), tr("Kst"));
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
    KST::scalarList.lock().unlock();
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
    KstScalarPtr ptr = *KST::scalarList.findTag(_scalar->currentText());
    if (ptr) {
        return _scalar->currentText();
    } else {
        return QString::null;
    }
}

KstScalarPtr ScalarSelector::selectedScalarPtr()
//returns a pointer to the selected scalar.
//if the scalar does not exist, but
	//the text entered is a number and
	//the selector is editable then
//a new scalar is created with the given value and returned.
{
  KstScalarPtr ptr = *KST::scalarList.findTag(_scalar->currentText());
  
  if (!ptr) {
    if (_scalar->editable()) {
      KstWriteLocker sl(&KST::scalarList.lock());
      bool ok;
      double val = _scalar->currentText().toDouble(&ok);

      if (ok) {
        ptr = new KstScalar(KstObjectTag::fromString(_scalar->currentText()), 0L, val, true, false);
      }
    }
  }

  return ptr;
}

void ScalarSelector::allowDirectEntry( bool allowed )
{
    _scalar->setEditable(allowed);
}
