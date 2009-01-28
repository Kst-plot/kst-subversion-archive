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

#include <kstdataobjectcollection.h>

#include <kdebug.h>

KstBindPowerSpectrum::KstBindPowerSpectrum(KJS::ExecState *exec, KstPSDPtr d)
: KstBindDataObject(exec, d.data(), "PowerSpectrum") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPowerSpectrum::KstBindPowerSpectrum(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindDataObject(exec, globalObject, "PowerSpectrum") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("PowerSpectrum", KstBindPowerSpectrum::bindFactory);
  }
}


KstBindDataObject *KstBindPowerSpectrum::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  KstPSDPtr v = kst_cast<KstPSD>(obj);
  if (v) {
    return new KstBindPowerSpectrum(exec, v);
  }
  return 0L;
}


KstBindPowerSpectrum::KstBindPowerSpectrum(int id)
: KstBindDataObject(id, "PowerSpectrum Method") {
}


KstBindPowerSpectrum::~KstBindPowerSpectrum() {
}


KJS::Object KstBindPowerSpectrum::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() < 2) {
    return createSyntaxError(exec);
  }

  if (args[1].type() != KJS::NumberType) {
    return createTypeError(exec, 1);
  }

  double gaussianSigma = 0.0;
  double freq = args[1].toNumber(exec);
  bool average = true;
  unsigned len = 16;
  bool apodize = true;
  bool removeMean = true;
  int apodizeFxn = WindowOriginal;
  int output = PSDAmplitudeSpectralDensity;
  QString vunits = "V";
  QString runits = "Hz";

  KstVectorPtr v = extractVector(exec, args[0]);
  if (!v) {
    return createTypeError(exec, 0);
  }

  if (args.size() > 2) {
    if (args[2].type() != KJS::BooleanType) {
      return createTypeError(exec, 2);
    }
    average = args[2].toBoolean(exec);
  }

  if (args.size() > 3) {
    if (args[3].type() != KJS::NumberType || !args[3].toUInt32(len)) {
      return createTypeError(exec, 3);
    }
  }

  if (args.size() > 4) {
    if (args[4].type() != KJS::BooleanType) {
      return createTypeError(exec, 4);
    }
    apodize = args[4].toBoolean(exec);
  }

  if (args.size() > 5) {
    if (args[5].type() != KJS::BooleanType) {
      return createTypeError(exec, 5);
    }
    removeMean = args[5].toBoolean(exec);
  }

  if (args.size() > 6) {
    if (args[6].type() != KJS::StringType) {
      return createTypeError(exec, 6);
    }
    vunits = args[6].toString(exec).qstring();
  }

  if (args.size() > 7) {
    if (args[7].type() != KJS::StringType) {
      return createTypeError(exec, 7);
    }
    runits = args[7].toString(exec).qstring();
  }

  if (args.size() > 8) {
    if (args[8].type() != KJS::NumberType) {
      return createTypeError(exec, 8);
    }
    apodizeFxn = args[8].toInt32(exec);
  }

  if (args.size() > 9) {
    if (args[9].type() != KJS::NumberType) {
      return createTypeError(exec, 9);
    }
    gaussianSigma = args[9].toNumber(exec);
  }

  if (args.size() > 10) {
    if (args[10].type() != KJS::NumberType) {
      return createTypeError(exec, 10);
    }
    output = args[10].toInt32(exec);
  }

  if (args.size() > 11) {
    return createSyntaxError(exec);
  }

  KstPSDPtr d = new KstPSD(QString::null, v, freq, average, len, apodize, removeMean, 
                          vunits, runits, (ApodizeFunction)apodizeFxn, gaussianSigma, 
                          (PSDType)output);

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d.data());
  KST::dataObjectList.lock().unlock();

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
  { "vector", &KstBindPowerSpectrum::setVector, &KstBindPowerSpectrum::vector },
  { "xVector", 0, &KstBindPowerSpectrum::xVector },
  { "yVector", 0, &KstBindPowerSpectrum::yVector },
  { "apodize", &KstBindPowerSpectrum::setApodize, &KstBindPowerSpectrum::apodize },
  { "removeMean", &KstBindPowerSpectrum::setRemoveMean, &KstBindPowerSpectrum::removeMean },
  { "average", &KstBindPowerSpectrum::setAverage, &KstBindPowerSpectrum::average },
  { "frequency", &KstBindPowerSpectrum::setFrequency, &KstBindPowerSpectrum::frequency },
  { "length", &KstBindPowerSpectrum::setLength, &KstBindPowerSpectrum::length },
  { "vUnits", &KstBindPowerSpectrum::setVUnits, &KstBindPowerSpectrum::vUnits },
  { "rUnits", &KstBindPowerSpectrum::setRUnits, &KstBindPowerSpectrum::rUnits },
  { "output", &KstBindPowerSpectrum::setOutput, &KstBindPowerSpectrum::output },
  { "apodizeFn", &KstBindPowerSpectrum::setApodizeFn, &KstBindPowerSpectrum::apodizeFn },
  { "interpolateHoles", &KstBindPowerSpectrum::setInterpolateHoles, &KstBindPowerSpectrum::interpolateHoles },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPowerSpectrum::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindDataObject::propList(exec, recursive);

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

  return KstBindDataObject::hasProperty(exec, propertyName);
}


void KstBindPowerSpectrum::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindDataObject::put(exec, propertyName, value, attr);
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

  KstBindDataObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPowerSpectrum::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindDataObject::get(exec, propertyName);
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

  return KstBindDataObject::get(exec, propertyName);
}


KJS::Value KstBindPowerSpectrum::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindPowerSpectrum *imp = dynamic_cast<KstBindPowerSpectrum*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*powerSpectrumBindings[id - start - 1].method)(exec, args);
  }

  return KstBindDataObject::call(exec, self, args);
}


void KstBindPowerSpectrum::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindDataObject::methodCount();
  for (int i = 0; powerSpectrumBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPowerSpectrum(i + start + 1));
    obj.put(exec, powerSpectrumBindings[i].name, o, KJS::Function);
  }
}


#define makePSD(X) dynamic_cast<KstPSD*>(const_cast<KstObject*>(X.data()))

void KstBindPowerSpectrum::setVUnits(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setVUnits(value.toString(exec).qstring());
  }
}


KJS::Value KstBindPowerSpectrum::vUnits(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->vUnits());
  }
  return KJS::String();
}


void KstBindPowerSpectrum::setRUnits(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setRUnits(value.toString(exec).qstring());
  }
}


KJS::Value KstBindPowerSpectrum::rUnits(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->rUnits());
  }
  return KJS::String();
}


void KstBindPowerSpectrum::setVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstPSDPtr d = makePSD(_d);
  if (!d) {
    createInternalError(exec);
  }

  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstWriteLocker wl(d);
    d->setVector(v);
    d->setDirty();
    d->setRecursed(false);
    if (d->recursion()) {
      d->setRecursed(true);

      createGeneralError(exec, i18n("Command resulted in a recursion."));
    }
  }
}


KJS::Value KstBindPowerSpectrum::vector(KJS::ExecState *exec) const {
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr vp = d->vector();
    if (vp) {
      return KJS::Object(new KstBindVector(exec, vp));
    }
  }
  return KJS::Null();
}


KJS::Value KstBindPowerSpectrum::xVector(KJS::ExecState *exec) const {
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr vp = d->vX();
    if (vp) {
      return KJS::Object(new KstBindVector(exec, vp));
    }
  }
  return KJS::Null();
}


KJS::Value KstBindPowerSpectrum::yVector(KJS::ExecState *exec) const {
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr vp = d->vY();
    if (vp) {
      return KJS::Object(new KstBindVector(exec, vp));
    }
  }
  return KJS::Undefined();
}


void KstBindPowerSpectrum::setApodize(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setApodize(value.toBoolean(exec));
  }
}


KJS::Value KstBindPowerSpectrum::apodize(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->apodize());
  }
  return KJS::Boolean(false);
}


void KstBindPowerSpectrum::setRemoveMean(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setRemoveMean(value.toBoolean(exec));
  }
}


KJS::Value KstBindPowerSpectrum::removeMean(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->removeMean());
  }
  return KJS::Boolean(false);
}


void KstBindPowerSpectrum::setAverage(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setAverage(value.toBoolean(exec));
  }
}


KJS::Value KstBindPowerSpectrum::average(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->average());
  }
  return KJS::Boolean(false);
}


void KstBindPowerSpectrum::setFrequency(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setFreq(value.toNumber(exec));
  }
}


KJS::Value KstBindPowerSpectrum::frequency(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->freq());
  }
  return KJS::Number(0);
}


void KstBindPowerSpectrum::setLength(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned val;
  if (value.type() != KJS::NumberType || !value.toUInt32(val)) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setLen(val);
  }
}


KJS::Value KstBindPowerSpectrum::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->len());
  }
  return KJS::Number(0);
}


void KstBindPowerSpectrum::setOutput(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    return createPropertyTypeError(exec);
  }

  int val = value.toInt32(exec);

  if (val < PSDAmplitudeSpectralDensity || val > PSDPowerSpectrum ) {
    return createPropertyRangeError(exec);
  }

  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setOutput((PSDType)val);
  }
}


KJS::Value KstBindPowerSpectrum::output(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->output());
  }
  return KJS::Number(0);
}


void KstBindPowerSpectrum::setApodizeFn(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    return createPropertyTypeError(exec);
  }

  int val = value.toInt32(exec);

  if (val < WindowOriginal || val > WindowUniform ) {
    return createPropertyRangeError(exec);
  }

  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setApodizeFxn((ApodizeFunction)val);
  }
}


KJS::Value KstBindPowerSpectrum::apodizeFn(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->apodizeFxn());
  }
  return KJS::Number(0);
}


void KstBindPowerSpectrum::setInterpolateHoles(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setInterpolateHoles(value.toBoolean(exec));
  }
}


KJS::Value KstBindPowerSpectrum::interpolateHoles(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstPSDPtr d = makePSD(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->interpolateHoles());
  }
  return KJS::Boolean(false);
}

#undef makePSD

