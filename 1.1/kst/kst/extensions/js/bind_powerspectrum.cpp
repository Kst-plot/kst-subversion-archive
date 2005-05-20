/***************************************************************************
                            bind_powerspectrum.cpp
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

#include "bind_powerspectrum.h"
#include "bind_datavector.h"
#include "bind_vector.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindPowerSpectrum::KstBindPowerSpectrum(KJS::ExecState *exec, KstPSDPtr d)
: KstBinding("PowerSpectrum"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPowerSpectrum::KstBindPowerSpectrum(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("PowerSpectrum") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "PowerSpectrum", o);
  }
}


KstBindPowerSpectrum::KstBindPowerSpectrum(int id)
: KstBinding("PowerSpectrum Method", id) {
}


KstBindPowerSpectrum::~KstBindPowerSpectrum() {
}


KJS::Object KstBindPowerSpectrum::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() < 2) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  if (args[1].type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Object();
  }

  double freq = args[1].toNumber(exec);
  bool average = true;
  unsigned len = 16;
  bool apodize = true;
  bool removeMean = true;

  KstVectorPtr v = extractVector(exec, args[0]);
  
  if (!v) {
    return KJS::Object();
  }

  if (args.size() > 2) {
    if (args[2].type() != KJS::BooleanType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    average = args[2].toBoolean(exec);
  }

  if (args.size() > 3) {
    if (args[3].type() != KJS::NumberType || !args[3].toUInt32(len)) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
  }

  if (args.size() > 4) {
    if (args[4].type() != KJS::BooleanType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    apodize = args[4].toBoolean(exec);
  }

  if (args.size() > 5) {
    if (args[5].type() != KJS::BooleanType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    removeMean = args[5].toBoolean(exec);
  }

  if (args.size() > 6) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  KstPSDPtr d = new KstPSD(QString::null, v, freq, average, len, apodize, removeMean, "V", "Hz");

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d.data());
  KST::dataObjectList.lock().writeUnlock();

  return KJS::Object(new KstBindPowerSpectrum(exec, d));
}


struct PowerSpectrumBindings {
  const char *name;
  KJS::Value (KstBindPowerSpectrum::*method)(KJS::ExecState*, const KJS::List&);
};


struct PowerSpectrumProperties {
  const char *name;
  void (KstBindPowerSpectrum::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindPowerSpectrum::*get)(KJS::ExecState*) const;
};


static PowerSpectrumBindings powerSpectrumBindings[] = {
  { 0L, 0L }
};


static PowerSpectrumProperties powerSpectrumProperties[] = {
  { "tagName", &KstBindPowerSpectrum::setTagName, &KstBindPowerSpectrum::tagName },
  { "xVector", 0, &KstBindPowerSpectrum::xVector },
  { "yVector", 0, &KstBindPowerSpectrum::yVector },
  { "apodize", &KstBindPowerSpectrum::setApodize, &KstBindPowerSpectrum::apodize },
  { "removeMean", &KstBindPowerSpectrum::setRemoveMean, &KstBindPowerSpectrum::removeMean },
  { "average", &KstBindPowerSpectrum::setAverage, &KstBindPowerSpectrum::average },
  { "frequency", &KstBindPowerSpectrum::setFrequency, &KstBindPowerSpectrum::frequency },
  { "length", &KstBindPowerSpectrum::setLength, &KstBindPowerSpectrum::length },
  { "vUnits", &KstBindPowerSpectrum::setVUnits, &KstBindPowerSpectrum::vUnits },
  { "rUnits", &KstBindPowerSpectrum::setRUnits, &KstBindPowerSpectrum::rUnits },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPowerSpectrum::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; powerSpectrumProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(powerSpectrumProperties[i].name)));
  }

  return rc;
}


bool KstBindPowerSpectrum::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; powerSpectrumProperties[i].name; ++i) {
    if (prop == powerSpectrumProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindPowerSpectrum::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; powerSpectrumProperties[i].name; ++i) {
    if (prop == powerSpectrumProperties[i].name) {
      if (!powerSpectrumProperties[i].set) {
        break;
      }
      (this->*powerSpectrumProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPowerSpectrum::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; powerSpectrumProperties[i].name; ++i) {
    if (prop == powerSpectrumProperties[i].name) {
      if (!powerSpectrumProperties[i].get) {
        break;
      }
      return (this->*powerSpectrumProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindPowerSpectrum::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindPowerSpectrum *imp = dynamic_cast<KstBindPowerSpectrum*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*powerSpectrumBindings[id - 1].method)(exec, args);
}


void KstBindPowerSpectrum::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; powerSpectrumBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPowerSpectrum(i + 1));
    obj.put(exec, powerSpectrumBindings[i].name, o, KJS::Function);
  }
}


void KstBindPowerSpectrum::setVUnits(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setVUnits(value.toString(exec).qstring());
}


KJS::Value KstBindPowerSpectrum::vUnits(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->vUnits());
}


void KstBindPowerSpectrum::setRUnits(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setRUnits(value.toString(exec).qstring());
}


KJS::Value KstBindPowerSpectrum::rUnits(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->rUnits());
}


void KstBindPowerSpectrum::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindPowerSpectrum::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


KJS::Value KstBindPowerSpectrum::xVector(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->vX();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


KJS::Value KstBindPowerSpectrum::yVector(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->vY();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


void KstBindPowerSpectrum::setApodize(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setApodize(value.toBoolean(exec));
}


KJS::Value KstBindPowerSpectrum::apodize(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->apodize());
}


void KstBindPowerSpectrum::setRemoveMean(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setRemoveMean(value.toBoolean(exec));
}


KJS::Value KstBindPowerSpectrum::removeMean(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->removeMean());
}


void KstBindPowerSpectrum::setAverage(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setAverage(value.toBoolean(exec));
}


KJS::Value KstBindPowerSpectrum::average(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->average());
}


void KstBindPowerSpectrum::setFrequency(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setFreq(value.toNumber(exec));
}


KJS::Value KstBindPowerSpectrum::frequency(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->freq());
}


void KstBindPowerSpectrum::setLength(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  unsigned val;
  if (!value.toUInt32(val)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setLen(val);
}


KJS::Value KstBindPowerSpectrum::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->len());
}

// vim: ts=2 sw=2 et
