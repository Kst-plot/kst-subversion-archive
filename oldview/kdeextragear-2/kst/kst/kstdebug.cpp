/***************************************************************************
                                kstdebug.cpp
                             -------------------
    begin                : Mar 07 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "kstdatasource.h"
#include "kstdebug.h"
#include "kstversion.h"

#include <kapplication.h>
#include <klocale.h>
#include <qdatetime.h>
#include <qvaluelist.h>

KStaticDeleter<KstDebug> sd;

KstDebug *KstDebug::self() {
  if (!_self) {
    _self = sd.setObject(_self, new KstDebug);
  }

  return _self;
}

KstDebug::KstDebug() {
  _bApplyLimit = false;
  _iLimit      = 10000;
}

KstDebug::~KstDebug() {
}

KstDebug *KstDebug::_self = 0L;

QStringList KstDebug::dataSourcePlugins() const {
  return KstDataSource::pluginList();
}

void KstDebug::log(const QString& msg, LogLevel level) {
  LogMessage message;
  
  message.date  = QDateTime::currentDateTime();
  message.msg   = msg;
  message.level = level;
      
  _messages.append( message );
  if( _bApplyLimit ) {
    if( (int)_messages.size( ) > _iLimit ) {
      QValueListIterator<LogMessage> first;
      QValueListIterator<LogMessage> last;
      
      first = _messages.begin( );
      last  = first;
      last += _messages.size( ) - _iLimit; 
      
      _messages.erase( first, last );
    }
  }
  
  emit logAdded( );
}

void KstDebug::clear() {
  _messages.clear(); 
}

QString KstDebug::getLabel(LogLevel level) const {
  QString str;
  
  switch (level) {
    case Notice:
      str = i18n("Notice");
      break;
    case Warning:
      str = i18n("Warning");
      break;
    case Error:
      str = i18n("Error");
      break;
    case Debug:
      str = i18n("Debug");
      break;
    default:
      str = i18n("Other");
      break;
  }    
    
  return str;
}

void KstDebug::getText(QString& body) {
  unsigned int i;
  
  body += i18n("Kst version %1\n\n\n").arg(KSTVERSION);

  body += i18n("Kst log:\n");
  
  for( i=0; i<_messages.count(); i++ ) {
    body += i18n("%1 %2: %3\n").arg(_messages[i].date.toString()).arg(getLabel(_messages[i].level)).arg(_messages[i].msg);
  }
  
  body += i18n("\n\nData-source plugins:");
  QStringList dsp = dataSourcePlugins();
  for (QStringList::ConstIterator it = dsp.begin(); it != dsp.end(); ++it) {
    body += '\n';
    body += *it;
  }
  body += "\n\n";
}

void KstDebug::setLimit( bool bApplyLimit, int iLimit ) {
  _bApplyLimit = bApplyLimit;
  _iLimit      = iLimit;
}

void KstDebug::sendEmail() {
  QString 			body;
  
  getText( body );

  kapp->invokeMailer(QString::null, QString::null, QString::null, i18n("Kst Debugging Information"), body);
}


#include "kstdebug.moc"
// vim: ts=2 sw=2 et
