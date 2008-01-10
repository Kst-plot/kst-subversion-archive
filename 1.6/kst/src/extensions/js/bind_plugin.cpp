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
#include "bind_scalar.h"
#include "bind_string.h"

#include <kstdatacollection.h>

#include <kdebug.h>

KstBindPlugin::KstBindPlugin(KJS::ExecState *exec, KstCPluginPtr d)
: KstBindDataObject(exec, d.data(), "Plugin") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPlugin::KstBindPlugin(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindDataObject(exec, globalObject, "Plugin") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("Plugin", KstBindPlugin::bindFactory);
  }
}


KstBindDataObject *KstBindPlugin::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  KstCPluginPtr v = kst_cast<KstCPlugin>(obj);
  if (v) {
    return new KstBindPlugin(exec, v);
  }
  return 0L;
}


KstBindPlugin::KstBindPlugin(int id)
: KstBindDataObject(id, "Plugin Method") {
}


KstBindPlugin::~KstBindPlugin() {
}


KJS::Object KstBindPlugin::construct(KJS::ExecState *exec, const KJS::List& args) {
  KstCPluginPtr p;

  if (args.size() > 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  p = new KstCPlugin;

  if (args.size() > 0) {
    KstSharedPtr<Plugin> m = extractPluginModule(exec, args[0]);
    if (!m) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    p->setPlugin(m);
  }

  return KJS::Object(new KstBindPlugin(exec, p));
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


static PluginBindings pluginBindings[] = {\
  { "validate", &KstBindPlugin::validate },
  { "setInput", &KstBindPlugin::setInput },
  { 0L, 0L }
};


static PluginProperties pluginProperties[] = {
  { "module", &KstBindPlugin::setModule, &KstBindPlugin::module },
  { "lastError", 0L, &KstBindPlugin::lastError },
  { "valid", 0L, &KstBindPlugin::valid },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPlugin::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindDataObject::propList(exec, recursive);

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

  return KstBindDataObject::hasProperty(exec, propertyName);
}


void KstBindPlugin::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindDataObject::put(exec, propertyName, value, attr);
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

  KstBindDataObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPlugin::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindDataObject::get(exec, propertyName);
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

  return KstBindDataObject::get(exec, propertyName);
}


KJS::Value KstBindPlugin::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindPlugin *imp = dynamic_cast<KstBindPlugin*>(self.imp());
    if (!imp) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
      exec->setException(eobj);
      return KJS::Undefined();
    }

    return (imp->*pluginBindings[id - start - 1].method)(exec, args);
  }

  return KstBindDataObject::call(exec, self, args);
}


void KstBindPlugin::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindDataObject::methodCount();
  for (int i = 0; pluginBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPlugin(i + start + 1));
    obj.put(exec, pluginBindings[i].name, o, KJS::Function);
  }
}


#define makePlugin(X) dynamic_cast<KstCPlugin*>(const_cast<KstObject*>(X.data()))

KJS::Value KstBindPlugin::validate(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);
    if (d->validate()) {
      return KJS::Boolean(true);
    }
  }

  return KJS::Boolean(false);
}


KJS::Value KstBindPlugin::setInput(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 2) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);

    if (!d->plugin()) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Plugin module must be set first.");
      exec->setException(eobj);
      return KJS::Undefined();
    }

    QString input;
    unsigned int index;

    if (args[0].type() == KJS::NumberType) {
      index = args[0].toUInt32(exec);
      if (index < d->plugin()->data()._inputs.size()) {
        input = d->plugin()->data()._inputs[index]._name;
      } else {
        KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Index is out of range.");
        exec->setException(eobj);
        return KJS::Undefined();
      }
    } else if (args[0].type() == KJS::StringType) {
      input = args[0].toString(exec).qstring();
      for (index = 0; index < d->plugin()->data()._inputs.size(); ++index ) {
        if (d->plugin()->data()._inputs[index]._name.compare(input) == 0) {
          break;
        }
      }
      if (index >= d->plugin()->data()._inputs.size()) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Invalid argument name.");
        exec->setException(eobj);
        return KJS::Undefined();
      }
    } else {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Undefined();
    }

    if (!input.isEmpty()) {
      if (index < d->plugin()->data()._inputs.size()) {
        Plugin::Data::IOValue::ValueType type = d->plugin()->data()._inputs[index]._type;

        //
        // ensure that the argument is of the right type...
        //
        if (type == Plugin::Data::IOValue::TableType) {
          KstVectorPtr vp = extractVector(exec, args[1]);
          if (vp) {
            d->inputVectors().insert(d->plugin()->data()._inputs[index]._name, vp);
          } else {
            KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Argument was not of the expected type.");
            exec->setException(eobj);
            return KJS::Undefined();
          }
        } else if (type == Plugin::Data::IOValue::StringType) {
          KstStringPtr sp;

          if (args[1].type() == KJS::ObjectType) {
            KstBindString *imp = dynamic_cast<KstBindString*>(args[1].toObject(exec).imp());
            if (imp) {
              sp = kst_cast<KstString>(imp->_d);
            }
           } else if (args[1].type() ==  KJS::StringType) {
            KST::stringList.lock().readLock();
            sp = *KST::stringList.findTag(args[0].toString(exec).qstring());
            KST::stringList.lock().unlock();
          }

          if (sp) {
            d->inputStrings().insert(input, sp);
          } else {
            KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Argument was not of the expected type.");
            exec->setException(eobj);
            return KJS::Undefined();
          }
        } else if (type == Plugin::Data::IOValue::FloatType ||
                   type == Plugin::Data::IOValue::PidType ) {
          KstScalarPtr sp;

          if (args[1].type() == KJS::ObjectType) {
            KstBindScalar *imp = dynamic_cast<KstBindScalar*>(args[1].toObject(exec).imp());
            if (imp) {
              sp = kst_cast<KstScalar>(imp->_d);
            }
          } else if (args[1].type() == KJS::StringType) {
            KST::scalarList.lock().readLock();
            sp = *KST::scalarList.findTag(args[0].toString(exec).qstring());
            KST::scalarList.lock().unlock();
          }

          if (sp) {
            d->inputScalars().insert(input, sp);
          } else {
            KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Argument was not of the expected type.");
            exec->setException(eobj);
            return KJS::Undefined();
          }
        } else {
          KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Argument was not of the expected type.");
          exec->setException(eobj);
          return KJS::Undefined();
        }
      } else {
        KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Argument was not of the expected type.");
        exec->setException(eobj);
        return KJS::Undefined();
      }
    } else {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Argument was not of the expected type.");
      exec->setException(eobj);
      return KJS::Undefined();
    }
  } else {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return KJS::Boolean(true);
}


KJS::Value KstBindPlugin::module(KJS::ExecState *exec) const {
  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);
    if (d->plugin()) {
      return KJS::Object(new KstBindPluginModule(exec, d->plugin()->data()));
    }
  }
  return KJS::Null();
}


void KstBindPlugin::setModule(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::ObjectType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstSharedPtr<Plugin> m = KstBinding::extractPluginModule(exec, value);
  if (m) {
    KstCPluginPtr d = makePlugin(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setModule(m);

      if (!d->plugin()) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Failed to set module");
        exec->setException(eobj);
        return;
      }
    }
  }
}


KJS::Value KstBindPlugin::lastError(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->lastError());
  }
  return KJS::String();
}


KJS::Value KstBindPlugin::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->isValid());
  }
  return KJS::Boolean(false);
}

#undef makePlugin

