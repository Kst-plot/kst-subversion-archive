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


void ViewLabelWidget::init()
{
    connect(_scalars, SIGNAL(selectionChanged(const QString &)),
	    this, SLOT(insertScalarInText(const QString &)));
    _horizontal->insertItem(i18n("Left"));
    _horizontal->insertItem(i18n("Right"));
    _horizontal->insertItem(i18n("Center"));
    _vertical->insertItem(i18n("Top"));
    _vertical->insertItem(i18n("Bottom"));
    _vertical->insertItem(i18n("Center"));

}


void ViewLabelWidget::insertScalarInText(const QString &S)
{
    _text->insert("["+S+"]");
}
