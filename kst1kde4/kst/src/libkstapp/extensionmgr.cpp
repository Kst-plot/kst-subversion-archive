/***************************************************************************
                   extensionmgr.cpp: Kst Extension Manager
                             -------------------
    begin                : Mar 30, 2004
    copyright            : (C) 2003 The University of Toronto
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

#include <assert.h>

#include <kparts/componentfactory.h>
#include <kservicetype.h>

#include "extensionmgr.h"
#include "kst.h"
#include "kstdebug.h"

ExtensionMgr *ExtensionMgr::self() {
  if (!_self) {
// xxx    sdExtension.setObject(_self, new ExtensionMgr);
  }

  return _self;
}


void ExtensionMgr::save() {
  QMap<QString,bool>::const_iterator i;
// xxx  KConfig cfg("kstextensionsrc", false, false);
  QStringList disabled;
  QStringList enabled;

// xxx  cfg.setGroup("Extensions");

  for (i = _extensions.begin(); i != _extensions.end(); ++i) {
    if (*i) {
      enabled += i.key();
    } else {
      disabled += i.key();
    }
  }
/* xxx
  cfg.writeEntry("Disabled", disabled);
  cfg.writeEntry("Enabled", enabled);
*/
}


ExtensionMgr::ExtensionMgr() : QObject(), _window(0L) {
  QStringList::const_iterator i;
// xxx  KConfig cfg("kstextensionsrc", true, false);
  QStringList disabled;
  QStringList enabled;

// xxx  disabled = cfg.readListEntry("Disabled");
// xxx  enabled = cfg.readListEntry("Enabled");

// xxx  cfg.setGroup("Extensions");
  for (i = disabled.begin(); i != disabled.end(); ++i) {
    _extensions[*i] = false;
  }
  for (i = enabled.begin(); i != enabled.end(); ++i) {
    _extensions[*i] = true;
  }
}


ExtensionMgr::~ExtensionMgr() {
  save();
  // we do not need to explicitly delete any remaining 
  // extensions as they are handled automatically...
}


KstExtension *ExtensionMgr::extension(const QString& name) const {
  QMap<QString,KstExtension*>::ConstIterator i = _registry.find(name);

  if (i != _registry.end()) {
    return *i;
  }

  return 0L;
}


void ExtensionMgr::loadExtension(const QString& name) {
  KService::List sl;
  KService::List::const_iterator it;

// xxx  sl = KServiceType::offers("Kst Extension");

  for (it = sl.begin(); it != sl.end(); ++it) {
    KService::Ptr service = *it;

    if (name == service->property("Name").toString()) {
      loadExtension(service);

      return;
    }
  }
}


void ExtensionMgr::loadExtension(const KService::Ptr& service) {
  int err = 0;
  QString name = service->property("Name").toString();
  KstExtension *e = 0L;

// xxx  e = KParts::ComponentFactory::createInstanceFromService<KstExtension>(service, _window, 0, QStringList(), &err);

  if (e) {
    connect(e, SIGNAL(unregister()), this, SLOT(unregister()));
    KstDebug::self()->log(tr("Kst Extension %1 loaded.").arg(name));
    doRegister(name,e);
  } else {
// xxx    KstDebug::self()->log(tr("Error trying to load Kst extension %1.  Code=%2, \"%3\"").arg(name).arg(err).arg(err == KParts::ComponentFactory::ErrNoLibrary ? tr("Library not found [%1].").arg(KLibLoader::self()->lastErrorMessage()) : KLibLoader::self()->lastErrorMessage()), KstDebug::Error);
  }
}


void ExtensionMgr::updateExtensions() {
  QMap<QString,bool>::ConstIterator i;
  QMap<QString,KstExtension*>::Iterator j;

  for (i = _extensions.begin(); i != _extensions.end(); ++i) {
    j = _registry.find(i.key());
    if (*i) {
      if (j == _registry.end()) {      
        loadExtension(i.key());
      }
    } else {
      if (j != _registry.end()) {      
        delete *j;
        // Does this automatically
        //_registry.remove(j);
      }
    }
  }
}


void ExtensionMgr::doRegister(const QString& name, KstExtension *inst) {
  assert(!_registry.count(name));

  _registry[name] = inst;
}


void ExtensionMgr::unregister(KstExtension *inst) {
  QMap<QString,KstExtension*>::Iterator i;

  for (i = _registry.begin(); i != _registry.end(); ++i) {
    if ((*i) == inst) {
// xxx      _registry.remove(i);

      break;
    }
  }
}


void ExtensionMgr::unregister() {
  unregister(const_cast<KstExtension*>(static_cast<const KstExtension*>(sender())));
}

ExtensionMgr *ExtensionMgr::_self = 0L;

#include "extensionmgr.moc"
