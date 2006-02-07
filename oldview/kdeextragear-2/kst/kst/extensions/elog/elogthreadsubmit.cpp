/***************************************************************************
                                   elogthreadsubmit.cpp
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

#include "elogthreadsubmit.h"
#include <kstdebug.h>
#include <kst.h>
#include <kstevents.h>
#include <qiodevice.h>
#include <qbuffer.h>
#include <qcstring.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

ElogThreadSubmit::ElogThreadSubmit(KstELOG* elog) : ElogThread(elog) {
}

ElogThreadSubmit::~ElogThreadSubmit() {
}

void ElogThreadSubmit::doTransmit(int sock) {
  KstELOGCaptureStruct captureStruct;
  QByteArray	byteArrayCapture;
  QBuffer bufferCapture( byteArrayCapture );
  QByteArray	byteArrayConfigure;
  QTextStream textStreamConfigure( byteArrayConfigure, IO_ReadWrite );
  QByteArray	byteArrayDebugInfo;
  QTextStream textStreamDebugInfo( byteArrayDebugInfo, IO_ReadWrite );
  QCustomEvent eventCapture(KstELOGCaptureEvent);
  QCustomEvent eventConfigure(KstELOGConfigureEvent);
  QCustomEvent eventDebugInfo(KstELOGDebugInfoEvent);
  QStringList::iterator it;
  QStringList	strListAttributes;
  QStringList	strListAttribute;
  QString strSplit( "\n" );
  QString strSplitAttribute( "=" );
  QString strUserName;
  QString strUserPassword;
  QString strWritePassword;
  QString strLogbook;
  QString strAttributes;
  QString strText;
  bool bIncludeCapture;
  bool bIncludeConfiguration;
  bool bIncludeDebugInfo;
  bool bSubmitAsHTML;
  bool bSuppressEmail;
  char* content;
  char request[100000];
  char response[100000];
  char boundary[80], str[80], *p;
  int i, n, header_length, content_length;
  int iAttachment = 0;

  bIncludeCapture   		= _elog->entry()->includeCapture();
  bIncludeConfiguration = _elog->entry()->includeConfiguration();
  bIncludeDebugInfo     = _elog->entry()->includeDebugInfo();
  content_length 			  = 100000;
  
  if( bIncludeCapture ) {  
    captureStruct.pBuffer     = &bufferCapture;
    captureStruct.iWidth      = _elog->configuration()->captureWidth();
    captureStruct.iHeight     = _elog->configuration()->captureHeight();
    eventCapture.setData( &captureStruct );
    QApplication::sendEvent( (QObject*)_elog->app()->viewObject(), (QEvent*)&eventCapture );
    content_length += byteArrayCapture.size();
  }
  
  if( bIncludeConfiguration ) {
    eventConfigure.setData( &textStreamConfigure );
    QApplication::sendEvent( (QObject*)_elog->app(), (QEvent*)&eventConfigure );
    content_length += byteArrayConfigure.size();
  }
  
  if( bIncludeDebugInfo ) {
    eventDebugInfo.setData( &textStreamDebugInfo );
    QApplication::sendEvent( (QObject*)_elog->app(), (QEvent*)&eventDebugInfo );
    content_length += byteArrayDebugInfo.size();
  }
  
  content = (char*)malloc(content_length);
  if (content != NULL) {
    strUserName 					= _elog->configuration()->userName();
    strUserPassword 			= _elog->configuration()->userPassword();
    strWritePassword			= _elog->configuration()->writePassword();
    strLogbook  					= _elog->configuration()->name();
    bSubmitAsHTML    		  = _elog->configuration()->submitAsHTML();
    bSuppressEmail    		= _elog->configuration()->suppressEmail();
    strAttributes     		= _elog->entry()->attributes();
    strText     					= _elog->entry()->text();
    
    srand((unsigned) time(NULL));
    sprintf(boundary, "---------------------------%04X%04X%04X", rand(), rand(), rand());
    strcpy(content, boundary);
    strcat(content, "\r\nContent-Disposition: form-data; name=\"cmd\"\r\n\r\nSubmit\r\n");
    
    //
    // add the attributes...
    //
    addAttribute( content, boundary, "unm", strUserName, false );
    addAttribute( content, boundary, "upwd", strUserPassword, true );
    addAttribute( content, boundary, "exp", strLogbook, false );
    
    strListAttributes = QStringList::split( strSplit, strAttributes, FALSE ); 
    for ( it = strListAttributes.begin(); it != strListAttributes.end(); ++it ) {
      strListAttribute = QStringList::split( strSplitAttribute, *it, FALSE );
      if( strListAttribute.count() == 2 ) {
        addAttribute( content, boundary, 
                      strListAttribute.first().stripWhiteSpace().ascii(),
                      strListAttribute.last().stripWhiteSpace(), false );
      }
    }
    if( bSubmitAsHTML ) {
      addAttribute( content, boundary, "html", "1", false );      
    }
    if( bSuppressEmail ) {
      addAttribute( content, boundary, "suppress", "1", false );      
    }
    addAttribute( content, boundary, "Text", strText, false );
    sprintf( content + strlen(content), "%s\r\n", boundary );

    content_length = strlen(content);
    p = content + content_length;
    
    //
    // add the attachments...
    //
    if( bIncludeCapture ) {
      iAttachment++;
      addAttachment( &content_length, &p, boundary, &byteArrayCapture, iAttachment, "Capture.png" );
    }
    if( bIncludeConfiguration ) {
      iAttachment++;
      addAttachment( &content_length, &p, boundary, &byteArrayConfigure, iAttachment, "Configure.kst" );
    }
    if( bIncludeDebugInfo ) {
      iAttachment++;
      addAttachment( &content_length, &p, boundary, &byteArrayDebugInfo, iAttachment, "DebugInfo.txt" );
    }

    strcpy(request, "POST /");
    if (!strLogbook.isEmpty()) {
      sprintf(request + strlen(request), "%s/", strLogbook.ascii());
    }
    strcat(request, " HTTP/1.0\r\n");
    
    sprintf(request + strlen(request), "Content-Type: multipart/form-data; boundary=%s\r\n", boundary);
    sprintf(request + strlen(request), "Host: %s\r\n", _host_name);
    sprintf(request + strlen(request), "User-Agent: ELOG\r\n");
    sprintf(request + strlen(request), "Content-Length: %d\r\n", content_length);
    
    if (!strWritePassword.isEmpty()) {
      base64_encode(strWritePassword.ascii(), str);
      sprintf(request + strlen(request), "Cookie: wpwd=%s\r\n", str);
    }

    strcat(request, "\r\n");
    
    header_length = strlen(request);
    
    send(sock, request, header_length, 0);
    send(sock, content, content_length, 0);
    
    //
    // handle the response...
    //
    i = recv(sock, response, 10000, 0);
    if (i >= 0) {    
      n = i;
      while (i > 0) {
        i = recv(sock, response + n, 10000, 0);
        if (i > 0) {
          n += i;
        }
      }
      response[n] = 0;
      
      doResponseError(response);
    }
    else
    {
      doError( tr2i18n("ELOG entry: unable to receive response"), KstDebug::Notice );
    }
  }
}

void ElogThreadSubmit::run( ) {
  int status, sock, error;
  
  error = makeConnection( &sock, &status );
  if( status == 0 ) {
    doTransmit( sock );
    close( sock );
  }
  else {
    switch( error ) {
      case -1:
        doError( tr2i18n("Failed to add ELOG entry: failed to connect to host.") );          	      		  break;
      case -2:
        doError( tr2i18n("Failed to add ELOG entry: failed to get host name.") );          	      		  break;
      case -3:
        doError( tr2i18n("Failed to add ELOG entry: failed to create socket.") );
        break;
      case -4:
        doError( tr2i18n("Failed to add ELOG entry: failed to get host by address.") );
        break;
      case -5:
        doError( tr2i18n("Failed to add ELOG entry: failed to get host by name.") );          	      		  break;
      default:
        doError( tr2i18n("Failed to add ELOG entry: unknown error.") );
        break;
    }
  }
}

bool ElogThreadSubmit::doResponseError( const char* response ) {
  QString strError;
  char str[80];
  
  if (strstr(response, "302 Found")) {
    if (strstr(response, "Location:")) {
      if (strstr(response, "wpwd")) {
        doError( tr2i18n("Failed to add ELOG entry: invalid password") );
      } else if (strstr(response, "wusr")) {
        doError( tr2i18n("Failed to add ELOG entry: invalid user name") );
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
          strError = tr2i18n("Successfully added ELOG entry: ID=%1\n").arg( strrchr(str, '/') + 1 );
          doError( strError, KstDebug::Notice );
        } else {
          strError = tr2i18n("Successfully added ELOG entry: ID=%1\n").arg( str );
          doError( strError, KstDebug::Notice );
        }
      }
    } else {
      doError( tr2i18n("Successfully added ELOG entry"), KstDebug::Notice );
    }
  } else if (strstr(response, "Logbook Selection"))
    doError( tr2i18n("Failed to add ELOG entry: no logbook specified") );
  else if (strstr(response, "enter password")) {
    doError( tr2i18n("Failed to add ELOG entry: missing or invalid password") );
  }
  else if (strstr(response, "form name=form1")) {
    doError( tr2i18n("Failed to add ELOG entry: missing or invalid user name/password") );
  }
  else if (strstr(response, "Error: Attribute")) {
    strncpy(str, strstr(response, "Error: Attribute") + 20, sizeof(str));
    if (strchr(str, '<')) {
      *strchr(str, '<') = 0;
    }
    strError = tr2i18n("Failed to add ELOG entry: missing required attribute \"%1\"").arg( str );
    doError( strError );
  } else {
    doError( tr2i18n("Failed to add ELOG entry: error transmitting message") ); 
  }
  
  return TRUE;
}
// vim: ts=2 sw=2 et
