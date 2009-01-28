/***************************************************************************
                             bind_crosspowerspectrum.cpp
                             -------------------
    begin                : Feb 15 2008
    copyright            : (C) 2008 The University of British Columbia
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

#include "bind_crosspowerspectrum.h"
#include "bind_scalar.h"
#include "bind_vector.h"

#include <kstdatacollection.h>
#include <kstdataobjectcollection.h>
#include <kstobject.h>
#include <kstobjectcollection.h>
#include <kdebug.h>
#include <qobject.h>

static const QString& VECTOR_ONE = KGlobal::staticQString("Vector One");
static const QString& VECTOR_TWO = KGlobal::staticQString("Vector Two");
static const QString& FFT_LENGTH = KGlobal::staticQString("FFT Length = 2^");
static const QString& SAMPLE_RATE = KGlobal::staticQString("Sample Rate");
static const QString& REAL = KGlobal::staticQString("Cross Spectrum: Real");
static const QString& IMAGINARY = KGlobal::staticQString("Cross Spectrum: Imaginary");
static const QString& FREQUENCY = KGlobal::staticQString("Frequency");

KstBindCrossPowerSpectrum::KstBindCrossPowerSpectrum(KJS::ExecState *exec, KstDataObjectPtr d)
: KstBindDataObject(exec, d.data(), "CrossPowerSpectrum") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindCrossPowerSpectrum::KstBindCrossPowerSpectrum(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindDataObject(exec, globalObject, "CrossPowerSpectrum") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("CrossPowerSpectrum", KstBindCrossPowerSpectrum::bindFactory);
  }
}


KstBindDataObject *KstBindCrossPowerSpectrum::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  if (obj) {
    return new KstBindCrossPowerSpectrum(exec, obj);
  }

  return 0L;
}


KstBindCrossPowerSpectrum::KstBindCrossPowerSpectrum(int id, const char *name)
: KstBindDataObject(id, name ? name : "CrossPowerSpectrum Method") {
}


KstBindCrossPowerSpectrum::~KstBindCrossPowerSpectrum() {
}


KJS::Object KstBindCrossPowerSpectrum::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstDataObjectPtr d = KstDataObject::createPlugin(QString("Cross Power Spectrum"));
  return KJS::Object(new KstBindCrossPowerSpectrum(exec, d));
}


struct CrossPowerSpectrumBindings {
  const char *name;
  KJS::Value (KstBindCrossPowerSpectrum::*method)(KJS::ExecState*, const KJS::List&);
};


struct CrossPowerSpectrumProperties {
  const char *name;
  void (KstBindCrossPowerSpectrum::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindCrossPowerSpectrum::*get)(KJS::ExecState*) const;
};


static CrossPowerSpectrumBindings crossPowerSpectrumBindings[] = {
  { "validate", &KstBindCrossPowerSpectrum::validate },
  { 0L, 0L }
};


static CrossPowerSpectrumProperties crossPowerSpectrumProperties[] = {
  { "v1", &KstBindCrossPowerSpectrum::setV1, &KstBindCrossPowerSpectrum::v1 },
  { "v2", &KstBindCrossPowerSpectrum::setV2, &KstBindCrossPowerSpectrum::v2 },
  { "length", &KstBindCrossPowerSpectrum::setLength, &KstBindCrossPowerSpectrum::length },
  { "sample", &KstBindCrossPowerSpectrum::setSample, &KstBindCrossPowerSpectrum::sample },
  { "real", &KstBindCrossPowerSpectrum::setReal, &KstBindCrossPowerSpectrum::real },
  { "imaginary", &KstBindCrossPowerSpectrum::setImaginary, &KstBindCrossPowerSpectrum::imaginary },
  { "frequency", &KstBindCrossPowerSpectrum::setFrequency, &KstBindCrossPowerSpectrum::frequency },
  { "valid", 0L, &KstBindCrossPowerSpectrum::valid },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindCrossPowerSpectrum::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindDataObject::propList(exec, recursive);

  for (int i = 0; crossPowerSpectrumProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(crossPowerSpectrumProperties[i].name)));
  }

  return rc;
}


bool KstBindCrossPowerSpectrum::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; crossPowerSpectrumProperties[i].name; ++i) {
    if (prop == crossPowerSpectrumProperties[i].name) {
      return true;
    }
  }

  return KstBindDataObject::hasProperty(exec, propertyName);
}


void KstBindCrossPowerSpectrum::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindDataObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; crossPowerSpectrumProperties[i].name; ++i) {
    if (prop == crossPowerSpectrumProperties[i].name) {
      if (!crossPowerSpectrumProperties[i].set) {
        break;
      }
      (this->*crossPowerSpectrumProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindDataObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindCrossPowerSpectrum::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindDataObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; crossPowerSpectrumProperties[i].name; ++i) {
    if (prop == crossPowerSpectrumProperties[i].name) {
      if (!crossPowerSpectrumProperties[i].get) {
        break;
      }
      return (this->*crossPowerSpectrumProperties[i].get)(exec);
    }
  }

  return KstBindDataObject::get(exec, propertyName);
}


KJS::Value KstBindCrossPowerSpectrum::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindCrossPowerSpectrum *imp = dynamic_cast<KstBindCrossPowerSpectrum*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*crossPowerSpectrumBindings[id - start - 1].method)(exec, args);
  }

  return KstBindDataObject::call(exec, self, args);
}


void KstBindCrossPowerSpectrum::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindDataObject::methodCount();

  for (int i = 0; crossPowerSpectrumBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindCrossPowerSpectrum(i + start + 1));
    obj.put(exec, crossPowerSpectrumBindings[i].name, o, KJS::Function);
  }
}

#define makeDataObject(X) dynamic_cast<KstDataObject*>(const_cast<KstObject*>(X.data()))


KJS::Value KstBindCrossPowerSpectrum::validate(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    if (*(d->inputVectors().find(VECTOR_ONE)) && 
        *(d->inputVectors().find(VECTOR_TWO)) && 
        *(d->inputScalars().find(FFT_LENGTH)) && 
        *(d->inputScalars().find(SAMPLE_RATE)) && 
        *(d->outputVectors().find(REAL)) && 
        *(d->outputVectors().find(IMAGINARY)) && 
        *(d->outputVectors().find(FREQUENCY))) {
      KST::dataObjectList.lock().writeLock();
      KST::dataObjectList.append(d.data());
      KST::dataObjectList.lock().unlock();

      return KJS::Boolean(true);
    }
  }

  return KJS::Boolean(false);
}


void KstBindCrossPowerSpectrum::setV1(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (v) {
        d->inputVectors()[VECTOR_ONE] = v;
      } else {
        d->inputVectors().remove(VECTOR_ONE);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindCrossPowerSpectrum::v1(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->inputVectors().find(VECTOR_ONE));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindCrossPowerSpectrum::setV2(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (v) {
        d->inputVectors()[VECTOR_TWO] = v;
      } else {
        d->inputVectors().remove(VECTOR_TWO);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindCrossPowerSpectrum::v2(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->inputVectors().find(VECTOR_TWO));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();

}


void KstBindCrossPowerSpectrum::setLength(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[FFT_LENGTH] = s;
      } else {
        d->inputScalars().remove(FFT_LENGTH);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindCrossPowerSpectrum::length(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(FFT_LENGTH));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindCrossPowerSpectrum::setSample(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[SAMPLE_RATE] = s;
      } else {
        d->inputScalars().remove(SAMPLE_RATE);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindCrossPowerSpectrum::sample(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(SAMPLE_RATE));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindCrossPowerSpectrum::setReal(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstWriteLocker rl(d);
    QString name = value.toString(exec).qstring();
    QString tname;

    if (name.isEmpty()) {
      tname = i18n("the real part of a complex number", "real");
    } else {
      tname = name;
    }
    KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
    KstVectorPtr v = new KstVector(KstObjectTag(tname, d->tag()), 0, d, false);
    d->outputVectors().insert(REAL, v);
  }
}


KJS::Value KstBindCrossPowerSpectrum::real(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->outputVectors().find(REAL));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindCrossPowerSpectrum::setImaginary(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstWriteLocker rl(d);
    QString name = value.toString(exec).qstring();
    QString tname;

    if (name.isEmpty()) {
      tname = i18n("the imaginary part of a complex number", "imaginary");
    } else {
      tname = name;
    }
    KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
    KstVectorPtr v = new KstVector(KstObjectTag(tname, d->tag()), 0, d, false);
    d->outputVectors().insert(IMAGINARY, v);
  }
}


KJS::Value KstBindCrossPowerSpectrum::imaginary(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->outputVectors().find(IMAGINARY));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindCrossPowerSpectrum::setFrequency(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstWriteLocker rl(d);
    QString name = value.toString(exec).qstring();
    QString tname;

    if (name.isEmpty()) {
      tname = i18n("frequency");
    } else {
      tname = name;
    }
    KstWriteLocker blockVectorUpdates(&KST::vectorList.lock());
    KstVectorPtr v = new KstVector(KstObjectTag(tname, d->tag()), 0, d, false);
    d->outputVectors().insert(FREQUENCY, v);
  }
}


KJS::Value KstBindCrossPowerSpectrum::frequency(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->outputVectors().find(FREQUENCY));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


KJS::Value KstBindCrossPowerSpectrum::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    if (*(d->inputVectors().find(VECTOR_ONE)) && 
        *(d->inputVectors().find(VECTOR_TWO)) && 
        *(d->inputScalars().find(FFT_LENGTH)) && 
        *(d->inputScalars().find(SAMPLE_RATE)) && 
        *(d->outputVectors().find(REAL)) && 
        *(d->outputVectors().find(IMAGINARY)) && 
        *(d->outputVectors().find(FREQUENCY))) {
      return KJS::Boolean(true);
    }
  }
  return KJS::Boolean(false);
}

#undef makeDataObject

