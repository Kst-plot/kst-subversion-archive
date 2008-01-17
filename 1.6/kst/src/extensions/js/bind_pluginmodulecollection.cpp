/***************************************************************************
                        bind_pluginmodulecollection.cpp
                             -------------------
    begin                : Apr 12 2005
    copyright            : (C) 2005 The University of Toronto
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

#include "bind_pluginmodulecollection.h"
#include "bind_pluginmodule.h"

#include <plugincollection.h>
#include <kstobject.h>

#include <kdebug.h>

KstBindPluginModuleCollection::KstBindPluginModuleCollection(KJS::ExecState *exec)
: KstBindCollection(exec, "PluginModuleCollection", true) {
}


KstBindPluginModuleCollection::~KstBindPluginModuleCollection() {
}


KJS::Value KstBindPluginModuleCollection::length(KJS::ExecState *exec) const {
  return KJS::Number(collection(exec).count());
}


QStringList KstBindPluginModuleCollection::collection(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  QStringList rc;

  const QMap<QString,Plugin::Data>& pluginList = PluginCollection::self()->pluginList();

  for (QMap<QString,Plugin::Data>::ConstIterator it = pluginList.begin(); it != pluginList.end(); ++it) {
    rc << it.data()._name;
  }

  const KstPluginInfoList pluginInfo = KstDataObject::pluginInfoList();

  for (KstPluginInfoList::ConstIterator it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
    rc << it.key();
  }

  return rc;
}


KJS::Value KstBindPluginModuleCollection::extract(KJS::ExecState *exec, const KJS::Identifier& item) const {
  QString plugin = item.qstring();

  const QMap<QString,Plugin::Data>& pluginList = PluginCollection::self()->pluginList();

  for (QMap<QString,Plugin::Data>::ConstIterator it = pluginList.begin(); it != pluginList.end(); ++it) {
    if (it.data()._name == plugin) {
      return KJS::Object(new KstBindPluginModule(exec, it.data()));
    }
  }

  const KstPluginInfoList pluginInfo = KstDataObject::pluginInfoList();

  for (KstPluginInfoList::ConstIterator it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
    if (it.key() == plugin) {
      KstDataObjectPtr ptr = KstDataObject::plugin(it.key());
      if (ptr) {
        KstBasicPluginPtr bp = kst_cast<KstBasicPlugin>(ptr);
        if (bp) {
          return KJS::Object(new KstBindPluginModule(exec, bp));
        }
      }
    }
  }

  return KJS::Undefined();
}


KJS::Value KstBindPluginModuleCollection::extract(KJS::ExecState *exec, unsigned item) const {
  uint j = 0;

  const QMap<QString,Plugin::Data>& pluginList = PluginCollection::self()->pluginList();

  for (QMap<QString,Plugin::Data>::ConstIterator it = pluginList.begin(); it != pluginList.end(); ++it) {
    if (j++ == item) {
      return KJS::Object(new KstBindPluginModule(exec, it.data()));
    }
  }

  const KstPluginInfoList pluginInfo = KstDataObject::pluginInfoList();

  for (KstPluginInfoList::ConstIterator it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
    if (j++ == item) {
      KstDataObjectPtr ptr = KstDataObject::plugin(it.key());
      if (ptr) {
        KstBasicPluginPtr bp = kst_cast<KstBasicPlugin>(ptr);
        if (bp) {
          return KJS::Object(new KstBindPluginModule(exec, bp));
        }
      }
    }
  }

  return KJS::Undefined();
}
