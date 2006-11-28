/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void VectorSelector::init()
{
    _provideNoneVector = false;
    update();
    connect(_vector, SIGNAL(activated(const QString&)), this, SIGNAL(selectionChanged(const QString&))); // annoying that signal->signal doesn't seem to work in .ui files
}


void VectorSelector::allowNewVectors( bool allowed )
{
    _newVector->setEnabled(allowed);
}


QString VectorSelector::selectedVector()
{
    if (_provideNoneVector && _vector->currentItem() == 0) {
	return QString::null;
    }
    return _vector->currentText();
}


void VectorSelector::update()
{
    if (_vector->listBox()->isVisible()) {
        QTimer::singleShot(250, this, SLOT(update()));
        return;
    }

    blockSignals(true);
    QString prev = _vector->currentText();
    bool found = false;
    _vector->clear();
    if (_provideNoneVector) {
	_vector->insertItem(tr("<None>"));
    }
    QStringList vectors;
    KST::vectorList.lock().readLock();
    for (KstVectorList::ConstIterator i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
	(*i)->readLock();
	if (!(*i)->isScalarList()){
	    vectors << (*i)->tag().tagString();
	    if (!found && (*i)->tag().tagString() == prev) {
		found = true;
	    }
	}
	(*i)->unlock();
    }
    KST::vectorList.lock().unlock();
    qHeapSort(vectors);
    _vector->insertStringList(vectors);
    if (found) {
	_vector->setCurrentText(prev);
    }
    blockSignals(false);
    setEdit(_vector->currentText());
}


void VectorSelector::createNewVector()
{
    KstDialogs::self()->newVectorDialog(this, SLOT(newVectorCreated(KstVectorPtr)), SLOT(setSelection(KstVectorPtr)), SLOT(update()));
}


void VectorSelector::selectionWatcher( const QString & tag )
{
    QString label = "[" + tag + "]";
    emit selectionChangedLabel(label);
    setEdit(tag);
}


void VectorSelector::setSelection( const QString & tag )
{
    if (tag.isEmpty()) {
	if (_provideNoneVector) {
	    blockSignals(true);
	    _vector->setCurrentItem(0);
	    blockSignals(false);

	    _editVector->setEnabled(false);
	}
	return;
    }
    blockSignals(true);
    _vector->setCurrentText(tag);  // What if it isn't in the combo?
    blockSignals(false);

    setEdit(tag);
}


void VectorSelector::newVectorCreated( KstVectorPtr v )
{
    v->readLock();
    QString name = v->tagName();
    v->unlock();
    v = 0L; // deref
    emit newVectorCreated(name);
}


void VectorSelector::setSelection( KstVectorPtr v )
{
    v->readLock();
    setSelection(v->tagName());
    v->unlock();
}


void VectorSelector::provideNoneVector( bool provide )
{
    if (provide != _provideNoneVector) {
	_provideNoneVector = provide;
	update();
    }
}


void VectorSelector::editVector()
{
    KstDialogs::self()->showVectorDialog(_vector->currentText());
}


void VectorSelector::setEdit( const QString& tag )
{
    KST::vectorList.lock().readLock();
    KstRVectorPtr rvp = kst_cast<KstRVector>(*KST::vectorList.findTag(tag));
    KstSVectorPtr svp = kst_cast<KstSVector>(*KST::vectorList.findTag(tag));
    KST::vectorList.lock().unlock();
    _editVector->setEnabled(rvp||svp);
}

// vim: ts=8 sw=4 noet
