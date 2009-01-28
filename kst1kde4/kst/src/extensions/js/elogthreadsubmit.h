/***************************************************************************
                                    elogthreadsubmit.h
                             -------------------
    begin                : Feb 09 2004
    copyright            : (C) 2004 The University of British Columbia
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

#ifndef ELOGTHREADSUBMIT_H
#define ELOGTHREADSUBMIT_H

#include <qobject.h>
#include <qstringlist.h>
#include <kio/scheduler.h>
#include <kstdebug.h>

class ElogThreadSubmit : public QObject {
  Q_OBJECT
  public:
    ElogThreadSubmit(
                     const QString& strHostname,
                     short port,
                     bool bIncludeCapture,
                     bool bIncludeConfiguration,
                     bool bIncludeDebugInfo,
                     const QString& strMessage,
                     const QString& strUserName,
                     const QString& strUserPassword,
                     const QString& strWritePassword,
                     const QString& strLogbook,
                     const QMap<QString, QString>& attributes,
                     const QStringList& attachments,
                     bool bSubmitAsHTML,
                     bool bSuppressEmail,
                     int iCaptureWidth,
                     int iCaptureHeight);

    virtual ~ElogThreadSubmit();
    virtual void doTransmit( );

  public slots:
    void result(KIO::Job *);
    void dataReq(KIO::Job *, QByteArray &);
    void data(KIO::Job *, const QByteArray &);

  protected:
    virtual bool doResponseError( const char* response, const QString& strDefault );
    virtual bool doResponseCheck( const char* response );
    virtual void doError( const QString& text, KstDebug::LogLevel level = KstDebug::Warning );

    void addAttachment( QDataStream& stream, const QString& boundary,
                        const QByteArray& byteArray, int iFileNumber, const QString& name );
    void addAttribute( QDataStream& stream, const QString& boundary,
                        const QString& tag, const QString& strValue, bool bEncode );

  protected:
    KIO::TransferJob        *_job;
    QByteArray              _byteArrayResult;
    QTextStream             _textStreamResult;

    QByteArray              _byteArrayAll;
    QDataStream             _dataStreamAll;
    QString                 _strHostname;
    QString                 _strType;
    QString                 _strMessage;
    QString                 _strUserName;
    QString                 _strUserPassword;
    QString                 _strWritePassword;
    QString                 _strLogbook;
    QMap<QString, QString>  _attributes;
    QStringList             _attachments;
    short                   _port;
    bool                    _bSubmitAsHTML;
    bool                    _bSuppressEmail;
    bool                    _bIncludeCapture;
    bool                    _bIncludeConfiguration;
    bool                    _bIncludeDebugInfo;
    int                     _iCaptureWidth;
    int                     _iCaptureHeight;
};

#endif
