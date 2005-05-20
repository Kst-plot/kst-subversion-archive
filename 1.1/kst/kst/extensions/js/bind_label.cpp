/***************************************************************************
                               bind_label.cpp
                             -------------------
    begin                : Apr 08 2005
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

#include "bind_label.h"

#include <kst.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindLabel::KstBindLabel(KJS::ExecState *exec, KstLabelPtr d)
: KstBinding("Label"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindLabel::KstBindLabel(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Label") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Label", o);
  }
}


KstBindLabel::KstBindLabel(int id)
: KstBinding("Label Method", id) {
}


KstBindLabel::~KstBindLabel() {
}


KJS::Object KstBindLabel::construct(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  return KJS::Object(new KstBindLabel(exec));
}


struct LabelBindings {
  const char *name;
  KJS::Value (KstBindLabel::*method)(KJS::ExecState*, const KJS::List&);
};


struct LabelProperties {
  const char *name;
  void (KstBindLabel::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindLabel::*get)(KJS::ExecState*) const;
};


static LabelBindings labelBindings[] = {
  { 0L, 0L }
};


static LabelProperties labelProperties[] = {
  { "text", &KstBindLabel::setText, &KstBindLabel::text },
  { "rotation", &KstBindLabel::setRotation, &KstBindLabel::rotation },
  { "interpreted", &KstBindLabel::setInterpreted, &KstBindLabel::interpreted },
  { "scalarReplacement", &KstBindLabel::setScalarReplacement, &KstBindLabel::scalarReplacement },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindLabel::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; labelProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(labelProperties[i].name)));
  }

  return rc;
}


bool KstBindLabel::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; labelProperties[i].name; ++i) {
    if (prop == labelProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindLabel::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; labelProperties[i].name; ++i) {
    if (prop == labelProperties[i].name) {
      if (!labelProperties[i].set) {
        break;
      }
      (this->*labelProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindLabel::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; labelProperties[i].name; ++i) {
    if (prop == labelProperties[i].name) {
      if (!labelProperties[i].get) {
        break;
      }
      return (this->*labelProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindLabel::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindLabel *imp = dynamic_cast<KstBindLabel*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*labelBindings[id - 1].method)(exec, args);
}


void KstBindLabel::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; labelBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindLabel(i + 1));
    obj.put(exec, labelBindings[i].name, o, KJS::Function);
  }
}


void KstBindLabel::setText(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setText(value.toString(exec).qstring());
  KstApp::inst()->paintAll(P_PAINT);
}


KJS::Value KstBindLabel::text(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->text());
}


void KstBindLabel::setRotation(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setRotation(value.toNumber(exec));
  KstApp::inst()->paintAll(P_PAINT);
}


KJS::Value KstBindLabel::rotation(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->rotation());
}


void KstBindLabel::setInterpreted(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setInterpreted(value.toBoolean(exec));
  KstApp::inst()->paintAll(P_PAINT);
}


KJS::Value KstBindLabel::interpreted(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->interpreted());
}


void KstBindLabel::setScalarReplacement(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setDoScalarReplacement(value.toBoolean(exec));
  KstApp::inst()->paintAll(P_PAINT);
}


KJS::Value KstBindLabel::scalarReplacement(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->doScalarReplacement());
}


// vim: ts=2 sw=2 et
