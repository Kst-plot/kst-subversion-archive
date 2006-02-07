/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void FilterListEditor::init()
{
    _up->setPixmap(SmallIcon("1uparrow"));
    _down->setPixmap(SmallIcon("1downarrow"));
    _left->setPixmap(SmallIcon("1leftarrow"));
    _right->setPixmap(SmallIcon("1rightarrow"));
    update();
}


void FilterListEditor::update()
{
    QString cur = _select->currentText();
    _select->clear();
    KST::filterSetList.lock().readLock();
    for (KstFilterSetList::Iterator it = KST::filterSetList.begin(); it != KST::filterSetList.end(); ++it) {
	_select->insertItem((*it)->name());
    }
    KST::filterSetList.lock().readUnlock();

    if (!cur.isEmpty()) {
      _select->setCurrentText(cur);
    }

    _filters->clear();
    PluginCollection *pc = PluginCollection::self();
    const QMap<QString,Plugin::Data>& pluginList = pc->pluginList();
    QMap<QString,Plugin::Data>::ConstIterator it;

    for (it = pluginList.begin(); it != pluginList.end(); ++it) {
	if (it.data()._filter == false) {
	    continue;
	}
	_filters->insertItem(it.data()._name);	
    }
    
    _vectors->clear();
    KST::vectorList.lock().readLock();
    for (KstVectorList::Iterator vit = KST::vectorList.begin(); vit != KST::vectorList.end(); ++vit) {
	new QCheckListItem(_vectors, (*vit)->tagName(), QCheckListItem::CheckBox);
    }
    KST::vectorList.lock().readUnlock();
 
    comboChanged();

    _new->setEnabled(_filters->count() > 0);
}


void FilterListEditor::displayObject()
{
//    kdDebug() << "displayObject - currentSet=" << _currentSet << " item=" << _select->currentItem() << endl;
    KST::filterSetList.lock().readLock();
    KstFilterSetPtr set = KST::filterSetList.find(_currentSet);
    KST::filterSetList.lock().readUnlock();
    if (set) {
	_applied->clear();
	for (KstFilterSet::Iterator it = set->begin(); it != set->end(); ++it) {
	    FilterListBoxItem *fi = new FilterListBoxItem(_applied, (*it)->tagName());
	    fi->arguments = (*it)->inputScalars();
	    fi->updateText();
	}
    } else {
	if (!_currentSet.isEmpty()) {
	    _select->removeItem(_select->currentItem());
	    _currentSet = QString::null;
	    comboChanged(); // could recurse
	}
    }
}


void FilterListEditor::moveUp()
{
    QListBoxItem *i = _applied->selectedItem();
    if (i) {
	QListBoxItem *p = i->prev();
	if (p) {
	    _applied->takeItem(i);
	    _applied->insertItem(i, p->prev());
	    _applied->setCurrentItem(i);
	    _applied->setSelected(i, true);
	    updateButtons();
	}
    }
}


void FilterListEditor::moveDown()
{
    QListBoxItem *i = _applied->selectedItem();
    if (i) {
	QListBoxItem *n = i->next();
	if (n) {
	    _applied->takeItem(i);
	    _applied->insertItem(i, n);
	    _applied->setCurrentItem(i);
	    _applied->setSelected(i, true);
	    updateButtons();
	}
    }
}


void FilterListEditor::moveLeft()
{
    QListBoxItem *i = _applied->selectedItem();
    delete i;
}


void FilterListEditor::moveRight()
{
    QListBoxItem *i = _filters->selectedItem();
    if (i) {
	QString txt = i->text();
	KstSharedPtr<Plugin> pp = PluginCollection::self()->plugin(txt);
	if (pp) {
	    FilterListBoxItem *fli = new FilterListBoxItem(_applied, txt);
	    for (QValueList<Plugin::Data::IOValue>::ConstIterator ivi = pp->data()._inputs.begin(); ivi != pp->data()._inputs.end(); ++ivi) {
		KstScalar *s = new KstScalar;
		fli->arguments.insert((*ivi)._name, s);
		KST::scalarList.lock().writeLock();
		KST::scalarList.remove(s);
		KST::scalarList.lock().writeUnlock();
		fli->updateText();
	    }
	}
    }
}


void FilterListEditor::applyToVectors()
{
    KST::filterSetList.lock().readLock();
    KstFilterSetPtr set = KST::filterSetList.find(_currentSet);
    KST::filterSetList.lock().readUnlock();
    if (!set) {
	KMessageBox::error(this, i18n("Error applying filter set '%1' to selected vectors."), i18n("Kst"));
	return;
    }

    for (QListViewItem *lvi = _vectors->firstChild(); lvi; lvi = lvi->nextSibling()) {
	QCheckListItem *cli = static_cast<QCheckListItem*>(lvi);
	if (cli->isOn()) {
	    KST::vectorList.lock().readLock();
	    KstVectorPtr v = *KST::vectorList.findTag(cli->text(0));
	    KST::vectorList.lock().readUnlock();
	    if (v) {
		//kdDebug() << "Creating a new filtered vector." << endl;
		KstFilteredVectorPtr fv = new KstFilteredVector(v, set);
		//kdDebug() << "Vector name is: " << fv->tagName() << endl;
	    }
	}
    }
    
    emit docChanged();
}


void FilterListEditor::show()
{
    update();
    updateButtons();
    QDialog::show();
}


void FilterListEditor::updateButtons()
{
    bool en = _filters->isEnabled();
    QListBoxItem *i = _filters->selectedItem();
    _right->setEnabled(en && i);
    QListBoxItem *j = _applied->selectedItem();
    _left->setEnabled(en && j);
    _up->setEnabled(en && j && j->prev());
    _down->setEnabled(en && j && j->next());
    _configure->setEnabled(en && j);
}


void FilterListEditor::newFilterSet()
{
    // Find a unique name and create it.
    int cnt = _select->count();
    if (cnt == 1 && _select->text(0).isEmpty() && _currentSet.isEmpty()) {
	cnt = 0;
    }
    
    for (int x = _select->count() + 1; ; ++x) {
	bool ok = true;
	_currentSet = QString("F%1").arg(x);
	for (int i = 0; i < cnt; ++i) {
	    if (_select->text(i) == _currentSet) {
		ok = false;
		break;
	    }
	}
	if (!ok) {
	    continue;
	}
	break;
    }
    
    KstFilterSetPtr fsp = new KstFilterSet;
    fsp->setName(_currentSet);
    KST::filterSetList.append(fsp);
    
    _select->setEnabled(true);
    _select->insertItem(_currentSet);
    _select->setCurrentItem(_select->count() - 1);
    
    comboChanged();
}


void FilterListEditor::comboChanged()
{
    comboChanged(_select->currentItem());
}


void FilterListEditor::deleteFilter()
{
    KstFilterSetPtr fsp = KST::filterSetList.find(_currentSet);
    if (fsp) {
	KST::filterSetList.remove(fsp);
    }
    _select->removeItem(_select->currentItem());
    _select->setCurrentItem(_select->count() - 1);
    _apply->setEnabled(false);
    //comboChanged();
}


void FilterListEditor::filtersetModified()
{
    _apply->setEnabled(true);
}


void FilterListEditor::applyChanges()
{
    KST::filterSetList.lock().writeLock();
    KstFilterSetPtr p = KST::filterSetList.find(_currentSet);
    if (!p.data()) {
	p = new KstFilterSet;
	p->setName(_currentSet);
	KST::filterSetList.append(p);
    } else {
	if (p->name() != _select->currentText()) {
	    _currentSet = _select->currentText();
	    p->setName(_currentSet);
	    _select->changeItem(_currentSet, _select->currentItem());
	}
    }
    KST::filterSetList.lock().writeUnlock();

    p->clear();
    for (unsigned int i = 0; i < _applied->count(); ++i) {
	FilterListBoxItem *item = static_cast<FilterListBoxItem*>(_applied->item(i));
	QString txt = item->_filter;
	KstSharedPtr<Plugin> pp = PluginCollection::self()->plugin(txt);
	if (pp) {
	    KstFilterPtr fp = new KstFilter;
	    fp->setPlugin(pp);
	    fp->setTagName(txt);
	    fp->inputScalars() = item->arguments;
	    p->append(fp);
	} else {
	    KMessageBox::sorry(this, i18n("Unable to load plugin for filter %1.").arg(txt), i18n("Kst"));
	}
    }
    
    _apply->setEnabled(false);
}


void FilterListEditor::editFilter()
{
    FilterListBoxItem *item = static_cast<FilterListBoxItem*>(_applied->selectedItem());
    if (!item) {
	return;
    }
    
    VariableListEditor *vle = new VariableListEditor(this);
    connect(vle->_view, SIGNAL(variableModified(const QString&)), this, SLOT(filtersetModified()));
    for (KstScalarMap::Iterator i = item->arguments.begin(); i != item->arguments.end(); ++i) {
	if (i.data()->isGlobal()) {
	    vle->_view->addVariable(i.key(), i.data()->tagName());
	} else {
	    vle->_view->addVariable(i.key(), QString::number(i.data()->value()));
	}
    }
    
    int rc = vle->exec();

    if (rc == QDialog::Accepted) {
	QStringList vl = vle->_view->variables();
	for (QStringList::ConstIterator i = vl.begin(); i != vl.end(); ++i) {
	    bool ok = false;
            double x = 0.0;
	    QString val = vle->_view->variable(*i);
	    if (!val.isEmpty()) {
	        x = val.toDouble(&ok);
	    }

	    if (ok) {
		if (item->arguments[*i]->isGlobal()) {
		    KstScalarPtr s = new KstScalar;
		    item->arguments[*i] = s;
		    KST::scalarList.lock().writeLock();
		    KST::scalarList.remove(s);
		    KST::scalarList.lock().writeUnlock();
		}
		item->arguments[*i]->setValue(x);
	    } else {
		KST::scalarList.lock().writeLock();
		KstScalarPtr s = *KST::scalarList.findTag(val);
		KST::scalarList.lock().writeUnlock();
		if (!s) {
		    s = new KstScalar(val);
		    s->setValue(0.0);
		}
		item->arguments[*i] = s;
	    }
	}
	item->updateText();
    }
    
    delete vle;
}


void FilterListEditor::comboChanged( int n )
{
//    kdDebug() << "comboChanged - currentSet=" << _currentSet << " item=" << _select->currentItem() << " n=" << n << endl;
    _apply->setEnabled(false);
    _delete->setEnabled(_select->count() > 0);
    _select->setEnabled(_select->count() > 0);
    _filters->setEnabled(_select->isEnabled());
    _applied->setEnabled(_select->isEnabled());
    _applyFilters->setEnabled(_select->isEnabled());
    updateButtons();
    if (_select->count() > 0) {
	_currentSet = _select->text(n);
	displayObject();
    } else {
	_currentSet = QString::null;
    }
}
