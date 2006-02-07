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
  KST::dataSourceList.lock().readLock();
  for (i = 0; i < (int)KST::dataSourceList.count(); i++) {
    ChangeFileName->insertItem(KST::dataSourceList[i]->fileName());
  }
  KST::dataSourceList.lock().readUnlock();
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
  KstDataSourcePtr file;
  KstRVectorPtr vector;
  KstWriteLocker ml(&KST::dataSourceList.lock());

  /* if there is not an active KstFile, create one */
  KstDataSourceList::Iterator it;
  for (it = KST::dataSourceList.begin(); it != KST::dataSourceList.end(); ++it) {
    if ((*it)->fileName() == ChangeFileName->currentText()) {
      file = *it;
      break;
    }
  }

  if (it == KST::dataSourceList.end()) {
    file = KstDataSource::loadSource(ChangeFileName->currentText());
    if (!file || !file->isValid()) {
      KMessageBox::sorry(0L, i18n("%1: Unable to open file.").arg(ChangeFileName->currentText()));
      return;
    }
    if (file->frameCount() < 1) {
      KMessageBox::sorry(0L, i18n("%1: File does not contain data. Operation canceled.").arg(ChangeFileName->currentText()));
      return;
    }
    KST::dataSourceList.append(file);
  }

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (int i = 0; i < (int)ChangeFileCurveList->count(); i++) {
    if (ChangeFileCurveList->isSelected(i)) {
      vector = rvl[i];
      if (!file->isValidField(vector->getField())) {
        KMessageBox::sorry(0L, i18n("%1: Field is not defined for the requested file.").arg(vector->getField()));
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

