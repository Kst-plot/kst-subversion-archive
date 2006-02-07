/***************************************************************************
                          ksteventmonitorentry.h  -  description
                             -------------------
    begin                : Tue Apr 6 2004
    copyright            : (C) 2000 by arw
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

#ifndef KSTEVENTMONITORENTRY_H
#define KSTEVENTMONITORENTRY_H

#include <math.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qvaluelist.h>
#include <qdom.h>
#include <qdict.h>
#include "kstobject.h"
#include "kstscalar.h"
#include "kstdebug.h"

namespace Equation {
  class Node;
  class Context;
}

class EventMonitorEntry : public QObject {
public:
  EventMonitorEntry();
  EventMonitorEntry(const EventMonitorEntry &e);
  EventMonitorEntry(QDomElement &e);
  ~EventMonitorEntry();
  
  bool needToEvaluate();
  bool compile();
  bool evaluate(Equation::Context& ctx);
  void save(QTextStream &ts);
  void addRef( );
  void release( );
  void log(const int& iIndex);
  QString getEvent( ) { return _strEvent; }
  QString getDescription( ) { return _strDescription; }
  KstDebug::LogLevel	getLevel( ) { return _level; }
  Equation::Node* getExpression( ) { return _pExpression; }
  bool getLogKstDebug( ) { return _bLogKstDebug; }
  void setEvent( QString str ) { _strEvent = str; }
  void setDescription( QString str ) { _strDescription = str; }
  void setLevel( KstDebug::LogLevel level ) { _level = level; }
  void setExpression( Equation::Node* pExpression ) { _pExpression = pExpression; }
  void setLogKstDebug( bool bLogKstDebug ) { _bLogKstDebug = bLogKstDebug; }
  
  
public slots:    
  void logImmediately();

private:
  QValueList<int>		  _indexArray;
  QTime							  _timeLastEvent;
  QTime			          _timeLastLogged;
  QString 						_strEvent;
  QString						  _strDescription;
  KstDebug::LogLevel  _level;
  Equation::Node*		  _pExpression;
  bool                _bLogKstDebug;
  int							    _iRefCount;
};

#endif
