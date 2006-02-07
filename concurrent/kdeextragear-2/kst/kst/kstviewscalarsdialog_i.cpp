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
#include <qwidget.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>

#include <kdebug.h>

#include "kstrvector.h"
#include "kstdatacollection.h"
#include "kstviewscalarsdialog_i.h"

KstViewScalarsDialogI::KstViewScalarsDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl) :
  KstViewScalarsDialog(parent, name, modal, fl) {

  int i;
  
  for( i=0; i<tableScalars->numCols( );  ) {
    tableScalars->removeColumn( 0 );
  }
  
  tableScalars->setReadOnly( true );
  
  tableScalars->insertColumns( 0, 2 );
  tableScalars->horizontalHeader()->setLabel( 0, tr("Scalar") );
  tableScalars->horizontalHeader()->setLabel( 1, tr("Value") );    
}

KstViewScalarsDialogI::~KstViewScalarsDialogI() {
}

void KstViewScalarsDialogI::updateViewScalarsDialog() {
kdDebug() << "update" << endl;
  int rows = tableScalars->numRows();
  int needed = KST::scalarList.count();
  tableScalars->setUpdatesEnabled(false);
  if (rows < needed) {
	  tableScalars->insertRows( rows, needed - rows );
  } else if (rows > needed) {
    while (needed < rows) {
      tableScalars->removeRow(rows--);
    }
  }
  
  int i = 0;
  for (KstScalarList::iterator it = KST::scalarList.begin(); it != KST::scalarList.end(); ++it) {
    tableScalars->setText( i, 0, (*it)->tagName() );
    tableScalars->setText( i++, 1, (*it)->label() );
  }
  tableScalars->setUpdatesEnabled(true);
  
  tableScalars->adjustColumn( 0 );
  tableScalars->adjustColumn( 1 );
}

void KstViewScalarsDialogI::showViewScalarsDialog() {
  updateViewScalarsDialog();
  updateDefaults(0);

  show();
  raise();
}

void KstViewScalarsDialogI::updateDefaults(int index) {
  Q_UNUSED( index )
}

#include "kstviewscalarsdialog_i.moc"
// vim: ts=2 sw=2 et
