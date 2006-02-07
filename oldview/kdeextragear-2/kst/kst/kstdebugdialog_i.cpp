/**************************************************************************
              kstdebugdialog_i.cpp - debug dialog: inherits designer dialog
                             -------------------
    begin                :  2004
    copyright            : (C) 2004 by Andrew Walker
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

// include files for Qt
#include <qdir.h>
#include <qlistview.h>
#include <qfileinfo.h>
#include <qwidget.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfontdatabase.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <klistbox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qlayout.h>

// include files for KDE
#include <klocale.h>
#include <kcolorbutton.h>
#include <kdualcolorbutton.h>
#include <kmessagebox.h>
#include <kfontcombo.h>
#include <kdebug.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtable.h>

#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstlegend.h"
#include "kstdebugdialog_i.h"
#include "kstsettings.h"
#include "kstversion.h"
#include "kstlogtable.h"

/*
 *  Constructs a KstDebugDialogI which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KstDebugDialogI::KstDebugDialogI(QWidget* parent,
                               const char* name, bool modal, WFlags fl)
: DebugDialog(parent, name, modal, fl ) {
  QStringList	labels;
  int i;
  
  _table = new KstLogTable( TabPage, "_table" );
  _table->setDebug(KstDebug::self());
  
  TabPageLayout->addMultiCellWidget( _table, 0, 0, 0, 2 );
 
  const QStringList& pl = KstDataSource::pluginList();
  
  for (QStringList::ConstIterator it = pl.begin(); it != pl.end(); ++it) {
    new QListViewItem(_dataSources, *it);
  }
  
  _buildInfo->setText(i18n("<h1>Kst</h1> version %1").arg(KSTVERSION));
    
  if( _table->numCols( ) != 3 ) {  
    for( i=0; i<_table->numCols( );  ) {
      _table->removeColumn( 0 );
    }
    _table->insertColumns( 0, 3 );
  }
  
  labels.append(i18n(" "));
  labels.append(i18n("Date"));
  labels.append(i18n("Message"));
  _table->setColumnLabels( labels );
  _table->setReadOnly( true );
  _table->setRowMovingEnabled( false );
  
  connect(KstDebug::self(), SIGNAL(logAdded()), this, SLOT(logAdded()));
  connect(_email, SIGNAL(clicked()), this, SLOT(email()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(clear()));
  connect(checkBoxHideDebug,   SIGNAL(clicked()), this, SLOT(messagesDebug()));
  connect(checkBoxHideWarning, SIGNAL(clicked()), this, SLOT(messagesWarning()));
  connect(checkBoxHideOther,   SIGNAL(clicked()), this, SLOT(messagesOther()));
  connect(checkBoxHideNotice,  SIGNAL(clicked()), this, SLOT(messagesNotice()));
  connect(checkBoxHideError,   SIGNAL(clicked()), this, SLOT(messagesError()));
  
  _bFirstNotification = true;
}

KstDebugDialogI::~KstDebugDialogI() {
}

void KstDebugDialogI::show_I() {   
  // Inefficient but shouldn't be a factor
  QListViewItemIterator it(_dataSources);
  while (it.current()) {
    while (it.current()->childCount() > 0) {
      delete it.current()->firstChild();
    }
    
    for (KstDataSourceList::Iterator i = KST::dataSourceList.begin(); i != KST::dataSourceList.end(); ++i) {
      if ((*i)->sourceName() == it.current()->text(0)) {
        new QListViewItem(it.current(), QString::null, (*i)->fileName());
      }
    }
    ++it;
  }
  
  update();
  
  QDialog::show();
}

void KstDebugDialogI::logAdded() {
  update();

  _table->setMoveToLast();
  
  if( _bFirstNotification ) {
    QRect rect;
    
    rect = _table->horizontalHeader()->rect();
    _table->setColumnWidth( 0,  3*rect.width()/20 );
    _table->setColumnWidth( 1,  5*rect.width()/20 );
    _table->setColumnWidth( 2, 12*rect.width()/20 );
    
    _bFirstNotification = false;
  }
}

void KstDebugDialogI::messagesDebug() {
  _table->hideRows( KstDebug::Debug, checkBoxHideDebug->isChecked(), 0 );
}

void KstDebugDialogI::messagesWarning() {
  _table->hideRows( KstDebug::Warning, checkBoxHideWarning->isChecked(), 0 );
}

void KstDebugDialogI::messagesNotice() {
  _table->hideRows( KstDebug::Notice, checkBoxHideNotice->isChecked(), 0 );
}

void KstDebugDialogI::messagesOther() {
  _table->hideRows( KstDebug::None, checkBoxHideOther->isChecked(), 0 );
}

void KstDebugDialogI::messagesError() {
  _table->hideRows( KstDebug::Error, checkBoxHideError->isChecked(), 0 );
}

void KstDebugDialogI::clear() {
  KstDebug::self()->clear();
  _table->clear();
}

void KstDebugDialogI::email() {
  KstDebug::self()->sendEmail();
}

#include "kstdebugdialog_i.moc"
// vim: et ts=2 sw=2
