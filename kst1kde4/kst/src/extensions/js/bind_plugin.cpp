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

#include "bind_objectcollection.h"
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


KstBindPlugin::KstBindPlugin(KJS::ExecState *exec, KstBasicPluginPtr d)
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
  if (args.size() > 1) {
    return createSyntaxError(exec);
  }

  if (args.size() == 0) {
    KstCPluginPtr p = new KstCPlugin;
    return KJS::Object(new KstBindPlugin(exec, p));
  } else if (args.size() == 1) {
    KstPluginPtr m = extractPluginModule(exec, args[0], false);
    if (m) {
      KstCPluginPtr p = new KstCPlugin;
      p->setModule(m);
      return KJS::Object(new KstBindPlugin(exec, p));
    } else {
      KstBasicPluginPtr tmplt = extractBasicPluginModule(exec, args[0]);
      if (tmplt) {
        KstBasicPluginPtr bp = kst_cast<KstBasicPlugin>(KstDataObject::createPlugin(tmplt->propertyString()));
        if (bp) {
          return KJS::Object(new KstBindPlugin(exec, bp));
        } else {
          return createGeneralError(exec, i18n("Failed to create new plugin."));
        }
      } else {
        return createTypeError(exec, 0);
      }
    }
  }

  return KJS::Object();
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
  { "validate", &KstBindPlugin::validate },
  { "setInput", &KstBindPlugin::setInput },
  { 0L, 0L }
};


static PluginProperties pluginProperties[] = {
  { "inputs", 0L, &KstBindPlugin::inputs },
  { "outputs", 0L, &KstBindPlugin::outputs },
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
    return createInternalError(exec);
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindPlugin *imp = dynamic_cast<KstBindPlugin*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
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
#define makeBasicPlugin(X) dynamic_cast<KstBasicPlugin*>(const_cast<KstObject*>(X.data()))

KJS::Value KstBindPlugin::validate(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);
    if (d->validate()) {
      return KJS::Boolean(true);
    }
  } else {
    KstBasicPluginPtr bp = makeBasicPlugin(_d);
    if (bp) {
      KstReadLocker rl(bp);
      if (bp->validate()) {
        return KJS::Boolean(true);
      }
    }
  }

  return KJS::Boolean(false);
}


KJS::Value KstBindPlugin::setInput(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 2) {
    return createSyntaxError(exec);
  }

  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);

    if (!d->plugin()) {
      return createGeneralError(exec, i18n("Plugin module must be set first."));
    }

    QString input;
    unsigned int index;

    if (args[0].type() == KJS::NumberType) {
      index = args[0].toUInt32(exec);
      if (index < d->plugin()->data()._inputs.size()) {
        input = d->plugin()->data()._inputs[index]._name;
      } else {
        return createRangeError(exec, 0);
      }
    } else if (args[0].type() == KJS::StringType) {
      input = args[0].toString(exec).qstring();
      for (index = 0; index < d->plugin()->data()._inputs.size(); ++index ) {
        if (d->plugin()->data()._inputs[index]._name.compare(input) == 0) {
          break;
        }
      }
      if (index >= d->plugin()->data()._inputs.size()) {
        return createGeneralError(exec, i18n("Invalid argument name."));
      }
    } else {
      return createTypeError(exec, 0);
    }

    if (!input.isEmpty()) {
      if (index < d->plugin()->data()._inputs.size()) {
        Plugin::Data::IOValue::ValueType type = d->plugin()->data()._inputs[index]._type;

        //
        // ensure that the argument is of the right type...
        //
        if (type == Plugin::Data::IOValue::TableType) {
          KstVectorPtr vp = extractVector(exec, args[1], false);
          if (vp) {
            d->inputVectors().insert(d->plugin()->data()._inputs[index]._name, vp);
            d->setDirty(true);
            d->setRecursed(false);
            if (d->recursion()) {
              d->setRecursed(true);

              return createGeneralError(exec, i18n("Command resulted in a recursion."));
            }
          } else {
            return createTypeError(exec, 1);
          }
        } else if (type == Plugin::Data::IOValue::StringType) {
          KstStringPtr sp = extractString(exec, args[1], false);
          if (sp) {
            d->inputStrings().insert(input, sp);
            d->setDirty(true);
            d->setRecursed(false);
            if (d->recursion()) {
              d->setRecursed(true);

              return createGeneralError(exec, i18n("Command resulted in a recursion."));
            }
          } else {
            return createTypeError(exec, 1);
          }
        } else if (type == Plugin::Data::IOValue::FloatType ||
                   type == Plugin::Data::IOValue::PidType ) {
          KstScalarPtr sp = extractScalar(exec, args[1], false);
          if (sp) {
            d->inputScalars().insert(input, sp);
            d->setDirty(true);
            d->setRecursed(false);
            if (d->recursion()) {
              d->setRecursed(true);

              return createGeneralError(exec, i18n("Command resulted in a recursion."));
            }
          } else {
            return createTypeError(exec, 1);
          }
        } else {
          return createTypeError(exec, 1);
        }
      } else {
        return createTypeError(exec, 1);
      }
    } else {
      return createTypeError(exec, 1);
    }
    return KJS::Boolean(true);
  }

  KstBasicPluginPtr bp = makeBasicPlugin(_d);
  if (bp) {
    KstReadLocker rl(bp);

    QString input;
    unsigned int index;
    bool vector = false;
    bool scalar = false;
    bool string = false;

    if (args[0].type() == KJS::NumberType) {
      index = args[0].toUInt32(exec);

      if (index < bp->inputVectorList().size()) {
        input = bp->inputVectorList()[index];
        vector = true;
      } else if (index < bp->inputVectorList().size() + bp->inputScalarList().size()) {
        index -= bp->inputVectorList().size();
        input = bp->inputScalarList()[index];
        scalar = true;
      } else if (index < bp->inputVectorList().size() + bp->inputScalarList().size() + bp->inputStringList().size()) {
        index -= bp->inputVectorList().size() + bp->inputScalarList().size();
        input = bp->inputStringList()[index];
        string = true;
      } else {
        return createRangeError(exec, 0);
      }
    } else if (args[0].type() == KJS::StringType) {
      input = args[0].toString(exec).qstring();
      for (index = 0; index < bp->inputVectorList().size(); ++index ) {
        if (bp->inputVectorList()[index].compare(input) == 0) {
          vector = true;
          break;
        }
      }
      if (!vector) {
        for (index = 0; index < bp->inputScalarList().size(); ++index ) {
          if (bp->inputScalarList()[index].compare(input) == 0) {
            scalar = true;
            break;
          }
        }
      }
      if (!vector && !scalar) {
        for (index = 0; index < bp->inputStringList().size(); ++index ) {
          if (bp->inputStringList()[index].compare(input) == 0) {
            string = true;
            break;
          }
        }
      }

      if (!vector && !scalar && !string) {
        return createGeneralError(exec, i18n("Invalid argument name."));
      }
    } else {
      return createTypeError(exec, 0);
    }

    if (!input.isEmpty()) {
      if (vector && index < bp->inputVectorList().size()) {
        KstVectorPtr vp = extractVector(exec, args[1], false);
        if (vp) {
          bp->setInputVector(input, vp);
          bp->setDirty(true);
          bp->setRecursed(false);
          if (bp->recursion()) {
            bp->setRecursed(true);

            return createGeneralError(exec, i18n("Command resulted in a recursion."));
          }
        } else {
          return createTypeError(exec, 1);
        }
      } else if (scalar && index < bp->inputScalarList().size()) {
        KstScalarPtr sp = extractScalar(exec, args[1], false);
        if (sp) {
          bp->setInputScalar(input, sp);
          bp->setDirty(true);
          bp->setRecursed(false);
          if (bp->recursion()) {
            bp->setRecursed(true);

            return createGeneralError(exec, i18n("Command resulted in a recursion."));
          }
        } else {
          return createTypeError(exec, 1);
        }
      } else if (string && index < bp->inputStringList().size()) {
        KstStringPtr sp = extractString(exec, args[1], false);
        if (sp) {
          bp->setInputString(input, sp);
          bp->setDirty(true);
          bp->setRecursed(false);
          if (bp->recursion()) {
            bp->setRecursed(true);

            return createGeneralError(exec, i18n("Command resulted in a recursion."));
          }
        } else {
          return createTypeError(exec, 1);
        }
      } else {
        return createTypeError(exec, 1);
      }
    } else {
      return createTypeError(exec, 1);
    }
    return KJS::Boolean(true);
  }

  return KJS::Boolean(false);
}


KJS::Value KstBindPlugin::inputs(KJS::ExecState *exec) const {
  KstCPluginPtr d = makePlugin(_d);

  if (d) {
    KstReadLocker rl(d);
    if (d->plugin()) {
      return KJS::Object(new KstBindObjectCollection(exec, d, true));
    }
  } else {
    KstBasicPluginPtr bp = makeBasicPlugin(_d);

    if (bp) {
      KstReadLocker rl(bp);
      return KJS::Object(new KstBindObjectCollection(exec, bp, true));
    }
  }

  return KJS::Undefined();
}


KJS::Value KstBindPlugin::outputs(KJS::ExecState *exec) const {
  KstCPluginPtr d = makePlugin(_d);

  if (d) {
    KstReadLocker rl(d);
    if (d->plugin()) {
      return KJS::Object(new KstBindObjectCollection(exec, d, false));
    }
  } else {
    KstBasicPluginPtr bp = makeBasicPlugin(_d);

    if (bp) {
      KstReadLocker rl(bp);
      return KJS::Object(new KstBindObjectCollection(exec, bp, false));
    }
  }

  return KJS::Undefined();
}


KJS::Value KstBindPlugin::module(KJS::ExecState *exec) const {
  KstCPluginPtr d = makePlugin(_d);
  if (d) {
    KstReadLocker rl(d);
    if (d->plugin()) {
      return KJS::Object(new KstBindPluginModule(exec, d->plugin()->data()));
    }
  } else {
    KstBasicPluginPtr bp = makeBasicPlugin(_d);
    if (bp) {
      KstReadLocker rl(bp);
      return KJS::Object(new KstBindPluginModule(exec, bp));
    }
  }

  return KJS::Null();
}


void KstBindPlugin::setModule(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::ObjectType) {
    return createPropertyTypeError(exec);
  }

  KstSharedPtr<Plugin> m = KstBinding::extractPluginModule(exec, value);
  if (m) {
    KstCPluginPtr d = makePlugin(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setModule(m);

      if (!d->plugin()) {
        return createPropertyGeneralError(exec, i18n("Failed to set module."));
      }
    } else {
      KstBasicPluginPtr bp = makeBasicPlugin(_d);
      if (bp) {
        return createPropertyGeneralError(exec, i18n("Module can only be set at object creation."));
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
  } else {
    KstBasicPluginPtr bp = makeBasicPlugin(_d);
    if (bp) {
      KstReadLocker rl(bp);
      return KJS::Boolean(bp->isValid());
    }
  }

  return KJS::Boolean(false);
}

#undef makePlugin

