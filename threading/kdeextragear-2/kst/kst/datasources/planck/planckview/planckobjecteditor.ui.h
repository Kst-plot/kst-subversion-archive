/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void PlanckObjectEditor::setItem( ObjectItem *oi )
{
    _oi = oi;
    
    for (QMap<QString,QString>::ConstIterator i = oi->_attributes.begin(); i != oi->_attributes.end(); ++i) {
	QListViewItem *item = new QListViewItem(_fields, i.key(), i.data());
	item->setRenameEnabled(0, true);
	item->setRenameEnabled(1, true);
    }
}


void PlanckObjectEditor::newItem()
{
    QListViewItem *item = new QListViewItem(_fields);
    item->setRenameEnabled(0, true);
    item->setRenameEnabled(1, true);
    item->startRename(0);
}


void PlanckObjectEditor::save()
{
    _oi->_attributes.clear();
    for (QListViewItem *i = _fields->firstChild(); i; i = i->nextSibling()) {
	_oi->_attributes[i->text(0)] = i->text(1);
    }
}


void PlanckObjectEditor::setDirty()
{
    _save->setEnabled(true);
}


void PlanckObjectEditor::setClean()
{
    _save->setDisabled(true);
}


void PlanckObjectEditor::updateDelete()
{
    _delete->setEnabled(_fields->selectedItem() != 0L);
}


void PlanckObjectEditor::deleteItem()
{
    delete _fields->selectedItem();
    updateDelete();
}
