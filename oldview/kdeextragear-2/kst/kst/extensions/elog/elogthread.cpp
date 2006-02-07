/***************************************************************************
                                   elogthread.cpp
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

#include "elogthread.h"
#include <kstdebug.h>
#include <kst.h>
#include <kstevents.h>
#include <qiodevice.h>
#include <qbuffer.h>
#include <qcstring.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <kaction.h>
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

ElogThread::ElogThread(KstELOG* elog) : QThread() {
  _elog = elog;
}


ElogThread::~ElogThread() {
}

void ElogThread::base64_encode( const char *s, char *d )
{
  static char map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned int t, pad;

  pad = 3 - strlen(s) % 3;
  if (pad == 3) {
    pad = 0;
  }
  while (*s) {
    t = (*s++) << 16;
    if (*s) {
      t |= (*s++) << 8;
    }
    if (*s) {
      t |= (*s++) << 0;
    }
    
    *(d + 3) = map[t & 63];
    t >>= 6;
    *(d + 2) = map[t & 63];
    t >>= 6;
    *(d + 1) = map[t & 63];
    t >>= 6;
    *(d + 0) = map[t & 63];
    
    d += 4;
  }
  *d = 0;
  while (pad--) {
    *(--d) = '=';
  }
}

void ElogThread::addAttachment( int* piContentLength,
                                char** pp,
                                const char* boundary,
                                QByteArray* pByteArray,
                                int iFileNumber,
                                const char* pName ) {
  sprintf( *pp, "Content-Disposition: form-data; name=\"attfile%d\"; filename=\"%s\"\r\n\r\n", 
           iFileNumber, pName);
  (*piContentLength) += strlen(*pp);
  (*pp) += strlen(*pp);
  
  memcpy(*pp, pByteArray->data(), pByteArray->size());   
  (*piContentLength) += pByteArray->size();
  (*pp) += pByteArray->size();
  
  strcpy(*pp, boundary);
  strcat(*pp, "\r\n");
  (*piContentLength) += strlen(*pp);
  (*pp) += strlen(*pp);
}

void ElogThread::addAttribute( char* content, 
                               const char* boundary, 
                               const char* tag,
                               const QString& strValue, 
                               bool bEncode ) {
  char str[80];
  
  if (!strValue.isEmpty()) {
    if( bEncode ) {
      base64_encode(strValue.ascii(), str);
      sprintf(content + strlen(content),
              "%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
              boundary, tag, str);
    } else {
      sprintf(content + strlen(content),
              "%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
              boundary, tag, strValue.ascii());
    }
  }
}

void ElogThread::doTransmit(int sock) {
  Q_UNUSED( sock )
}

int ElogThread::makeConnection(int* psock, int* pstatus) {
  QString strHost;
  int port;
  int error = 0;
  struct hostent* phe;
  struct sockaddr_in bind_addr;

  *pstatus = -1;
  port     = _elog->configuration()->portNumber();
  strHost  = _elog->configuration()->ipAddress();
  
  gethostname(_host_name, sizeof(_host_name));
  phe = gethostbyname(_host_name);
  if (phe != NULL) {
    phe = gethostbyaddr(phe->h_addr, sizeof(int), AF_INET);
    if (phe != NULL) {
      if (strchr(_host_name, '.') == NULL) {
        strcpy(_host_name, phe->h_name);
      }
      
      if ((*psock = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family 			= AF_INET;
        bind_addr.sin_addr.s_addr = 0;
        bind_addr.sin_port 				= htons((unsigned short)port);

        phe = gethostbyname(strHost.ascii());
        if (phe != NULL) {
          memcpy((char*)&(bind_addr.sin_addr), phe->h_addr, phe->h_length);
          *pstatus = connect(*psock, (sockaddr*)&bind_addr, (socklen_t)sizeof(bind_addr));
          if( *pstatus != 0 ) {
            error = -1;
          }
        }
        else {
          error = -2;
        }
      }
      else {
        error = -3;
      }
    }
    else
    {
      error = -4;
    }
  }
  else {
    error = -5;
  }
  
  return error;
}

void ElogThread::run() {
}

bool ElogThread::doResponseError( const char* response ) {
  Q_UNUSED( response )
    
  return TRUE;
}

void ElogThread::doError( const QString& text, KstDebug::LogLevel level ) {
  KstDebug::self()->log(text, level );
}
// vim: ts=2 sw=2 et
