/***************************************************************************
                       kstvectordialog_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "kstvectordialog_i.h"

#include <iostream>
#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <kstaticdeleter.h>
#include <kcombobox.h>

#include "kstrvector.h"
#include "kstdoc.h"
#include "kstdatacollection.h"

KstVectorDialogI *KstVectorDialogI::_inst = 0L;
static KStaticDeleter<KstVectorDialogI> _vInst;

KstVectorDialogI *KstVectorDialogI::globalInstance() {
  if (!_inst) {
    _inst = _vInst.setObject(new KstVectorDialogI);
  }
return _inst;
}

KstVectorDialogI::KstVectorDialogI(QWidget* parent, const char* name,
                                   bool modal, WFlags fl)
: KstVectorDialog(parent, name, modal, fl) {
    connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
    connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
    connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
    connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));

    FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly
                      | KFile::LocalOnly);
    _fieldCompletion = Field->completionObject();
    Field->setAutoDeleteCompletionObject( true );
//     connect(Field,SIGNAL(returnPressed(const QString&)),
//             _fieldCompletion,SLOT(addItem(const QString&));
}

KstVectorDialogI::~KstVectorDialogI() {

}

void KstVectorDialogI::show_I() {
  update();
  show();
  raise();
}

void KstVectorDialogI::show_New() {
  update(-2);
  show();
  raise();
}

void KstVectorDialogI::show_I(const QString &field) {
  int i = kstObjectSubList<KstVector,KstRVector>(KST::vectorList).findIndexTag(field);
  update(i);
  show();
  raise();
}

void KstVectorDialogI::update(int new_index) {
  int i_vector, index, n_v;
  KstRVectorPtr vector;
  bool isNew = false;

  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  if (new_index == -1) {
    if (vectorList.findTag(Select->currentText()) != vectorList.end()) {
      QString save = Select->currentText();
      Select->blockSignals(true);
      Select->clear();
      for (KstRVectorList::iterator i = vectorList.begin(); i != vectorList.end(); ++i) {
        Select->insertItem((*i)->tagName());
      }
      Select->setCurrentText(save);
      Select->blockSignals(false);
      return;
    }
  }

  n_v = vectorList.count();
  if (n_v < 1) {
    Select->clear();
    Select->insertItem("V1-" + i18n("<New_Vector>"));
    Delete->setEnabled(false);
    return;
  }

  if (new_index == -2) {
    isNew = true;
    new_index = n_v-1;
  }

  if (new_index >= 0) {
    index = new_index;
  } else if (n_v > 0) {
    index = Select->currentItem();
  } else {
    index = n_v-1;
  }

  /** fill VectorListBox with vector tags */
  Select->clear();
  for (KstRVectorList::iterator i = vectorList.begin(); i != vectorList.end(); ++i) {
    Select->insertItem((*i)->tagName());
  }

  if (index >= 0 && index < n_v) {
    Select->setCurrentItem(index);
  } else if (n_v > 0) {
    Select->setCurrentItem(n_v - 1);
  }

  i_vector = Select->currentItem();
  vector = vectorList[i_vector];

  /* fill the fields */
  Field->clear();
  _fieldCompletion->clear();
  KstDataSourcePtr tf;
  {
    KST::dataSourceList.lock().readLock();
    KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(vector->filename());
    if (it != KST::dataSourceList.end()) {
      tf = *it;
      Field->insertStringList(tf->fieldList());
      _fieldCompletion->insertItems(tf->fieldList());
      //std::cout << "inserting string list: " << tf->fieldList().count() << "\n";
    }
    KST::dataSourceList.lock().readUnlock();
  }
  if (isNew) {
    Field->setCurrentText(QString::null);
  } else {
    Field->setCurrentText(vector->getField());
  }

  /* select the proper file */
  FileName->setURL(vector->filename());

  /* fill the vector range entries */
  if (vector->countFromEOF()) {
    CountFromEnd->setChecked(true);
  } else {
    CountFromEnd->setChecked(false);
  }
  F0->setValue(vector->reqStartFrame());

  /* fill number of frames entries */
  if (vector->readToEOF()) {
    ReadToEnd->setChecked(true);
  } else {
    ReadToEnd->setChecked(false);
  }
  N->setValue(vector->reqNumFrames());

  /* fill in frames to skip box */
  Skip->setValue(vector->skip());
  DoSkip->setChecked(vector->doSkip());
  DoFilter->setChecked(vector->doAve());

  if (isNew) {
    QString new_label;
    new_label.sprintf("V%d-", n_v+1);
    new_label += i18n("<New_Vector>");
    Select->insertItem(new_label);
    Select->setCurrentItem(n_v);
    Delete->setEnabled(false);
  } else {
    Delete->setEnabled(vector->getUsage() == 2);
  }
}

void KstVectorDialogI::browseFile() {
  QString fileToOpen = KFileDialog::getOpenFileName("::<kstdatadir>",
                                                    QString::null,
                                                    this,
                                                    i18n("Open File..."));
  if(!fileToOpen.isEmpty()) {
    FileName->setURL(fileToOpen);
  }
}

void KstVectorDialogI::new_I() {
  KstDataSourcePtr file;
  KstRVectorPtr vector;
  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  QString tag_name = Select->currentText();
  tag_name.replace(i18n("<New_Vector>"), Field->currentText());

  KST::vectorList.lock().readLock();
  int i_c = KST::vectorList.count() + 1;
  KST::vectorList.lock().readUnlock();
  while (KST::dataTagNameNotUnique(tag_name, false)) {
    tag_name.sprintf("V%d-", i_c);
    tag_name += Field->currentText();
    i_c++;
  }

  /* if there is not an active KstFile, create one */
  {
    KST::dataSourceList.lock().writeLock();
    KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(FileName->url());

    if (it == KST::dataSourceList.end()) {
      file = KstDataSource::loadSource(FileName->url());
      if (!file || !file->isValid()) {
        KMessageBox::sorry(0L, i18n("The file could not be loaded."));
        return;
      }
      if (file->frameCount() < 1) {
        KMessageBox::sorry(0L, i18n("The file does not contain data."));
        return;
      }
      KST::dataSourceList.append(file);
    } else {
      file = *it;
    }
    KST::dataSourceList.lock().writeUnlock();
  }
  if (!file->isValidField(Field->currentText())) {
    KMessageBox::sorry(0L, i18n("The requested field is not defined for the requested file."));
    return;
  }

  /* create the vector */
  vector = new KstRVector(file, Field->currentText(),
                         tag_name,
                         (CountFromEnd->isChecked() ? -1 : F0->value()),
                         (ReadToEnd->isChecked() ? -1 : N->value()),
                         Skip->value(),
			 DoSkip->isChecked(),
			 DoFilter->isChecked());

  emit vectorCreated(KstVectorPtr(vector));
  vector = 0L;
  vectorList.clear();
  emit modified();
}

void KstVectorDialogI::edit_I() {
  KstDataSourcePtr file;
  KstRVectorPtr vector;

  int index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active vector to edit."));
    return;
  }

  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

  if (unsigned(index) >= vectorList.count()) {
    new_I();
  } else {
    /* verify that the vector name is unique */
    if (Select->currentText() != vectorList[index]->tagName()) {
      if (KST::dataTagNameNotUnique(Select->currentText())) return;
    }

    /* if there is not an active KstFile, create one */
    {
      KST::dataSourceList.lock().writeLock();
      KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(FileName->url());

      if (it == KST::dataSourceList.end()) {
        file = KstDataSource::loadSource(FileName->url());
        if (!file || !file->isValid()) {
          KMessageBox::sorry(0L, i18n("The file could not be opened."));
          return;
        }
        if (file->frameCount() < 1) {
          KMessageBox::sorry(0L, i18n("The file does not contain data."));
          return;
        }
        KST::dataSourceList.append(file);
      } else {
        file = *it;
      }
      KST::dataSourceList.lock().writeUnlock();
    }
    if (!file->isValidField(Field->currentText())) {
      KMessageBox::sorry(0L, i18n("The requested field is not defined for the requested file\n"));
      return;
    }

    vector = vectorList[index];

    /* change the vector */
    vector->change(file, Field->currentText(),
                   Select->currentText(),
                   (CountFromEnd->isChecked() ?
                    -1 : F0->value()),
                   (ReadToEnd->isChecked() ?
                    -1 : N->value()),
                   Skip->value(),
		   DoSkip->isChecked(),
		   DoFilter->isChecked());

    /** purge unused files */
    //doc->fileList.Purge();

    vector = 0L;
    vectorList.clear();
    emit modified();
  }
}

void KstVectorDialogI::delete_I() {
  int index;
  KstRVectorPtr vector;

  KstRVectorList vectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active vector to delete."));
    return;
  }

  if (unsigned(index) >= vectorList.count()) {
    return;
  }

  vector = vectorList[index];
  if (vector->getUsage() > 2) {
    KMessageBox::sorry(0L, i18n("Cannot delete: Selected vector is used by at least one curve. Delete curves first."));
    return;
  }

  KST::vectorList.lock().writeLock();
  KST::vectorList.remove(vectorList[index].data());
  KST::vectorList.lock().writeUnlock();

  /** purge unused files */
  //doc->fileList.Purge();

  vector = 0L;
  vectorList.clear();
  emit modified();
}

#include "kstvectordialog_i.moc"
// vim: ts=2 sw=2 et
