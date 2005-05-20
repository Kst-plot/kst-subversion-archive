/***************************************************************************
                                bind_vector.cpp
                             -------------------
    begin                : Mar 23 2005
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

#include "bind_vector.h"

#include <kstavector.h>
#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindVector::KstBindVector(KJS::ExecState *exec, KstVectorPtr v)
: KstBinding("Vector"), _v(v) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindVector::KstBindVector(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Vector") {
  kdDebug() << "Construct a new KstBindVector" << endl;
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Vector", o);
  } else {
    _v = new KstAVector(1, QString::null);
    KST::addVectorToList(_v);
  }
}


KstBindVector::KstBindVector(int id)
: KstBinding("Vector Method", id) {
}


KstBindVector::~KstBindVector() {
  kdDebug() << "Destroy a KstBindVector" << endl;
}


KJS::Object KstBindVector::construct(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  return KJS::Object(new KstBindVector(exec));
}


struct VectorBindings {
  const char *name;
  KJS::Value (KstBindVector::*method)(KJS::ExecState*, const KJS::List&);
};


struct VectorProperties {
  const char *name;
  void (KstBindVector::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindVector::*get)(KJS::ExecState*) const;
};


static VectorBindings vectorBindings[] = {
  { "resize", &KstBindVector::resize },
  { "interpolate", &KstBindVector::interpolate },
  { "zero", &KstBindVector::zero },
  { 0L, 0L }
};


static VectorProperties vectorProperties[] = {
  { "tagName", &KstBindVector::setTagName, &KstBindVector::tagName },
  { "length", 0L, &KstBindVector::length },
  { "min", 0L, &KstBindVector::min },
  { "max", 0L, &KstBindVector::max },
  { "mean", 0L, &KstBindVector::mean },
  { "newSamples", 0L, &KstBindVector::numNew },
  { "shiftedSamples", 0L, &KstBindVector::numShifted },
  { "editable", 0L, &KstBindVector::editable },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindVector::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; vectorProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(vectorProperties[i].name)));
  }

  return rc;
}


bool KstBindVector::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; vectorProperties[i].name; ++i) {
    if (prop == vectorProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


KJS::Value KstBindVector::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindVector *imp = dynamic_cast<KstBindVector*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*vectorBindings[id - 1].method)(exec, args);
}


void KstBindVector::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_v) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();

  for (int i = 0; vectorProperties[i].name; ++i) {
    if (prop == vectorProperties[i].name) {
      if (!vectorProperties[i].set) {
        break;
      }
      (this->*vectorProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindVector::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_v) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; vectorProperties[i].name; ++i) {
    if (prop == vectorProperties[i].name) {
      if (!vectorProperties[i].get) {
        break;
      }
      return (this->*vectorProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindVector::getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const {
  Q_UNUSED(exec)
  if (!_v) {
    return KJS::Undefined();
  }

  double rc;
  _v->readLock();
  if (propertyName < unsigned(_v->length())) {
    rc = _v->value()[propertyName];
  } else {
    return KJS::Undefined();
  }
  _v->readUnlock();

  return KJS::Number(rc);
}


void KstBindVector::putPropertyByIndex(KJS::ExecState *exec, unsigned propertyName, const KJS::Value &value, int attr) {
  Q_UNUSED(attr)
  if (!_v || !_v->editable()) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return;
  }

  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }

  _v->writeLock();
  if (propertyName < unsigned(_v->length())) {
    _v->value()[propertyName] = value.toNumber(exec);
    _v->setDirty();
  } else {
    KJS::Object eobj = KJS::Error::create(exec, KJS::RangeError);
    exec->setException(eobj);
  }

  _v->writeUnlock();
}


void KstBindVector::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; vectorBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindVector(i + 1));
    obj.put(exec, vectorBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindVector::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Number(_v->length());
}


KJS::Value KstBindVector::resize(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (!_v->editable()) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  unsigned sz = 0;

  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(sz)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  _v->writeLock();
  _v->resize(sz);
  _v->writeUnlock();

  return KJS::Undefined();
}


KJS::Value KstBindVector::min(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Number(_v->min());
}


KJS::Value KstBindVector::max(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Number(_v->max());
}


KJS::Value KstBindVector::mean(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Number(_v->mean());
}


KJS::Value KstBindVector::numNew(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Number(_v->numNew());
}


KJS::Value KstBindVector::numShifted(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Number(_v->numShift());
}


KJS::Value KstBindVector::interpolate(KJS::ExecState *exec, const KJS::List& args) {
  double rc = KST::NOPOINT;
  if (args.size() != 2) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly two arguments.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (!_v) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  unsigned i = 0;
  unsigned ns_i = 0;

  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(i) ||
      args[1].type() != KJS::NumberType || !args[1].toUInt32(ns_i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  // no need to check ranges - KstVector does it for us

  _v->writeLock();
  rc = _v->interpolate(i, ns_i);
  _v->writeUnlock();

  return KJS::Number(rc);
}


KJS::Value KstBindVector::zero(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  if (!_v) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }
  KstWriteLocker wl(_v);
  _v->zero();
  return KJS::Undefined();
}


void KstBindVector::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  _v->writeLock();
  _v->setTagName(value.toString(exec).qstring());
  _v->writeUnlock();
}


KJS::Value KstBindVector::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::String(_v->tagName());
}


KJS::Value KstBindVector::editable(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_v);
  return KJS::Boolean(_v->editable());
}


// vim: ts=2 sw=2 et
