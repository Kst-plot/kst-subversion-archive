/***************************************************************************
                                    elog.h
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

#ifndef ELOG_H
#define ELOG_H

#include <kstextension.h>
#include <kxmlguiclient.h>
#include "elogconfiguration_i.h"
#include "elogentry_i.h"

class KstELOG : public KstExtension, public KXMLGUIClient {
  Q_OBJECT
  public:
    KstELOG(QObject *parent, const char *name, const QStringList&);
    virtual ~KstELOG();
    void submitEntry();
    ElogConfigurationI* configuration() { return _elogConfiguration; }
    ElogEntryI* entry() { return _elogEntry; }
    
    // To save state
    virtual void load(QDomElement& e);
    virtual void save(QTextStream& ts);
    
  public slots:
    void doShow();
    void doEntry();

  private:
    ElogConfigurationI*	_elogConfiguration;
    ElogEntryI*				  _elogEntry;
};

#endif

// vim: ts=2 sw=2 et
