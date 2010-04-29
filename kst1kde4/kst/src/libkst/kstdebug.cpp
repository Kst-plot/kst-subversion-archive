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

#include <QApplication>

#include "kstdatasource.h"
#include "kstdebug.h"
#include "kstrevision.h"
#include "logevents.h"

KstDebug *KstDebug::_self = 0L;
static QMutex soLock;

KstDebug *KstDebug::self() {
  QMutexLocker ml(&soLock);
  if (!_self) {
    _self = new KstDebug;
  }

  return _self;
}


KstDebug::KstDebug() : QObject() {
  _applyLimit = false;
  _limit = 10000;
  _kstRevision = QString::fromLatin1(KSTREVISION);
// xxx  _kstVersion = QString::fromLatin1(KSTVERSION);
  _hasNewError = false;
}


KstDebug::~KstDebug() {
}


int KstDebug::limit() const {
  QMutexLocker ml(&_lock);
  
  return _limit;
}


QStringList KstDebug::dataSourcePlugins() const {
  return KstDataSource::pluginList();
}


void KstDebug::setHandler(QObject *handler) {
  _handler = handler;
}


void KstDebug::log(const QString& msg, LogLevel level) {
  QMutexLocker ml(&_lock);
  LogMessage message;

  message.date  = QDateTime::currentDateTime();
  message.msg   = msg;
  message.level = level;

  _messages.append(message);
  if (_applyLimit && int(_messages.size()) > _limit) {
    QLinkedList<LogMessage>::iterator msgFirst = _messages.begin();
    QLinkedList<LogMessage>::iterator msgLast = msgFirst;

    msgLast += _messages.size() - _limit;
    _messages.erase(msgFirst, msgLast);
  }

  if (level == Error) {
    _hasNewError = true;
  }

  if (_handler) {
    LogEvent *e = new LogEvent(LogEvent::LogAdded);
    
    e->_msg = message;
    QApplication::postEvent(_handler, e);
  }
}


void KstDebug::clear() {
  clearHasNewError(); // has to be before the lock is acquired
  
  QMutexLocker ml(&_lock);
  
  _messages.clear(); 
  
  LogEvent *e = new LogEvent(LogEvent::LogCleared);
  
  QApplication::postEvent(_handler, e);
}


QString KstDebug::label(LogLevel level) const {
  switch (level) {
    case Notice:
      return QObject::tr("log level notice", "Notice");
    case Warning:
      return QObject::tr("log level warning", "Warning");
    case Error:
      return QObject::tr("log level error", "Error");
    case Debug:
      return QObject::tr("log level debug", "Debug");
    default:
      return QObject::tr("log level other", "Other");
  }
}


QString KstDebug::text() {
  QMutexLocker ml(&_lock);
  QString body = QObject::tr("Kst version %1\n\n\nKst log:\n").arg( _kstVersion );
  QLinkedList<LogMessage>::const_iterator i;
  QStringList::const_iterator it;
  QStringList dsp;

  for (i =_messages.begin(); i != _messages.end(); ++i) {
// xxx    body += QObject::tr("date leveltext: message", "%1 %2: %3\n").arg(KGlobal::locale()->formatDateTime(i->date)).arg(label(i->level)).arg(i->msg);
  }

  body += QObject::tr("\n\nData-source plugins:");
  dsp = dataSourcePlugins();

  for (it = dsp.begin(); it != dsp.end(); ++it) {
    body += '\n';
    body += *it;
  }
  body += "\n\n";

  return body;
}


void KstDebug::setLimit(bool applyLimit, int limit) {
  QMutexLocker ml(&_lock);
  
  _applyLimit = applyLimit;
  _limit = limit;
}


void KstDebug::sendEmail() {
//  kapp->invokeMailer(QString::null, QString::null, QString::null, QObject::tr("Kst Debugging Information"), text());
}


QLinkedList<KstDebug::LogMessage> KstDebug::messages() const {
  QMutexLocker ml(&_lock);

  return _messages;
}


KstDebug::LogMessage KstDebug::message(unsigned n) const {
  QMutexLocker ml(&_lock);
  unsigned i;
  
  if (n < (unsigned)_messages.count()) {
    QLinkedList<LogMessage>::const_iterator it;

    it = _messages.begin();
    for (i=0; i<n; ++i) {
      ++it;
    }
    
    return *it;
  }
  
  return KstDebug::LogMessage();
}


int KstDebug::logLength() const {
  QMutexLocker ml(&_lock);
  
  return _messages.size();
}


const QString& KstDebug::kstVersion() const {
  QMutexLocker ml(&_lock);
  
  return _kstVersion;
}


const QString& KstDebug::kstRevision() const {
  QMutexLocker ml(&_lock);
  
  return _kstRevision;
}


bool KstDebug::hasNewError() const {
  QMutexLocker ml(&_lock);
  
  return _hasNewError;
}


void KstDebug::clearHasNewError() {
  QMutexLocker ml(&_lock);
  
  _hasNewError = false;
}

#include "kstdebug.moc"
