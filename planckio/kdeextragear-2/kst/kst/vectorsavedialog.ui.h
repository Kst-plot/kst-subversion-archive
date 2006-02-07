/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void VectorSaveDialog::save()
{
    KstVectorList::Iterator vi = KST::vectorList.findTag(_vectorList->currentText());
    if (vi == KST::vectorList.end()) {
	return;
    }

    KstVectorPtr vp = *vi; // hold a reference
    
    QString filename = KFileDialog::getSaveFileName(QDir::currentDirPath(), QString::null, this, i18n("Save Vector As"));
    if(!filename.isEmpty()) {
        if (QFile::exists(filename)) {
            int rc = KMessageBox::warningYesNo(this, i18n("File %1 exists.  Overwrite?").arg(filename), i18n("Kst"));
            if (rc == KMessageBox::No) {
                return;
            }
        }

	QFile f(filename);
	if (f.open(IO_WriteOnly)) {
	    if (0 != KST::vectorToFile(vp, &f)) {
                KMessageBox::sorry(this, i18n("Error saving vector to file %1.").arg(filename), i18n("Kst"));
            }
	} else {
            KMessageBox::sorry(this, i18n("Error opening file %1 for output.").arg(filename), i18n("Kst"));
        }
    }
}

void VectorSaveDialog::show()
{
    init();
    QDialog::show();
}

void VectorSaveDialog::init()
{
    _vectorList->clear();
    for (KstVectorList::Iterator i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
	_vectorList->insertItem((*i)->tagName());
    }
    _saveButton->setEnabled(false);
}


void VectorSaveDialog::selectionChanged()
{
    _saveButton->setEnabled(_vectorList->currentItem() >= 0);
}
