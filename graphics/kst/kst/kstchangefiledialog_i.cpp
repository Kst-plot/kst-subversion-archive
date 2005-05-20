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

#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include <qregexp.h>

#include "kst.h"
#include "kstchangefiledialog_i.h"
#include "kstdatacollection.h"
#include "kstrvector.h"
#include "kstvectordefaults.h"

KstChangeFileDialogI::KstChangeFileDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl)
: KstChangeFileDialog(parent, name, modal, fl) {
  _dataFile->completionObject()->setDir(QDir::currentDirPath());
  _clearFilter->setPixmap(BarIcon("locationbar_erase"));
  connect(_clearFilter, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(_clearFilter, SIGNAL(clicked()), ChangeFileCurveList, SLOT(clearSelection()));
  connect(_filter, SIGNAL(textChanged(const QString&)), this, SLOT(updateSelection(const QString&)));
  connect(ChangeFileClear, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(ChangeFileClear, SIGNAL(clicked()), ChangeFileCurveList, SLOT(clearSelection()));
  connect(ChangeFileSelectAll, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(ChangeFileSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
  connect(ChangeFileApply, SIGNAL(clicked()), this, SLOT(applyFileChange()));
  connect(_allFromFile, SIGNAL(clicked()), _filter, SLOT(clear()));
  connect(_allFromFile, SIGNAL(clicked()), this, SLOT(allFromFile()));
}


KstChangeFileDialogI::~KstChangeFileDialogI() {
}


void KstChangeFileDialogI::selectAll() {
  ChangeFileCurveList->selectAll(true);
}


void KstChangeFileDialogI::updateChangeFileDialog() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  QStringList tags;

  // insert vectors into ChangeFileCurveList
  for (int i = 0; i < (int)rvl.count(); i++) {
    rvl[i]->readLock();
    tags += rvl[i]->tagName();
    rvl[i]->readUnlock();
  }

  QListBoxItem *bi = ChangeFileCurveList->firstItem();
  while (bi) {
    if (!tags.contains(bi->text())) {
      QListBoxItem *del = bi;
      bi = bi->next();
      delete del;
    } else {
      bi = bi->next();
    }
  }

  for (QStringList::ConstIterator i = tags.begin(); i != tags.end(); ++i) {
    if (!ChangeFileCurveList->findItem(*i)) {
      ChangeFileCurveList->insertItem(*i, -1);
    }
  }

  if (!isShown()) {
    _dataFile->setURL(KST::vectorDefaults.dataSource());
  }

  QString currentFile = _files->currentText();
  _files->clear();
  KstReadLocker ml(&KST::dataSourceList.lock());
  for (KstDataSourceList::Iterator it = KST::dataSourceList.begin(); it != KST::dataSourceList.end(); ++it) {
    _files->insertItem((*it)->fileName());
  }

  if (_files->contains(currentFile)) {
    _files->setCurrentText(currentFile);
  }

  _allFromFile->setEnabled(_files->count() > 0);
  _files->setEnabled(_files->count() > 0);
}


void KstChangeFileDialogI::showChangeFileDialog() {
  updateChangeFileDialog();
  show();
  raise();
  _filter->setFocus();
}


void KstChangeFileDialogI::applyFileChange() {
  KstDataSourcePtr file;
  KST::dataSourceList.lock().writeLock();
  KstDataSourceList::Iterator it = KST::dataSourceList.findReusableFileName(_dataFile->url());
  QString invalidSources;
  int invalid = 0;

  if (it == KST::dataSourceList.end()) {
    file = KstDataSource::loadSource(_dataFile->url());
    if (!file || !file->isValid()) {
      KST::dataSourceList.lock().writeUnlock();
      KMessageBox::sorry(this, i18n("The file could not be loaded."));
      return;
    }
    if (file->isEmpty()) {
      KST::dataSourceList.lock().writeUnlock();
      KMessageBox::sorry(this, i18n("The file does not contain data."));
      return;
    }
    KST::dataSourceList.append(file);
  } else {
    file = *it;
  }
  KST::dataSourceList.lock().writeUnlock();

  file->writeLock();

  KstApp *app = KstApp::inst();
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  int selected = 0;
  int handled = 0;
  int count;
  int i;

  count = (int)ChangeFileCurveList->count();
  for (i = 0; i < count; i++) {
    if (ChangeFileCurveList->isSelected(i)) {
      selected++;
    }
  }

  for (i = 0; i < count; i++) {
    if (ChangeFileCurveList->isSelected(i)) {
      KstRVectorPtr vector = rvl[i];
      vector->writeLock();
      if (!file->isValidField(vector->field())) {
        if (invalid > 0) {
          // FIXME: invalid list construction for i18n
          invalidSources = i18n("%1, %2").arg(invalidSources).arg(vector->field());
        } else {
          invalidSources = vector->field();
        }
        invalid++;
      } else {
        vector->changeFile(file);
      }
      vector->writeUnlock();
      app->slotUpdateProgress(selected, ++handled, i18n("Updating vectors..."));
    }
  }

  app->slotUpdateProgress(0, 0, QString::null);

  file->writeUnlock();
  file = 0L;

  if (!invalidSources.isEmpty()) {
    if (invalid == 1) {
      KMessageBox::sorry(this, i18n("The following field is not defined for the requested file:\n%1").arg(invalidSources));
    } else {
      KMessageBox::sorry(this, i18n("The following fields are not defined for the requested file:\n%1").arg(invalidSources));
    }
  }

  emit docChanged();

  // force an update in case we're in paused mode
  KstApp::inst()->forceUpdate();
}


void KstChangeFileDialogI::allFromFile() {
  if (_files->count() <= 0) {
    return;
  }

  ChangeFileCurveList->selectAll(false);
  KstReadLocker rl(&KST::vectorList.lock());
  for (uint i = 0; i < ChangeFileCurveList->count(); ++i) {
    KstRVectorPtr v = kst_cast<KstRVector>(*KST::vectorList.findTag(ChangeFileCurveList->text(i)));
    ChangeFileCurveList->setSelected(i, v && v->filename() == _files->currentText());
  }
}


void KstChangeFileDialogI::updateSelection(const QString& txt) {
  ChangeFileCurveList->selectAll(false);
  QRegExp re(txt, true, true);
  for (uint i = 0; i < ChangeFileCurveList->count(); ++i) {
    ChangeFileCurveList->setSelected(i, re.exactMatch(ChangeFileCurveList->text(i)));
  }
}

#include "kstchangefiledialog_i.moc"
// vim: ts=2 sw=2 et
