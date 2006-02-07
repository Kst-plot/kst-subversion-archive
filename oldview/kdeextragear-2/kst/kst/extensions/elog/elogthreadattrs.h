/***************************************************************************
                                    elogthreadattrs.h
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

#ifndef ELOGTHREADATTRS_H
#define ELOGTHREADATTRS_H

#include <qthread.h>
#include <kstdebug.h>
#include "elogthread.h"
#include "elog.h"

class ElogThreadAttrs : public ElogThread {
  public:
    ElogThreadAttrs(KstELOG*);
    virtual ~ElogThreadAttrs();
    virtual void run();
    
  protected:
    virtual void doTransmit( int sock );
    virtual bool doResponseError( const char* response );
    
  private:
    void doResponse( char* response );    
};

#endif

// vim: ts=2 sw=2 et
