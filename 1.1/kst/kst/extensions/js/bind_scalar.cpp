/***************************************************************************
                               bind_scalar.cpp
                             -------------------
    begin                : Mar 28 2005
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

#include "bind_scalar.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindScalar::KstBindScalar(KJS::ExecState *exec, KstScalarPtr s)
: KstBinding("Scalar"), _s(s) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindScalar::KstBindScalar(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Scalar") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Scalar", o);
  } else {
    _s = new KstScalar;
  }
}


KstBindScalar::KstBindScalar(int id)
: KstBinding("Scalar Method", id) {
}


KstBindScalar::~KstBindScalar() {
}


KJS::Object KstBindScalar::construct(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  return KJS::Object(new KstBindScalar(exec));
}


struct ScalarBindings {
  const char *name;
  KJS::Value (KstBindScalar::*method)(KJS::ExecState*, const KJS::List&);
};


struct ScalarProperties {
  const char *name;
  void (KstBindScalar::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindScalar::*get)(KJS::ExecState*) const;
};


static ScalarBindings scalarBindings[] = {
  { 0L, 0L }
};


static ScalarProperties scalarProperties[] = {
  { "tagName", &KstBindScalar::setTagName, &KstBindScalar::tagName },
  { "value", &KstBindScalar::setValue, &KstBindScalar::value },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindScalar::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; scalarProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(scalarProperties[i].name)));
  }

  return rc;
}


bool KstBindScalar::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; scalarProperties[i].name; ++i) {
    if (prop == scalarProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindScalar::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_s) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; scalarProperties[i].name; ++i) {
    if (prop == scalarProperties[i].name) {
      if (!scalarProperties[i].set) {
        break;
      }
      (this->*scalarProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindScalar::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_s) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; scalarProperties[i].name; ++i) {
    if (prop == scalarProperties[i].name) {
      if (!scalarProperties[i].get) {
        break;
      }
      return (this->*scalarProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindScalar::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindScalar *imp = dynamic_cast<KstBindScalar*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*scalarBindings[id - 1].method)(exec, args);
}


void KstBindScalar::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; scalarBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindScalar(i + 1));
    obj.put(exec, scalarBindings[i].name, o, KJS::Function);
  }
}


void KstBindScalar::setValue(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_s);
  _s->setValue(value.toNumber(exec));
}


KJS::Value KstBindScalar::value(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::Number(_s->value());
}


void KstBindScalar::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
    if (value.type() != KJS::StringType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return;
    }
    KstWriteLocker wl(_s);
    _s->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindScalar::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_s);
  return KJS::String(_s->tagName());
}


// vim: ts=2 sw=2 et
