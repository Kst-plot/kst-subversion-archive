/***************************************************************************
                               bind_plugin.cpp
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

#include "bind_plugin.h"
#include "bind_pluginmodule.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindPlugin::KstBindPlugin(KJS::ExecState *exec, KstPluginPtr d)
: KstBinding("Plugin"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPlugin::KstBindPlugin(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Plugin") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Plugin", o);
  }
}


KstBindPlugin::KstBindPlugin(int id)
: KstBinding("Plugin Method", id) {
}


KstBindPlugin::~KstBindPlugin() {
}


KJS::Object KstBindPlugin::construct(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  return KJS::Object(new KstBindPlugin(exec));
}


struct PluginBindings {
  const char *name;
  KJS::Value (KstBindPlugin::*method)(KJS::ExecState*, const KJS::List&);
};


struct PluginProperties {
  const char *name;
  void (KstBindPlugin::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindPlugin::*get)(KJS::ExecState*) const;
};


static PluginBindings pluginBindings[] = {
  { 0L, 0L }
};


static PluginProperties pluginProperties[] = {
  { "tagName", &KstBindPlugin::setTagName, &KstBindPlugin::tagName },
  { "module", 0L, &KstBindPlugin::module },
  { "lastError", 0L, &KstBindPlugin::lastError },
  { "valid", 0L, &KstBindPlugin::valid },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPlugin::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; pluginProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(pluginProperties[i].name)));
  }

  return rc;
}


bool KstBindPlugin::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; pluginProperties[i].name; ++i) {
    if (prop == pluginProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindPlugin::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; pluginProperties[i].name; ++i) {
    if (prop == pluginProperties[i].name) {
      if (!pluginProperties[i].set) {
        break;
      }
      (this->*pluginProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPlugin::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; pluginProperties[i].name; ++i) {
    if (prop == pluginProperties[i].name) {
      if (!pluginProperties[i].get) {
        break;
      }
      return (this->*pluginProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindPlugin::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindPlugin *imp = dynamic_cast<KstBindPlugin*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*pluginBindings[id - 1].method)(exec, args);
}


void KstBindPlugin::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; pluginBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPlugin(i + 1));
    obj.put(exec, pluginBindings[i].name, o, KJS::Function);
  }
}


void KstBindPlugin::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindPlugin::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


KJS::Value KstBindPlugin::module(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindPluginModule(exec, _d->plugin()->data()));
}


void KstBindPlugin::setModule(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::ObjectType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  //_d->setPlugin(); FIXME - then enable above
}


KJS::Value KstBindPlugin::lastError(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->lastError());
}


KJS::Value KstBindPlugin::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->isValid());
}


// vim: ts=2 sw=2 et
