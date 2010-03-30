/**************************************************************************
              kstquickstartdialog.cpp - quickstart dialog: inherits designer dialog
                             -------------------
    begin                :  2004
    copyright            : (C) 2004 University of British Columbia
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

// xxx #include <QCheckBox>
// xxx #include <Q3ListBox>

#include <kurlcompletion.h>
#include <kurlrequester.h>

#include "kst.h"
#include "kstdoc.h"
#include "kstquickstartdialog.h"
#include "kstsettings.h"

KstQuickStartDialog::KstQuickStartDialog(QWidget *parent, const char *name, bool modal, Qt::WindowFlags fl)
: QDialog(parent,fl) {
  setupUi(this);
  _fileName->completionObject()->setDir(QDir::currentPath());
  _app = KstApp::inst();
  _isRecentFile = false;
  _openFile->setEnabled(false);

  connect(_startDataWizard, SIGNAL(clicked()), this, SLOT(wizard_I()));
  connect(_openFile, SIGNAL(clicked()), this, SLOT(open_I()));
  connect(_recentFileList, SIGNAL(highlighted(const QString&)), this, SLOT(changeURL(const QString&)));
  connect(_showAtStartup, SIGNAL(clicked()), this, SLOT(updateSettings()));
  connect(_fileName, SIGNAL(textChanged(const QString&)), this, SLOT(deselectRecentFile()));
  connect(_fileName, SIGNAL(textChanged(const QString&)), this, SLOT(fileChanged(const QString&)));
  connect(_recentFileList, SIGNAL(selected(const QString&)), this, SLOT(open_I()));
}


KstQuickStartDialog::~KstQuickStartDialog() {
}


void KstQuickStartDialog::wizard_I() {
  close();
  _app->showDataWizard();
}


void KstQuickStartDialog::open_I() {
/* xxx
  if (_isRecentFile) {
    if (_app->slotFileOpenRecent(_fileName->url())) {
      // select the recently opened file...
      _app->selectRecentFile(_fileName->url());
      close();
    }
  } else if (_app->openDocumentFile(_fileName->url())) {
    close();
  }
*/
}


void KstQuickStartDialog::update() {
  //get the list of recent files
  _recentFileList->clear();
  _recentFileList->insertItems(0,_app->recentFiles());

  //by default, select a recent file
/* xxx  if (_recentFileList->numItemsVisible() > 0) {
    _recentFileList->setSelected(0, true);
  } */

  //update the startup checkbox
  _showAtStartup->setChecked(KstSettings::globalSettings()->showQuickStart);
}


void KstQuickStartDialog::show_I() {
  update();
  show();
  raise();
}


void KstQuickStartDialog::fileChanged(const QString& name) {
  QString file;

  file = name;
  file = file.trimmed();
  if (!file.isEmpty()) {
    _openFile->setEnabled(true);
  } else {
    _openFile->setEnabled(false);
  }
}


void KstQuickStartDialog::changeURL(const QString& name) {
  _fileName->blockSignals(true);
  _fileName->setUrl(name);
  _fileName->blockSignals(false);

  fileChanged(name);

  _isRecentFile = true;
}


void KstQuickStartDialog::updateSettings() {
  KstSettings::globalSettings()->showQuickStart = _showAtStartup->isChecked();
  KstSettings::globalSettings()->save();

  emit settingsChanged();
}


void KstQuickStartDialog::deselectRecentFile() {
  _recentFileList->clearSelection();
  _isRecentFile = false;
}

#include "kstquickstartdialog.moc"

