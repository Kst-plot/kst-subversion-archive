/***************************************************************************
                           bind_objectcollection.cpp
                             -------------------
    begin                : May 31 2005
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
#include "bind_object.h"

#include <qdeepcopy.h>

#include <kstdatacollection.h>
#include <kstobject.h>
#include <plugin.h>

#include <kdebug.h>

KstBindObjectCollection::KstBindObjectCollection(KJS::ExecState *exec, const KstObjectList<KstObjectPtr>& objects)
: KstBindCollection(exec, "ObjectCollection", true) {
  _objects = QDeepCopy<KstObjectList<KstObjectPtr> >(objects);
  _inputs = false;
}


KstBindObjectCollection::KstBindObjectCollection(KJS::ExecState *exec, const KstCPluginPtr plugin, bool inputs)
: KstBindCollection(exec, "ObjectCollection", true) {
  _plugin = plugin;
  _inputs = inputs;
}


KstBindObjectCollection::KstBindObjectCollection(KJS::ExecState *exec)
: KstBindCollection(exec, "ObjectCollection", true) {
  _inputs = false;
}


KstBindObjectCollection::~KstBindObjectCollection() {
}


KJS::Value KstBindObjectCollection::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  if (_plugin) {
    if (_plugin->plugin()) {
      if (_inputs) {
        return KJS::Number(_plugin->plugin()->data()._inputs.size());
      } else {
        return KJS::Number(_plugin->plugin()->data()._outputs.size());
      }
    } else {
      return KJS::Undefined();
    }
  }

  return KJS::Number(_objects.count());
}


QStringList KstBindObjectCollection::collection(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  if (_plugin) {
    if (_plugin->plugin()) {
      return _plugin->inputVectors().tagNames() + _plugin->inputStrings().tagNames() + _plugin->inputScalars().tagNames();
    }
  }

  return _objects.tagNames();
}


KJS::Value KstBindObjectCollection::extract(KJS::ExecState *exec, const KJS::Identifier& item) const {
  KstObjectPtr vp;

  if (_plugin) {
    if (_plugin->plugin()) {
      if (_inputs) {
        for (QValueList<Plugin::Data::IOValue>::ConstIterator it = _plugin->plugin()->data()._inputs.begin(); it != _plugin->plugin()->data()._inputs.end(); ++it) {
          if (item.qstring().compare((*it)._name) == 0) {
            Plugin::Data::IOValue::ValueType type = (*it)._type;

            if (type == Plugin::Data::IOValue::TableType) {
              vp = _plugin->inputVectors()[(*it)._name];
            } else if (type == Plugin::Data::IOValue::StringType) {
              vp = _plugin->inputStrings()[(*it)._name];
            } else if (type == Plugin::Data::IOValue::FloatType ||
                      type == Plugin::Data::IOValue::PidType) {
              vp = _plugin->inputScalars()[(*it)._name];
            }

            break;
          }
        }
      } else {
        for (QValueList<Plugin::Data::IOValue>::ConstIterator it = _plugin->plugin()->data()._outputs.begin(); it != _plugin->plugin()->data()._outputs.end(); ++it) {
          if (item.qstring().compare((*it)._name) == 0) {
            Plugin::Data::IOValue::ValueType type = (*it)._type;

            if (type == Plugin::Data::IOValue::TableType) {
              vp = _plugin->outputVectors()[(*it)._name];
            } else if (type == Plugin::Data::IOValue::StringType) {
              vp = _plugin->outputStrings()[(*it)._name];
            } else if (type == Plugin::Data::IOValue::FloatType ||
                      type == Plugin::Data::IOValue::PidType) {
              vp = _plugin->outputScalars()[(*it)._name];
            }

            break;
          }
        }
      }
    }
  } else {
    vp = *_objects.findTag(item.qstring());
  }

  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindObject(exec, vp));
}


KJS::Value KstBindObjectCollection::extract(KJS::ExecState *exec, unsigned item) const {
  KstObjectPtr vp;

  if (_plugin) {
    if (_plugin->plugin()) {
      if (_inputs) {
        if (item < _plugin->plugin()->data()._inputs.size()) {
          Plugin::Data::IOValue::ValueType type;

          type = _plugin->plugin()->data()._inputs[item]._type;

          if (type == Plugin::Data::IOValue::TableType) {
            vp = _plugin->inputVectors()[_plugin->plugin()->data()._inputs[item]._name];
          } else if (type == Plugin::Data::IOValue::StringType) {
            vp = _plugin->inputStrings()[_plugin->plugin()->data()._inputs[item]._name];
          } else if (type == Plugin::Data::IOValue::FloatType ||
                     type == Plugin::Data::IOValue::PidType) {
            vp = _plugin->inputScalars()[_plugin->plugin()->data()._inputs[item]._name];
          }
        }
      } else {
        if (item < _plugin->plugin()->data()._outputs.size()) {
          Plugin::Data::IOValue::ValueType type;

          type = _plugin->plugin()->data()._outputs[item]._type;

          if (type == Plugin::Data::IOValue::TableType) {
            vp = _plugin->inputVectors()[_plugin->plugin()->data()._outputs[item]._name];
          } else if (type == Plugin::Data::IOValue::StringType) {
            vp = _plugin->inputStrings()[_plugin->plugin()->data()._outputs[item]._name];
          } else if (type == Plugin::Data::IOValue::FloatType ||
                     type == Plugin::Data::IOValue::PidType) {
            vp = _plugin->inputScalars()[_plugin->plugin()->data()._outputs[item]._name];
          }
        }
      }
    }
  } else if (item < _objects.count()) {
    vp = _objects[item];
  }

  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindObject(exec, vp));
}
