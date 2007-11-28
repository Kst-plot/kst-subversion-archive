/***************************************************************************
                            bind_csd.cpp
                             -------------------
    begin                : Nov 28 2007
    copyright            : (C) 2007 The University of British Columbia
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

#include "bind_csd.h"
#include "bind_matrix.h"
#include "bind_vector.h"

#include <kstdataobjectcollection.h>

#include <kdebug.h>

KstBindCSD::KstBindCSD(KJS::ExecState *exec, KstCSDPtr d)
: KstBindDataObject(exec, d.data(), "Spectrogram") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindCSD::KstBindCSD(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindDataObject(exec, globalObject, "Spectrogram") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("Spectrogram", KstBindCSD::bindFactory);
  }
}


KstBindDataObject *KstBindCSD::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  KstCSDPtr v = kst_cast<KstCSD>(obj);
  if (v) {
    return new KstBindCSD(exec, v);
  }
  return 0L;
}


KstBindCSD::KstBindCSD(int id)
: KstBindDataObject(id, "Spectrogram Method") {
}


KstBindCSD::~KstBindCSD() {
}


KJS::Object KstBindCSD::construct(KJS::ExecState *exec, const KJS::List& args) {
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

  double gaussianSigma = 0.0;
  double freq = args[1].toNumber(exec);
  unsigned len = 16;
  bool average = true;
  bool apodize = true;
  bool removeMean = true;
  int apodizeFxn = WindowOriginal;
  int output = PSDAmplitudeSpectralDensity;
  int windowSize = 5000;
  QString vunits = "V";
  QString runits = "Hz";

  KstVectorPtr v = extractVector(exec, args[0]);

  if (!v) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
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
    if (args[6].type() != KJS::StringType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    vunits = args[6].toString(exec).qstring();
  }

  if (args.size() > 7) {
    if (args[7].type() != KJS::StringType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    runits = args[7].toString(exec).qstring();
  }

  if (args.size() > 8) {
    if (args[8].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    apodizeFxn = args[8].toInt32(exec);
  }

  if (args.size() > 9) {
    if (args[9].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    gaussianSigma = args[9].toNumber(exec);
  }

  if (args.size() > 10) {
    if (args[10].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    output = args[10].toInt32(exec);
  }

  if (args.size() > 11) {
    if (args[11].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    windowSize = args[11].toInt32(exec);
  }

  if (args.size() > 12) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  KstCSDPtr d = new KstCSD(QString::null, v, freq, average, removeMean, apodize, 
                          (ApodizeFunction)apodizeFxn, windowSize, len, gaussianSigma, 
                          (PSDType)output, vunits, runits);

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d.data());
  KST::dataObjectList.lock().unlock();

  return KJS::Object(new KstBindCSD(exec, d));
}


struct CSDBindings {
  const char *name;
  KJS::Value (KstBindCSD::*method)(KJS::ExecState*, const KJS::List&);
};


struct CSDProperties {
  const char *name;
  void (KstBindCSD::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindCSD::*get)(KJS::ExecState*) const;
};


static CSDBindings csdBindings[] = {
  { 0L, 0L }
};


static CSDProperties csdProperties[] = {
  { "matrix", 0, &KstBindCSD::matrix },
  { "apodize", &KstBindCSD::setApodize, &KstBindCSD::apodize },
  { "removeMean", &KstBindCSD::setRemoveMean, &KstBindCSD::removeMean },
  { "average", &KstBindCSD::setAverage, &KstBindCSD::average },
  { "frequency", &KstBindCSD::setFrequency, &KstBindCSD::frequency },
  { "length", &KstBindCSD::setLength, &KstBindCSD::length },
  { "vUnits", &KstBindCSD::setVUnits, &KstBindCSD::vUnits },
  { "rUnits", &KstBindCSD::setRUnits, &KstBindCSD::rUnits },
  { "windowSize", &KstBindCSD::setWindowSize, &KstBindCSD::windowSize },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindCSD::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindDataObject::propList(exec, recursive);

  for (int i = 0; csdProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(csdProperties[i].name)));
  }

  return rc;
}


bool KstBindCSD::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; csdProperties[i].name; ++i) {
    if (prop == csdProperties[i].name) {
      return true;
    }
  }

  return KstBindDataObject::hasProperty(exec, propertyName);
}


void KstBindCSD::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindDataObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; csdProperties[i].name; ++i) {
    if (prop == csdProperties[i].name) {
      if (!csdProperties[i].set) {
        break;
      }
      (this->*csdProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindDataObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindCSD::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindDataObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; csdProperties[i].name; ++i) {
    if (prop == csdProperties[i].name) {
      if (!csdProperties[i].get) {
        break;
      }
      return (this->*csdProperties[i].get)(exec);
    }
  }

  return KstBindDataObject::get(exec, propertyName);
}


KJS::Value KstBindCSD::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindCSD *imp = dynamic_cast<KstBindCSD*>(self.imp());
    if (!imp) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
      exec->setException(eobj);
      return KJS::Undefined();
    }

    return (imp->*csdBindings[id - start - 1].method)(exec, args);
  }

  return KstBindDataObject::call(exec, self, args);
}


void KstBindCSD::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindDataObject::methodCount();
  for (int i = 0; csdBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindCSD(i + start + 1));
    obj.put(exec, csdBindings[i].name, o, KJS::Function);
  }
}


#define makeCSD(X) dynamic_cast<KstCSD*>(const_cast<KstObject*>(X.data()))

void KstBindCSD::setVUnits(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setVectorUnits(value.toString(exec).qstring());
  }
}


KJS::Value KstBindCSD::vUnits(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->vectorUnits());
  }
  return KJS::String();
}


void KstBindCSD::setRUnits(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setRateUnits(value.toString(exec).qstring());
  }
}


KJS::Value KstBindCSD::rUnits(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->rateUnits());
  }
  return KJS::String();
}


KJS::Value KstBindCSD::matrix(KJS::ExecState *exec) const {
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    KstMatrixPtr mp = d->outputMatrix();
    if (mp) {
      return KJS::Object(new KstBindMatrix(exec, mp));
    }
  }
  return KJS::Null();
}


void KstBindCSD::setApodize(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setApodize(value.toBoolean(exec));
  }
}


KJS::Value KstBindCSD::apodize(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->apodize());
  }
  return KJS::Boolean(false);
}


void KstBindCSD::setRemoveMean(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setRemoveMean(value.toBoolean(exec));
  }
}


KJS::Value KstBindCSD::removeMean(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->removeMean());
  }
  return KJS::Boolean(false);
}


void KstBindCSD::setAverage(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setAverage(value.toBoolean(exec));
  }
}


KJS::Value KstBindCSD::average(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->average());
  }
  return KJS::Boolean(false);
}


void KstBindCSD::setFrequency(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setFreq(value.toNumber(exec));
  }
}


KJS::Value KstBindCSD::frequency(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->freq());
  }
  return KJS::Number(0);
}


void KstBindCSD::setLength(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned val;
  if (value.type() != KJS::NumberType || !value.toUInt32(val)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setLength(val);
  }
}


KJS::Value KstBindCSD::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->length());
  }
  return KJS::Number(0);
}


void KstBindCSD::setWindowSize(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned val;
  if (value.type() != KJS::NumberType || !value.toUInt32(val)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setWindowSize(val);
  }
}


KJS::Value KstBindCSD::windowSize(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstCSDPtr d = makeCSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->windowSize());
  }
  return KJS::Number(0);
}

#undef makeCSD
