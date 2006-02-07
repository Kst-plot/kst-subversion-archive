/***************************************************************************
                                 kstdebug.h
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

#ifndef KSTDEBUG_H
#define KSTDEBUG_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>

#include <kstaticdeleter.h>

class KstDebug : public QObject {
  Q_OBJECT

  friend class KStaticDeleter<KstDebug>;

  public:
    enum LogLevel { Notice = 0, Warning, Error, Debug, None = 9999 };
    struct LogMessage {
      QDateTime	date;
      QString 	msg;
      LogLevel 	level;
    };
    static KstDebug* self();

    void clear();
    void log(const QString& msg, LogLevel level = Notice);
    void setLimit( bool bApplyLimit, int iLimit );   
    void getText(QString& body);
    void sendEmail();
    
    QValueList<LogMessage>* messages( ) { return &_messages; }
    QStringList dataSourcePlugins() const;
    QString getLabel(LogLevel level) const;

  signals: 
    void logAdded();

  private:
    KstDebug();
    ~KstDebug();
    
    static KstDebug *_self;
    QValueList<LogMessage> _messages;
    bool  _bApplyLimit;
    int 	_iLimit;
};


#endif

// vim: ts=2 sw=2 et
