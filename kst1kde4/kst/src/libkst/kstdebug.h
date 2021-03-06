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

#include <QDateTime>
#include <QLinkedList>
#include <QObject>
#include <QMutex>
#include <QPointer>

//#include <ksttimers.h> xxx

#include "kst_export.h"

//
// this class has to be threadsafe...
//

class KST_EXPORT KstDebug : public QObject {
  Q_OBJECT

  public:
    KstDebug();
    ~KstDebug();
    
    enum LogLevel { Unknown = 0, Notice = 1, Warning = 2, Error = 4, Debug = 8, None = 16384 };
    struct LogMessage {
      QDateTime date;
      QString msg;
      LogLevel level;
    };
    static KstDebug *self();

    void clear();
    void log(const QString& msg, LogLevel level = Notice);
    void setLimit(bool applyLimit, int limit);
    QString text();
    void sendEmail();

    int logLength() const;
    QLinkedList<LogMessage> messages() const;
    KstDebug::LogMessage message(unsigned n) const;
    QStringList dataSourcePlugins() const;
    QString label(LogLevel level) const;
    const QString& kstVersion() const;
    const QString& kstRevision() const;

    int limit() const;

    bool hasNewError() const;
    void clearHasNewError();

  protected:
    friend class KstApp;
    void setHandler(QObject *handler);

  private:
    static KstDebug *_self;
    QLinkedList<LogMessage> _messages;
    bool _applyLimit;
    bool _hasNewError;
    int _limit;
    mutable QMutex _lock;
    QPointer<QObject> _handler;
    QString _kstVersion;
    QString _kstRevision;
};

#endif
