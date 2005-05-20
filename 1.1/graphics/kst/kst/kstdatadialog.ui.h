/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void KstDataDialog::ok()
{
    if (_newDialog || _dp == 0L) {
	if (newObject()) {
	    close();
	}
    } else {
	if (editObject()) {
	    close();
	}
    }
}


void KstDataDialog::close()
{
    _dp = 0L;
    QDialog::close();
}


void KstDataDialog::reject()
{
    _dp = 0L;
    QDialog::reject();
}


void KstDataDialog::init()
{
    _newDialog = false;
}


void KstDataDialog::showNew()
{
    _newDialog = true;
    _dp = 0L;

    fillFieldsForNew();

    setCaption(i18n("New %1").arg(objectName()));
    show();
    raise();
}


void KstDataDialog::showEdit(const QString& field)
{
    _newDialog = false;
    _dp = findObject(field);

    if (!_dp) {
	showNew();
	return;
    }

    fillFieldsForEdit();

    setCaption(i18n("Edit %1").arg(objectName()));
    show();
    raise();
}


QString KstDataDialog::objectName()
{
    // Designer support for pure virtuals appears to be broken
    return QString::null;
}


void KstDataDialog::fillFieldsForEdit()
{
    // Designer support for pure virtuals appears to be broken
}


void KstDataDialog::fillFieldsForNew()
{
    // Designer support for pure virtuals appears to be broken
}


KstDataObjectPtr KstDataDialog::findObject( const QString & name )
{
    Q_UNUSED(name)
    // Designer support for pure virtuals appears to be broken
    return 0L;
}


bool KstDataDialog::newObject()
{
    // Designer support for pure virtuals appears to be broken
    return false;
}


bool KstDataDialog::editObject()
{
    // Designer support for pure virtuals appears to be broken
    return false;
}
