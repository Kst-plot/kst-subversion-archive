/***************************************************************************
                              bind_equation.cpp
                             -------------------
    begin                : Mar 29 2005
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

#include "bind_equation.h"
#include "bind_datavector.h"
#include "bind_vector.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindEquation::KstBindEquation(KJS::ExecState *exec, KstEquationPtr d)
: KstBinding("Equation"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindEquation::KstBindEquation(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Equation") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Equation", o);
  }
}


KstBindEquation::KstBindEquation(int id)
: KstBinding("Equation Method", id) {
}


KstBindEquation::~KstBindEquation() {
}


KJS::Object KstBindEquation::construct(KJS::ExecState *exec, const KJS::List& args) {
  KstEquationPtr d;

  // KstEquation(equation, xVector[, interp = true])
  if (args.size() == 2 || args.size() == 3) {
    if (args[0].type() != KJS::StringType ||
        (args.size() == 3 && args[2].type() != KJS::BooleanType)) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }

    KstVectorPtr vp = extractVector(exec, args[1]);

    if (!vp) {
      return KJS::Object();
    }

    bool interp = true;

    QString eq = args[0].toString(exec).qstring();
    if (args.size() == 3) {
      interp = args[2].toBoolean(exec);
    }

    d = new KstEquation(QString::null, eq, vp, interp);
  }

  // KstEquation(equation, x0, x1, nx)
  if (args.size() == 4) {
    if (args[0].type() != KJS::StringType ||
        args[1].type() != KJS::NumberType ||
        args[2].type() != KJS::NumberType ||
        args[3].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }

    unsigned npts;
    if (!args[3].toUInt32(npts)) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }

    d = new KstEquation(QString::null, args[0].toString(exec).qstring(), args[1].toNumber(exec), args[2].toNumber(exec), npts);
  }

  if (!d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d.data());
  KST::dataObjectList.lock().writeUnlock();

  return KJS::Object(new KstBindEquation(exec, d));
}


struct EquationBindings {
  const char *name;
  KJS::Value (KstBindEquation::*method)(KJS::ExecState*, const KJS::List&);
};


struct EquationProperties {
  const char *name;
  void (KstBindEquation::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindEquation::*get)(KJS::ExecState*) const;
};


static EquationBindings equationBindings[] = {
  { 0L, 0L }
};


static EquationProperties equationProperties[] = {
  { "tagName", &KstBindEquation::setTagName, &KstBindEquation::tagName },
  { "equation", &KstBindEquation::setEquation, &KstBindEquation::equation },
  { "valid", 0L, &KstBindEquation::valid },
  { "xVector", 0L, &KstBindEquation::xVector },
  { "yVector", 0L, &KstBindEquation::yVector },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindEquation::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; equationProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(equationProperties[i].name)));
  }

  return rc;
}


bool KstBindEquation::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; equationProperties[i].name; ++i) {
    if (prop == equationProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindEquation::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; equationProperties[i].name; ++i) {
    if (prop == equationProperties[i].name) {
      if (!equationProperties[i].set) {
        break;
      }
      (this->*equationProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindEquation::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; equationProperties[i].name; ++i) {
    if (prop == equationProperties[i].name) {
      if (!equationProperties[i].get) {
        break;
      }
      return (this->*equationProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindEquation::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindEquation *imp = dynamic_cast<KstBindEquation*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*equationBindings[id - 1].method)(exec, args);
}


void KstBindEquation::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; equationBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindEquation(i + 1));
    obj.put(exec, equationBindings[i].name, o, KJS::Function);
  }
}


void KstBindEquation::setEquation(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setEquation(value.toString(exec).qstring());
}


KJS::Value KstBindEquation::equation(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->equation());
}


void KstBindEquation::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindEquation::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


KJS::Value KstBindEquation::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->isValid());
}


KJS::Value KstBindEquation::xVector(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->vX();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


KJS::Value KstBindEquation::yVector(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->vY();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


// vim: ts=2 sw=2 et
