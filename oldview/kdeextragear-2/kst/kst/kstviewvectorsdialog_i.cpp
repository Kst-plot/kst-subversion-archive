/***************************************************************************
                    kstviewvectorsdialog_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
                           (C) 2004 Andrew R. Walker
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

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtable.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

#include "vectorselector.h"
#include "kstvectortable.h"
#include "kstrvector.h"
#include "kstdatacollection.h"
#include "kstviewvectorsdialog_i.h"

KstViewVectorsDialogI::KstViewVectorsDialogI(QWidget* parent,
                                             const char* name,
                                             bool modal,
                                             WFlags fl) :
KstViewVectorsDialog(parent, name, modal, fl) {
  
  tableVectors = new KstVectorTable( this, "tableVectors" );
  tableVectors->setNumRows( 0 );
  tableVectors->setNumCols( 1 );
  tableVectors->setReadOnly( TRUE );
  tableVectors->setSorting( FALSE );
  tableVectors->setSelectionMode( QTable::Single );
  KstViewVectorsDialogLayout->addWidget( tableVectors );
  
  layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 
  QSpacerItem* spacer = new QSpacerItem( 440, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout2->addItem( spacer );
  
  Cancel = new QPushButton( this, "Cancel" );
  layout2->addWidget( Cancel );
  KstViewVectorsDialogLayout->addLayout( layout2 );
  
  connect( Cancel, SIGNAL( clicked() ), this, SLOT( close() ) );
  connect( vectorSelector, SIGNAL( selectionChanged(const QString&) ), this, SLOT( vectorChanged(const QString&) ) );
  connect( vectorSelector, SIGNAL( newVectorCreated(const QString&) ), this, SLOT( vectorChanged(const QString&) ) );
  
  if( tableVectors->numCols( ) != 1 ) {  
    int i;
    
    for( i=0; i<tableVectors->numCols( );  ) {
      tableVectors->removeColumn( 0 );
    }
    tableVectors->insertColumns( 0, 1 );
  }
  
  tableVectors->setReadOnly( true );
  languageChange( );
}

KstViewVectorsDialogI::~KstViewVectorsDialogI() {
}

void KstViewVectorsDialogI::updateViewVectorsDialog() {
  QString strVector;
    
  vectorSelector->update();
  
  strVector = vectorSelector->selectedVector();
  tableVectors->setVector(strVector);
  
  updateViewVectorsDialog(strVector);
}

void KstViewVectorsDialogI::updateViewVectorsDialog(const QString& strVector) {
  QRect rect;
  int needed = 0;
  KstVectorPtr vector;
  
  vector = *KST::vectorList.findTag(strVector);
  if( vector != NULL ) {
    needed = vector->length();
  }
  
  tableVectors->setNumRows(needed);
  rect = tableVectors->horizontalHeader()->rect();
  tableVectors->setColumnWidth( 0, rect.width() );
}

void KstViewVectorsDialogI::showViewVectorsDialog() {
  updateViewVectorsDialog();
  updateDefaults(0);
  
  show();
  raise();
}

void KstViewVectorsDialogI::vectorChanged(const QString& strVector) {
  updateViewVectorsDialog(strVector);
  tableVectors->setVector(strVector);
  tableVectors->update();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewVectorsDialogI::languageChange()
{
  setCaption( i18n( "View Vector Values" ) );
  tableVectors->horizontalHeader()->setLabel( 0, tr("Values") );
  Cancel->setText( tr("Close") );
}

void KstViewVectorsDialogI::updateDefaults(int index) {
  Q_UNUSED( index )
}

#include "kstviewvectorsdialog_i.moc"
// vim: ts=2 sw=2 et
