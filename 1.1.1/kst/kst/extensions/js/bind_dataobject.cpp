/***************************************************************************
                             bind_dataobject.cpp
                             -------------------
    begin                : Apr 10 2005
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

#include "bind_dataobject.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindDataObject::KstBindDataObject(KJS::ExecState *exec, KstDataObjectPtr d)
: KstBinding("DataObject", false), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindDataObject::KstBindDataObject(int id)
: KstBinding("DataObject Method", id) {
}


KstBindDataObject::~KstBindDataObject() {
}


struct DataObjectBindings {
  const char *name;
  KJS::Value (KstBindDataObject::*method)(KJS::ExecState*, const KJS::List&);
};


struct DataObjectProperties {
  const char *name;
  void (KstBindDataObject::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindDataObject::*get)(KJS::ExecState*) const;
};


static DataObjectBindings dataObjectBindings[] = {
  { 0L, 0L }
};


static DataObjectProperties dataObjectProperties[] = {
  { "tagName", &KstBindDataObject::setTagName, &KstBindDataObject::tagName },
  { "type", 0L, &KstBindDataObject::type },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindDataObject::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; dataObjectProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(dataObjectProperties[i].name)));
  }

  return rc;
}


bool KstBindDataObject::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; dataObjectProperties[i].name; ++i) {
    if (prop == dataObjectProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindDataObject::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; dataObjectProperties[i].name; ++i) {
    if (prop == dataObjectProperties[i].name) {
      if (!dataObjectProperties[i].set) {
        break;
      }
      (this->*dataObjectProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindDataObject::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; dataObjectProperties[i].name; ++i) {
    if (prop == dataObjectProperties[i].name) {
      if (!dataObjectProperties[i].get) {
        break;
      }
      return (this->*dataObjectProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindDataObject::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindDataObject *imp = dynamic_cast<KstBindDataObject*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*dataObjectBindings[id - 1].method)(exec, args);
}


void KstBindDataObject::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; dataObjectBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindDataObject(i + 1));
    obj.put(exec, dataObjectBindings[i].name, o, KJS::Function);
  }
}


void KstBindDataObject::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindDataObject::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


KJS::Value KstBindDataObject::type(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->typeString());
}

// vim: ts=2 sw=2 et
