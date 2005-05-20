/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void MatrixSelector::allowNewMatrices( bool allowed )
{
    _newMatrix->setEnabled(allowed);
}


QString MatrixSelector::selectedMatrix()
{
    if (_provideNoneMatrix && _matrix->currentItem() == 0) {
	return QString::null;
    }
    return _matrix->currentText();
}


void MatrixSelector::update()
{
    if (_matrix->listBox()->isVisible()) {
	QTimer::singleShot(250, this, SLOT(update()));
	return;
    }

    blockSignals(true);
    QString prev = _matrix->currentText();
    bool found = false;
    _matrix->clear();
    if (_provideNoneMatrix) {
	_matrix->insertItem("<None>");
    }
    KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
    for (KstMatrixList::Iterator i = matrices.begin(); i != matrices.end(); ++i) {
	_matrix->insertItem((*i)->tagName());
	if (!found && (*i)->tagName() == prev) {
	    found = true;
	}
    }
    if (found) {
	_matrix->setCurrentText(prev);
    }
    blockSignals(false);
    setEdit(_matrix->currentText());
}


void MatrixSelector::init()
{
    _provideNoneMatrix = false;
    update();
    connect(_matrix, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&)));
}


void MatrixSelector::createNewMatrix()
{
    KstMatrixDialogI *ad = new KstMatrixDialogI(this, "matrix dialog");
    connect(ad, SIGNAL(matrixCreated(KstMatrixPtr)), this, SLOT(newMatrixCreated(KstMatrixPtr)));
    connect(ad, SIGNAL(matrixCreated(KstMatrixPtr)), this, SLOT(setSelection(KstMatrixPtr)));
    connect(ad, SIGNAL(modified()), this, SLOT(update()));
    ad->show_New();
    ad->exec();
    delete ad;
}


void MatrixSelector::selectionWatcher( const QString & tag )
{
    QString label = "["+tag+"]";
    emit selectionChangedLabel(label);
    setEdit(tag);
}


void MatrixSelector::setSelection( const QString & tag )
{
    if (tag.isEmpty()) {
	if (_provideNoneMatrix) {
	    blockSignals(true);
	    _matrix->setCurrentItem(0);
	    blockSignals(false);
	    _editMatrix->setEnabled(false);
	}
	return;
    }
    blockSignals(true);
    _matrix->setCurrentText(tag);  // What if it isn't in the combo?
    blockSignals(false);

    setEdit(tag);
}


void MatrixSelector::newMatrixCreated( KstMatrixPtr v )
{
    QString name = v->tagName();
    v = 0L; // deref
    emit newMatrixCreated(name);
}


void MatrixSelector::setSelection( KstMatrixPtr v )
{
    setSelection(v->tagName());
}


void MatrixSelector::provideNoneMatrix( bool provide )
{
    if (provide != _provideNoneMatrix) {
	_provideNoneMatrix = provide;
	update();
    }
}


void MatrixSelector::editMatrix()
{
    KstMatrixDialogI::globalInstance()->show_Edit(_matrix->currentText());
}


void MatrixSelector::setEdit(QString tag)
{
    KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
    _editMatrix->setEnabled(matrices.findTag(tag) != matrices.end());
}

// vim: ts=8 sw=4 noet
