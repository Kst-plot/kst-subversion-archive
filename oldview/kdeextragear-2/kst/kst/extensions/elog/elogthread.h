/***************************************************************************
                                    elogthread.h
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

#ifndef ELOGTHREAD_H
#define ELOGTHREAD_H

#include <qthread.h>
#include <kstdebug.h>
#include "elog.h"

class ElogThread : public QThread {
  public:
    ElogThread(KstELOG*);
    virtual ~ElogThread();
    virtual void run();
    
  protected:  
    virtual void doTransmit( int sock );
    virtual bool doResponseError( const char* response );
    
    int  makeConnection( int* psock, int* pstatus );
    void base64_encode( const char *s, char *d );
    void addAttachment( int* piContentLength, char** pp, const char* boundary, QByteArray* pByteArray, int iFileNumber, const char* pName );
    void addAttribute( char* content, const char* boundary, const char* value, const QString& str, bool bEncode );
    void doError( const QString& text, KstDebug::LogLevel level = KstDebug::Warning);
    
    char 			_host_name[256];
    KstELOG* 	_elog;
};

#endif

// vim: ts=2 sw=2 et
