/***************************************************************************
                                   elogthreadsubmit.cpp
                             -------------------
    begin                : Nov 22 2007
    copyright            : (C) 2007 The University of British Columbia
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <klocale.h>
#include <kmdcodec.h>

#include "elogthreadsubmit.h"

#define private public
#define protected public
#include <kjsembed/kjsembedpart.h>
#undef protected
#undef private

#include "js.h"

#include <kst.h>
#include <kstevents.h>

ElogThreadSubmit::ElogThreadSubmit(
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
                                  int iCaptureHeight)
: _textStreamResult(_byteArrayResult, IO_ReadWrite), _dataStreamAll(_byteArrayAll, IO_ReadWrite) {
  _bIncludeCapture        = bIncludeCapture;
  _bIncludeConfiguration  = bIncludeConfiguration;
  _bIncludeDebugInfo      = bIncludeDebugInfo;
  _strHostname            = strHostname;
  _port                   = port;
  _strMessage             = strMessage;
  _strUserName            = strUserName;
  _strUserPassword        = strUserPassword;
  _strWritePassword       = strWritePassword;
  _strLogbook             = strLogbook;
  _attributes             = attributes;
  _attachments            = attachments;
  _bSubmitAsHTML          = bSubmitAsHTML;
  _bSuppressEmail         = bSuppressEmail;
  _iCaptureWidth          = iCaptureWidth;
  _iCaptureHeight         = iCaptureHeight;
  _strType                = i18n("script entry");
}

ElogThreadSubmit::~ElogThreadSubmit() {
  if (_job) {
    _job->kill();
    _job = 0L;
  }
}

void ElogThreadSubmit::doTransmit( ) {
  KURL destination;
  QStringList::iterator it;
  QString boundary;
  int iAttachment = 0;

  destination.setProtocol("http");
  destination.setHost(_strHostname);
  destination.setPort(_port);
  destination.setQuery("");
  if (!_strLogbook.isEmpty()) {
    destination.setPath(QString("/%1/").arg(_strLogbook));
  }

  srand((unsigned) time(NULL));
  boundary = QString("---------------------------%1%2%3").arg(rand(), 4, 16).arg(rand(), 4, 16).arg(rand(), 4, 16);

  //
  // add the attributes...
  //
  addAttribute( _dataStreamAll, boundary, "cmd", "Submit", false );
  addAttribute( _dataStreamAll, boundary, "unm", _strUserName, false );
  addAttribute( _dataStreamAll, boundary, "upwd", _strUserPassword, true );
  addAttribute( _dataStreamAll, boundary, "exp", _strLogbook, false );

  for ( QMap<QString, QString>::iterator itMap = _attributes.begin(); itMap != _attributes.end(); ++itMap ) {
    addAttribute( _dataStreamAll, boundary, itMap.key().stripWhiteSpace(), 
                  itMap.data().stripWhiteSpace(), false );
  }

  if( _bSubmitAsHTML ) {
    addAttribute( _dataStreamAll, boundary, "html", "1", false );
  }
  if( _bSuppressEmail ) {
    addAttribute( _dataStreamAll, boundary, "suppress", "1", false );
  }

  addAttribute( _dataStreamAll, boundary, "Text", _strMessage, false );
  QString str = QString("%1\r\n").arg(boundary);
  _dataStreamAll.writeRawBytes(str.ascii(), str.length());

  //
  // add the attachments...
  //
  if( _bIncludeCapture ) {
    KstELOGCaptureStruct captureStruct;
    QByteArray byteArrayCapture;
    QDataStream dataStreamCapture( byteArrayCapture, IO_ReadWrite );
    QCustomEvent eventCapture( KstELOGCaptureEvent );

    captureStruct.pBuffer = &dataStreamCapture;
    captureStruct.iWidth = _iCaptureWidth;
    captureStruct.iHeight = _iCaptureHeight;

    eventCapture.setData( &captureStruct );
    QApplication::sendEvent( (QObject*)KstJS::inst()->app(), (QEvent*)&eventCapture );
    iAttachment++;
    addAttachment( _dataStreamAll, boundary, byteArrayCapture, iAttachment, "Capture.png" );
  }

  if( _bIncludeConfiguration ) {
    QByteArray byteArrayConfigure;
    QTextStream textStreamConfigure( byteArrayConfigure, IO_ReadWrite );
    QCustomEvent eventConfigure( KstELOGConfigureEvent );

    eventConfigure.setData( &textStreamConfigure );
    QApplication::sendEvent( (QObject*)KstJS::inst()->app(), (QEvent*)&eventConfigure );
    iAttachment++;
    addAttachment( _dataStreamAll, boundary, byteArrayConfigure, iAttachment, "Configure.kst" );
  }

  if( _bIncludeDebugInfo ) {
    QByteArray byteArrayDebugInfo;
    QTextStream textStreamDebugInfo( byteArrayDebugInfo, IO_ReadWrite );
    QCustomEvent eventDebugInfo( KstELOGDebugInfoEvent );

    eventDebugInfo.setData( &textStreamDebugInfo );
    QApplication::sendEvent( (QObject*)KstJS::inst()->app(), (QEvent*)&eventDebugInfo );
    iAttachment++;
    addAttachment( _dataStreamAll, boundary, byteArrayDebugInfo, iAttachment, "DebugInfo.txt" );
  }

  for ( QStringList::iterator itList = _attachments.begin(); itList != _attachments.end(); ++itList ) {
    QByteArray byteArrayAttachment;
    QFile file(*itList);

    if (file.open( IO_ReadOnly )) {
      byteArrayAttachment = file.readAll();
      iAttachment++;
      addAttachment( _dataStreamAll, boundary, byteArrayAttachment, iAttachment, *itList );
    } else {
      doError( i18n("%1: unable to open attachment '%2'").arg(_strType).arg(*itList), KstDebug::Warning );
    }
  }

  _job = KIO::http_post(destination, _byteArrayAll, false);
  if (_job) {
    _job->addMetaData("content-type", QString("multipart/form-data; boundary=%1").arg(boundary));
    if (!_strWritePassword.isEmpty()) {
      QCString enc = KCodecs::base64Encode(_strWritePassword.ascii());

      _job->addMetaData("cookies", "manual");
      _job->addMetaData("setcookies", QString("Cookie: wpwd=%1").arg(enc.data()));
    }

    QObject::connect(_job, SIGNAL(result(KIO::Job*)), this, SLOT(result(KIO::Job*)));
    QObject::connect(_job, SIGNAL(dataReq(KIO::Job*, QByteArray&)), this, SLOT(dataReq(KIO::Job*, QByteArray&)));
    QObject::connect(_job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(data(KIO::Job*, const QByteArray&)));

    KIO::Scheduler::scheduleJob(_job);
  } else {
    doError( i18n("%1: unable to create KIO::Job").arg(_strType), KstDebug::Warning );
  }
}

bool ElogThreadSubmit::doResponseCheck( const char* response ) {
  QString strError;
  char str[80];

  if (strstr(response, "Location:")) {
    if (strstr(response, "wpwd")) {
      doError( i18n("Failed to add %1: invalid password").arg(_strType) );
    } else if (strstr(response, "wusr")) {
      doError( i18n("Failed to add %1: invalid username").arg(_strType) );
    } else {
      strncpy(str, strstr(response, "Location:") + 10, sizeof(str));
      if (strchr(str, '?')) {
        *strchr(str, '?') = 0;
      }
      if (strchr(str, '\n')) {
        *strchr(str, '\n') = 0;
      }
      if (strchr(str, '\r')) {
        *strchr(str, '\r') = 0;
      }

      if (strrchr(str, '/')) {
        strError = i18n("Successfully added %1: ID=%2\n").arg(_strType).arg( strrchr(str, '/') + 1 );
        doError( strError, KstDebug::Notice );
      } else {
        strError = i18n("Successfully added %1: ID=%2\n").arg(_strType).arg( str );
        doError( strError, KstDebug::Notice );
      }
    }
  } else {
    doError( i18n("Successfully added %1").arg(_strType), KstDebug::Notice );
  }

  return TRUE;
}

bool ElogThreadSubmit::doResponseError( const char* response, const QString& strDefault ) {
  QString strError;
  char str[80];

  if (strstr(response, "Logbook Selection"))
    doError( i18n("Failed to add %1: no logbook specified").arg(_strType) );
  else if (strstr(response, "enter password")) {
    doError( i18n("Failed to add %1: missing or invalid password").arg(_strType) );
  } else if (strstr(response, "form name=form1")) {
    doError( i18n("Failed to add %1: missing or invalid username/password").arg(_strType) );
  } else if (strstr(response, "Error: Attribute")) {
    strncpy(str, strstr(response, "Error: Attribute") + 20, sizeof(str));
    if (strchr(str, '<')) {
      *strchr(str, '<') = 0;
    }
    strError = i18n("Failed to add %1: missing required attribute \"%2\"").arg(_strType).arg( str );
    doError( strError );
  } else {
    strError = i18n("Failed to add %1: error: %2").arg(_strType).arg( strDefault );
    doError( strError ); 
  }

  return TRUE;
}

void ElogThreadSubmit::doError( const QString& text, KstDebug::LogLevel level ) {
  KstDebug::self()->log(text, level );
}

void ElogThreadSubmit::dataReq(KIO::Job *job, QByteArray &array) {
  Q_UNUSED(job)

  array.resize(0);

  return;
}

void ElogThreadSubmit::data(KIO::Job *job, const QByteArray &array) {
  Q_UNUSED(job)

  if (array.count() > 0) {
    _textStreamResult << array.data();
  }
}

void ElogThreadSubmit::result(KIO::Job *job) {
  if (_job) {
    _job = 0L;

    if (job->error()) {
      _textStreamResult << '\0';
      doResponseError(_byteArrayResult.data(), job->errorText());
    } else {
      if (_byteArrayResult.count() > 0) {
        _textStreamResult << '\0';
        doResponseCheck(_byteArrayResult.data());
      } else {
        doError( i18n("%1: unable to receive response").arg(_strType), KstDebug::Notice );
      }
    }
  }

  delete this;
}

void ElogThreadSubmit::addAttachment( QDataStream& stream,
                                const QString& boundary,
                                const QByteArray& byteArray,
                                int iFileNumber,
                                const QString& name ) {
  if (byteArray.count() > 0) {
    QString strStart = QString("Content-Disposition: form-data; name=\"attfile%1\"; filename=\"%2\"\r\n\r\n").arg(iFileNumber).arg(name);
    QString strEnd = QString("%1\r\n").arg(boundary);

    stream.writeRawBytes(strStart.ascii(), strStart.length());
    stream.writeRawBytes(byteArray.data(), byteArray.count());
    stream.writeRawBytes(strEnd.ascii(), strEnd.length());
  }
}

void ElogThreadSubmit::addAttribute( QDataStream& stream,
                               const QString& boundary,
                               const QString& tag,
                               const QString& strValue,
                               bool bEncode ) {
  if (!strValue.isEmpty()) {
    QString str;

    if( bEncode ) {
      QCString enc = KCodecs::base64Encode(strValue.latin1());

      str = QString("%1\r\nContent-Disposition: form-data; name=\"%2\"\r\n\r\n%3\r\n").arg(boundary).arg(tag).arg(enc.data());
    } else {
      str = QString("%1\r\nContent-Disposition: form-data; name=\"%2\"\r\n\r\n%3\r\n").arg(boundary).arg(tag).arg(strValue);
    }
    stream.writeRawBytes(str.ascii(), str.length());
  }
}

#include "elogthreadsubmit.moc"

