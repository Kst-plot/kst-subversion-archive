/***************************************************************************
                                   elog.cpp
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

#include "elog.h"
#include "elogthreadsubmit.h"
#include "elogthreadattrs.h"
#include <kst.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>

K_EXPORT_COMPONENT_FACTORY(kstextension_elog, KGenericFactory<KstELOG>)

KstELOG::KstELOG(QObject *parent, const char *name, const QStringList& l) : KstExtension(parent, name, l), KXMLGUIClient() {
  new KAction(i18n("&ELOG..."), 0, 0, this, SLOT(doShow()), actionCollection(), "elog_settings_show");
  new KAction(i18n("Add ELOG Entry..."), 0, 0, this, SLOT(doEntry()), actionCollection(), "elog_entry_add");
  setInstance(app()->instance());
  setXMLFile("kstextension_elog.rc", true);
  app()->guiFactory()->addClient(this);

  _elogConfiguration = new ElogConfigurationI( this, app() );
  _elogEntry = new ElogEntryI( this, app() );
  
  _elogEntry->initialize();
  _elogConfiguration->initialize();
}


KstELOG::~KstELOG() {
  if( app() ) {
    if( app()->guiFactory() ) {
      app()->guiFactory()->removeClient(this);
    }
  }
  
  delete _elogConfiguration;
  delete _elogEntry;
}


void KstELOG::submitEntry() {  
  ElogThreadSubmit	thread(this);
  
  thread.run();
}


void KstELOG::doEntry() {
  if( _elogEntry ) {
    _elogEntry->show();
    _elogEntry->raise();
  }
}


void KstELOG::doShow() {
  if( _elogConfiguration ) {
    _elogConfiguration->show();
    _elogConfiguration->raise();
  }
}


void KstELOG::load(QDomElement& e) {
  Q_UNUSED(e)
}


void KstELOG::save(QTextStream& ts) {
  Q_UNUSED(ts)
}

#include "elog.moc"
// vim: ts=2 sw=2 et
