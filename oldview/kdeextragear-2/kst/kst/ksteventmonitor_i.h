/***************************************************************************
                       ksteventmonitor_i.h  -  Part of KST
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

#ifndef KSTEVENTMONITORI_H
#define KSTEVENTMONITORI_H

#include "eventmonitor.h"
#include "ksteventtable.h"

class KstEventMonitorI : public EventMonitor {
  Q_OBJECT
public:
  KstEventMonitorI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstEventMonitorI();
  KstEventTable* tableEvents;

protected:

protected slots:
  virtual void languageChange();
  virtual void vectorChanged(const QString& str);
  virtual void addEvent();
  virtual void deleteEvent();
  virtual void acceptChanges();
  virtual void discardChanges();
  virtual void changedSelection();
  virtual void changedEvent(int row, int col);
  virtual void expressionChanged( const QString& str );
  virtual void descriptionChanged( const QString& str );
  virtual void logCheckChanged( );
  virtual void logChanged( int id );
  virtual void apply();
  
public slots:
  void update();
  void show_I();
  void updateDefaults(int index=0);
  static KstEventMonitorI *globalInstance();
  
private slots:

signals:
  void modified();
  
private:
  EventMonitors _eventMonitors;
  QString			  _strVector;
  bool          _bChanged;
  bool				  _bSetWidths;
  static KstEventMonitorI* _inst;  
  
  void enableEditing( bool bEnable );
  void displayEvents( const QString& strVector );
  void updateTable( );
  void setChanged( bool bChanged );
};


#endif
// vim: ts=2 sw=2 et
