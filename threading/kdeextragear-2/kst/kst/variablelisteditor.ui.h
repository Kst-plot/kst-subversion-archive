/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/




void VariableListEditor::doubleClicked( QListViewItem *i, const QPoint &, int c )
{
    if (c == 1) {
	i->startRename(c);
    }
}


