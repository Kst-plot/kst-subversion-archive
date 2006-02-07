/***************************************************************************
                    kstviewscalarsdialog_i.cpp  -  Part of KST
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

#include <kdebug.h>
#include <klocale.h>

#include "kstscalartable.h"
#include "kstrvector.h"
#include "kstdatacollection.h"
#include "kstviewscalarsdialog_i.h"

KstViewScalarsDialogI::KstViewScalarsDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl) :
  KstViewScalarsDialog(parent, name, modal, fl) {
   
  tableScalars = new KstScalarTable( this, "tableScalars" );
  tableScalars->setNumRows( 0 );
  tableScalars->setNumCols( 2 );
  tableScalars->setReadOnly( TRUE );
  tableScalars->setSorting( FALSE );
  tableScalars->setSelectionMode( QTable::Single );
  KstViewScalarsDialogLayout->addWidget( tableScalars );
    
  layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 
  QSpacerItem* spacer = new QSpacerItem( 440, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout2->addItem( spacer );
  
  Cancel = new QPushButton( this, "Cancel" );
  layout2->addWidget( Cancel );
  KstViewScalarsDialogLayout->addLayout( layout2 );
  
  connect( Cancel, SIGNAL( clicked() ), this, SLOT( close() ) );
  
  if( tableScalars->numCols( ) != 2 ) {  
    int i;
  
    for( i=0; i<tableScalars->numCols( );  ) {
      tableScalars->removeColumn( 0 );
    }
    tableScalars->insertColumns( 0, 2 );
  }
  
  tableScalars->setReadOnly( true );
  languageChange( );
}

KstViewScalarsDialogI::~KstViewScalarsDialogI() {
}

void KstViewScalarsDialogI::updateViewScalarsDialog() {
  QRect rect;
  int needed = KST::scalarList.count();
    
  tableScalars->setNumRows(needed);
  rect = tableScalars->horizontalHeader()->rect();
  tableScalars->setColumnWidth( 0, rect.width()/2 );
  tableScalars->setColumnWidth( 1, rect.width()/2 );
}

void KstViewScalarsDialogI::showViewScalarsDialog() {
  updateViewScalarsDialog();
  updateDefaults(0);

  show();
  raise();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstViewScalarsDialogI::languageChange()
{
  setCaption( i18n( "View Scalar Values" ) );
  Cancel->setText( tr("Close") );
  tableScalars->horizontalHeader()->setLabel( 0, tr("Scalar") );
  tableScalars->horizontalHeader()->setLabel( 1, tr("Value") );    
}

void KstViewScalarsDialogI::updateDefaults(int index) {
  Q_UNUSED( index )
}

#include "kstviewscalarsdialog_i.moc"
// vim: ts=2 sw=2 et
