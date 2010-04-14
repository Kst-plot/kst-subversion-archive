/**************************************************************************
              kstdebugdialog.cpp - debug dialog: inherits designer dialog
                             -------------------
    begin                :  2004
    copyright            : (C) 2004 The University of British Columbia
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

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QListWidgetItem>

#include "kst.h"
#include "kstdatacollection.h"
#include "kstdebugdialog.h"
#include "kstdoc.h"
#include "kstlogwidget.h"

KstDebugDialog::KstDebugDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl) {
  setupUi(this);

  _log = new KstLogWidget(TabPage, "logwidget");
  _log->setDebug(KstDebug::self());

// xxx  TabPageLayout->addMultiCellWidget(_log, 0, 0, 0, 2);

  const QStringList& pl = KstDataSource::pluginList();
  QStringList::const_iterator it;
  
  for (it = pl.begin(); it != pl.end(); ++it) {
    new QListWidgetItem(*it, _dataSources);
  }

  _buildInfo->setText(tr("<h1>Kst</h1> version %1 (%2)").arg(KSTVERSION).arg(KstDebug::self()->kstRevision()));

// xxx  _dataSources->setAllColumnsShowFocus(true);

  connect(KstApp::inst()->document(), SIGNAL(logAdded(const KstDebug::LogMessage&)), _log, SLOT(logAdded(const KstDebug::LogMessage&)));
  connect(KstApp::inst()->document(), SIGNAL(logCleared()), _log, SLOT(clear()));
  connect(_email, SIGNAL(clicked()), this, SLOT(email()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(clear()));
  connect(checkBoxShowDebug, SIGNAL(toggled(bool)), _log, SLOT(setShowDebug(bool)));
  connect(checkBoxShowWarning, SIGNAL(toggled(bool)), _log, SLOT(setShowWarning(bool)));
  connect(checkBoxShowNotice, SIGNAL(toggled(bool)), _log, SLOT(setShowNotice(bool)));
  connect(checkBoxShowError, SIGNAL(toggled(bool)), _log, SLOT(setShowError(bool)));
}


KstDebugDialog::~KstDebugDialog() {
}


void KstDebugDialog::show_I() {
/* xxx
  QListWidgetItem::iterator it(_dataSources);

  KST::dataSourceList.lock().readLock();

  while (it.current()) {
    while (it.current()->childCount() > 0) {
      delete it.current()->firstChild();
    }

    for (KstDataSourceList::Iterator i = KST::dataSourceList.begin(); i != KST::dataSourceList.end(); ++i) {
      (*i)->readLock();
      if ((*i)->sourceName() == it.current()->text(0)) {
        new QListWidgetItem(it.current(), QString::null, (*i)->fileName());
      }
      (*i)->unlock();
    }
    ++it;
  }

  KST::dataSourceList.lock().unlock();
*/
  QDialog::show();
}


void KstDebugDialog::clear() {
  KstDebug::self()->clear();
  _log->clear();
}


void KstDebugDialog::email() {
  KstDebug::self()->sendEmail();
}


KstLogWidget *KstDebugDialog::logWidget() const {
  return _log;
}

#include "kstdebugdialog.moc"
