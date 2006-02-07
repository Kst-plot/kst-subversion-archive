/***************************************************************************
                    ksteventmonitor_i.cpp  -  Part of KST
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
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qtable.h>
#include <qwidget.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <kstaticdeleter.h>
#include "vectorselector.h"
#include "scalarselector.h"
#include "kstrvector.h"
#include "kstdatacollection.h"
#include "ksteventmonitor_i.h"
#include "kstdebug.h"
#include "ksteventmonitorentry.h"
#include "kstdoc.h"

KstEventMonitorI* KstEventMonitorI::_inst = 0L;
static KStaticDeleter<KstEventMonitorI> _eventInst;

KstEventMonitorI* KstEventMonitorI::globalInstance() {
  if (!_inst) {
    _inst = _eventInst.setObject(new KstEventMonitorI);
  }
  
  return _inst;
}

KstEventMonitorI::KstEventMonitorI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl) :
  EventMonitor(parent, name, modal, fl) {
  QStringList	labels;

  _bSetWidths = FALSE;
  
  labels.append(i18n("Log as:"));
  labels.append(i18n("Expression:"));
  labels.append(i18n("Description:"));
  
  tableEvents = new KstEventTable( this, "tableEvents" );
  tableEvents->setNumRows( 0 );
  tableEvents->setNumCols( 3 );
  tableEvents->setReadOnly( TRUE );
  tableEvents->setSorting( FALSE );     
  tableEvents->setRowMovingEnabled( FALSE );
  tableEvents->setColumnLabels( labels );
  tableEvents->setSelectionMode( QTable::SingleRow );
  tableEvents->setEventMonitors( &_eventMonitors );
  layoutGrid->addWidget( tableEvents, 1, 0 );

  connect(vectorSelector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(vectorSelectorEq, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect( scalarSelectorEq, SIGNAL( newScalarCreated() ), this, SIGNAL(modified()));
  connect( buttonGroupLog, SIGNAL( clicked(int) ), this, SLOT( logChanged(int) ) );
  connect( checkBoxDebug, SIGNAL(clicked()), this, SLOT(logCheckChanged()));
  connect( Close, SIGNAL( clicked() ), this, SLOT( close() ) );
  connect( pushButtonAdd, SIGNAL( clicked() ), this, SLOT( addEvent() ) );
  connect( pushButtonDelete, SIGNAL( clicked() ), this, SLOT( deleteEvent() ) );
  connect( pushButtonAccept, SIGNAL( clicked() ), this, SLOT( acceptChanges() ) );
  connect( pushButtonDiscard, SIGNAL( clicked() ), this, SLOT( discardChanges() ) );
  connect( lineEditEquation, SIGNAL( textChanged(const QString&) ), this, SLOT( expressionChanged(const QString&) ) );
  connect( lineEditDescription, SIGNAL( textChanged(const QString&) ), this, SLOT( descriptionChanged(const QString&) ) );
  connect( tableEvents, SIGNAL( currentChanged(int, int) ), this, SLOT( changedEvent(int, int) ) );
  connect( tableEvents, SIGNAL( selectionChanged() ), this, SLOT( changedSelection() ) );
  connect( vectorSelector, SIGNAL( selectionChanged(const QString&) ), this, SLOT( vectorChanged(const QString&) ) );
  connect( vectorSelector, SIGNAL( newVectorCreated(const QString&) ), this, SLOT( vectorChanged(const QString&) ) );
  connect( vectorSelectorEq, SIGNAL( selectionChangedLabel(const QString&)), lineEditEquation, SLOT( insert(const QString&) ) );
  connect( scalarSelectorEq, SIGNAL( selectionChangedLabel(const QString&)), lineEditEquation, SLOT( insert(const QString&) ) );
  connect( pushButtonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
    
  languageChange( );
  
  setChanged( FALSE );
}

KstEventMonitorI::~KstEventMonitorI() {
}

void KstEventMonitorI::update() {
  vectorSelector->update();
  vectorSelectorEq->update();
  scalarSelectorEq->update();
}

void KstEventMonitorI::show_I() {
  QRect	rect;
  
  update();
  updateDefaults(0);

  if( !_bSetWidths ) {
    rect = tableEvents->horizontalHeader()->rect();
    tableEvents->setColumnWidth( 0, 1*rect.width()/5 );
    tableEvents->setColumnWidth( 1, 2*rect.width()/5 );
    tableEvents->setColumnWidth( 2, 2*rect.width()/5 );
    _bSetWidths = TRUE;
  }
  show();
  raise();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KstEventMonitorI::languageChange()
{
  
}

void KstEventMonitorI::addEvent() {
  EventMonitorEntry	event;

  _eventMonitors.append( event );
  
  updateTable( );
  
  if( _eventMonitors.size() == 1 ) {
    enableEditing( TRUE );  
  }
  tableEvents->setCurrentCell( tableEvents->numRows( ), 0 );
}

void KstEventMonitorI::deleteEvent() {
  int row;
  
  row = tableEvents->currentRow( );
  if( row >= 0 && row < (int)_eventMonitors.size() ) {
    _eventMonitors.erase( _eventMonitors.at(row) );  
    updateTable( );
  }
  
  if( _eventMonitors.size() == 0 ) {
    enableEditing( FALSE );  
  }
}

void KstEventMonitorI::acceptChanges() {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KstRVectorPtr  vector;
  
  vector = *(rvl.findTag( _strVector ));
  if( vector != NULL ) {
    vector->setEventMonitors( &_eventMonitors );
  }
  
  setChanged( FALSE );
}

void KstEventMonitorI::discardChanges() {
  displayEvents( _strVector );
  
  setChanged( FALSE );
}

void KstEventMonitorI::setChanged( bool bChanged ) {
  if( bChanged != _bChanged ) {
    pushButtonAccept->setEnabled( bChanged );
    pushButtonDiscard->setEnabled( bChanged );
    pushButtonApply->setEnabled( bChanged );
    
    _bChanged = bChanged;
  }
}

void KstEventMonitorI::logCheckChanged() {
  bool bLogKstDebug = FALSE;
  int row;
 
  row = tableEvents->currentRow( );
  if( row >= 0 && row < (int)_eventMonitors.size() ) {  
    if( checkBoxDebug->isChecked()) {
      bLogKstDebug = TRUE;
    }
    
    if( bLogKstDebug != _eventMonitors[row].getLogKstDebug( ) ) {
      _eventMonitors[row].setLogKstDebug( bLogKstDebug );
      setChanged( TRUE );
    }
  }
}

void KstEventMonitorI::logChanged( int id ) {
  KstDebug::LogLevel	level;
  int row;
 
  row = tableEvents->currentRow( );
  if( row >= 0 && row < (int)_eventMonitors.size() ) {
    switch (id ) {
    case 1:
      level = KstDebug::Notice;
      break;
    case 2:
      level = KstDebug::Warning;
      break;
    case 3:
      level = KstDebug::Error;
      break;
    default:
      level = KstDebug::None;
      break;
    }
    if( level != _eventMonitors[row].getLevel( ) ) {
      _eventMonitors[row].setLevel( level );
      tableEvents->updateCell( row, 0 );  
      
      setChanged( TRUE );
    }
  }
}

void KstEventMonitorI::expressionChanged( const QString& str ) {
  int row;
    
  row = tableEvents->currentRow( );
  if( row >= 0 && row < (int)_eventMonitors.size() ) {
    _eventMonitors[row].setEvent( str );
    tableEvents->updateCell( row, 1 );
    
    setChanged( TRUE );
  }
}

void KstEventMonitorI::descriptionChanged( const QString& str ) {
  int row;
    
  row = tableEvents->currentRow( );
  if( row >= 0 && row < (int)_eventMonitors.size() ) {
    _eventMonitors[row].setDescription( str );
    tableEvents->updateCell( row, 2 );
    
    setChanged( TRUE );
  }
}

void KstEventMonitorI::changedSelection() {
  int row;
  
  row = tableEvents->currentRow();
    
  changedEvent( row, 0 );
}

void KstEventMonitorI::changedEvent(int row, int col) {
  Q_UNUSED( col );
  
  bool bChanged = _bChanged;
  
  if( row >= 0 && row < (int)_eventMonitors.size() ) {
    lineEditEquation->setText( _eventMonitors[row].getEvent( ) );
    lineEditDescription->setText( _eventMonitors[row].getDescription( ) );    
    checkBoxDebug->setChecked(_eventMonitors[row].getLogKstDebug());
    
    switch( _eventMonitors[row].getLevel( ) ) {
    case KstDebug::Notice:
      radioButtonLogNotice->setChecked( TRUE );
      break;
    case KstDebug::Warning:
      radioButtonLogWarning->setChecked( TRUE );
      break;
    case KstDebug::Error:
      radioButtonLogError->setChecked( TRUE );
      break;
    default:
      radioButtonLogNotice->setChecked( TRUE );
      break;
    }
  }
    
  if( !bChanged ) {
    setChanged( FALSE );
  }
}

void KstEventMonitorI::displayEvents( const QString& strVector ) {
  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KstRVectorPtr rvector;
  int i;
  
  rvector = *rvl.findTag(strVector);
  if( rvector != NULL ) {
    _eventMonitors = *(rvector->getEventMonitors());
    tableEvents->setEventMonitors( &_eventMonitors );
    _strVector = strVector;
    for( i=0; i<(int)_eventMonitors.size(); i++ ) {
      _eventMonitors[i].setExpression( NULL );
    }
    pushButtonAdd->setEnabled( TRUE );
    pushButtonDelete->setEnabled( TRUE );
    enableEditing( TRUE );  
  } else {
    _strVector = "";
    tableEvents->setEventMonitors( NULL );
    pushButtonAdd->setEnabled( FALSE );
    pushButtonDelete->setEnabled( FALSE );
    enableEditing( FALSE );  
  }
}

void KstEventMonitorI::vectorChanged(const QString& strVector) {
  QString    			str;
  int							iButton;
  
  if( _bChanged ) {
    iButton = QMessageBox::information( this, 
                                        i18n("Accept changes?"),
                                        i18n("You have made changes to the event monitor entries for the vector %1\nDo you wish to accept or discard your changes?").arg( _strVector ),
                                        i18n("Accept"),
                                        i18n("Discard") );
    if( iButton == 0 ) {
      acceptChanges( );
    }
  }
  
  lineEditEquation->setText( "" );
  displayEvents( strVector );
  
  if( _eventMonitors.size( ) > 0 ) {
    enableEditing( TRUE );    
    
    tableEvents->setCurrentCell( 0, 0 );
  } else {
    enableEditing( FALSE );    
  }
  
  setChanged( FALSE );
}

void KstEventMonitorI::enableEditing( bool bEnable ) {
  buttonGroupLog->setEnabled( bEnable );    
  lineEditEquation->setEnabled( bEnable );    
  lineEditDescription->setEnabled( bEnable );
}

void KstEventMonitorI::updateTable( ) {
  tableEvents->setNumRows( _eventMonitors.size() );
}

void KstEventMonitorI::updateDefaults(int index) {
  Q_UNUSED( index )
}

void KstEventMonitorI::apply() {
  acceptChanges();
}

#include "ksteventmonitor_i.moc"
// vim: ts=2 sw=2 et
