/**************************************************************************
        kstchangefiledialog_i.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2001
    copyright            : (C) 2000-2003 by Barth Netterfield
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
#include <qwidget.h>
#include <qfontdatabase.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <kurl.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "kstchangefiledialog_i.h"
#include "kstrvector.h"
#include "kstdatacollection.h"

KstChangeFileDialogI::KstChangeFileDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl) :
  KstChangeFileDialog(parent, name, modal, fl) {


    connect(ChangeFileClear,     SIGNAL(clicked()),
            ChangeFileCurveList, SLOT(clearSelection()));
    connect(ChangeFileSelectAll, SIGNAL(clicked()),
            this,                SLOT(selectAll()));
    connect(VectorFileBrowse,    SIGNAL(clicked()),
            this,                SLOT(browseFile()));

    connect(ChangeFileApply,     SIGNAL(clicked()),
            this,                SLOT(applyFileChange()));
}

KstChangeFileDialogI::~KstChangeFileDialogI() {
}

void KstChangeFileDialogI::selectAll() {
  ChangeFileCurveList->selectAll(true);
}

void KstChangeFileDialogI::updateChangeFileDialog() {
  int i;

  ChangeFileCurveList->clear();
  ChangeFileName->clear();

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  /* insert vectors into ChangeFileCurveList */
  for (i = 0; i < (int)rvl.count(); i++) {
    ChangeFileCurveList->insertItem(rvl[i]->tagName(), -1);
  }

  /* insert file names into ChangeFileName list */
  for (i = 0; i < (int)KST::fileList.count(); i++) {
    ChangeFileName->insertItem(KST::fileList[i]->fileName());
  }
}

void KstChangeFileDialogI::showChangeFileDialog() {

  updateChangeFileDialog();

  show();
  raise();
}

void KstChangeFileDialogI::browseFile() {
    QString newFileName=
    KFileDialog::getOpenFileName("::<kstdatadir>", QString::null,
                                 this, i18n("New"));
  if(!newFileName.isEmpty()) {
    ChangeFileName->insertItem(newFileName,0);
    ChangeFileName->setCurrentItem(0);
  }
}

void KstChangeFileDialogI::applyFileChange() {
  int i_vector;
  KstFilePtr file;
  KstRVectorPtr vector;

  /* if there is not an active KstFile, create one */
  KstFileList::Iterator it;
  for (it = KST::fileList.begin(); it != KST::fileList.end(); ++it) {
    if ((*it)->fileName() == ChangeFileName->currentText()) {
      file = *it;
      break;
    }
  }

  if (it == KST::fileList.end()) {
    file = new KstFile(ChangeFileName->currentText());
    if (file->numFrames() < 1) { // No data in file
      KMessageBox::sorry(0L, i18n("%1: The requested file does not contain data. Operation canceled.").arg(ChangeFileName->currentText()));
      file = 0;
      return;
    }
    KST::fileList.append(file);
  }

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (i_vector = 0; i_vector < (int)ChangeFileCurveList->count(); i_vector++) {
    if (ChangeFileCurveList->isSelected(i_vector)) {
      vector = rvl[i_vector];
      if (!file->isValidField(vector->getField())) {
        KMessageBox::sorry(0L, i18n("%1: The requested field is not defined for the requested file.").arg(vector->getField()));
      } else {
        vector->changeFile(file);
      }
    }
  }

  /** purge unused files */
  //KST::fileList.Purge();

  emit docChanged();
}

#include "kstchangefiledialog_i.moc"

