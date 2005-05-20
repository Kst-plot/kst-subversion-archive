/***************************************************************************
                              bind_histogram.cpp
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

#include "bind_histogram.h"
#include "bind_vector.h"

#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindHistogram::KstBindHistogram(KJS::ExecState *exec, KstHistogramPtr d)
: KstBinding("Histogram"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindHistogram::KstBindHistogram(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Histogram") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Histogram", o);
  }
}


KstBindHistogram::KstBindHistogram(int id)
: KstBinding("Histogram Method", id) {
}


KstBindHistogram::~KstBindHistogram() {
}


KJS::Object KstBindHistogram::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() < 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  double xmin = -10.0;
  double xmax = 10.0;
  unsigned bins = 60;

  KstVectorPtr v = extractVector(exec, args[0]);
  
  if (!v) {
    return KJS::Object();
  }

  if (args.size() > 1) {
    if (args[1].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    xmin = args[1].toNumber(exec);
  }

  if (args.size() > 2) {
    if (args[2].type() != KJS::NumberType) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
    xmax = args[2].toNumber(exec);
  }

  if (args.size() > 3) {
    if (args[3].type() != KJS::NumberType || !args[3].toUInt32(bins)) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
  }

  if (args.size() > 4) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  KstHistogramPtr d = new KstHistogram(QString::null, v, xmin, xmax, bins, KST_HS_NUMBER);

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d.data());
  KST::dataObjectList.lock().writeUnlock();

  return KJS::Object(new KstBindHistogram(exec, d));
}


struct HistogramBindings {
  const char *name;
  KJS::Value (KstBindHistogram::*method)(KJS::ExecState*, const KJS::List&);
};


struct HistogramProperties {
  const char *name;
  void (KstBindHistogram::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindHistogram::*get)(KJS::ExecState*) const;
};


static HistogramBindings histogramBindings[] = {
  { "setVector", &KstBindHistogram::setVector },
  { "setRange", &KstBindHistogram::setRange },
  { 0L, 0L }
};


static HistogramProperties histogramProperties[] = {
  { "tagName", &KstBindHistogram::setTagName, &KstBindHistogram::tagName },
  { "realTimeAutoBin", &KstBindHistogram::setRealTimeAutoBin, &KstBindHistogram::realTimeAutoBin },
  { "bins", &KstBindHistogram::setBins, &KstBindHistogram::bins },
  { "xVector", 0L, &KstBindHistogram::xVector },
  { "yVector", 0L, &KstBindHistogram::yVector },
  { "xMin", 0L, &KstBindHistogram::xMin },
  { "xMax", 0L, &KstBindHistogram::xMax },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindHistogram::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; histogramProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(histogramProperties[i].name)));
  }

  return rc;
}


bool KstBindHistogram::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; histogramProperties[i].name; ++i) {
    if (prop == histogramProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindHistogram::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; histogramProperties[i].name; ++i) {
    if (prop == histogramProperties[i].name) {
      if (!histogramProperties[i].set) {
        break;
      }
      (this->*histogramProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindHistogram::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; histogramProperties[i].name; ++i) {
    if (prop == histogramProperties[i].name) {
      if (!histogramProperties[i].get) {
        break;
      }
      return (this->*histogramProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindHistogram::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindHistogram *imp = dynamic_cast<KstBindHistogram*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*histogramBindings[id - 1].method)(exec, args);
}


void KstBindHistogram::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; histogramBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindHistogram(i + 1));
    obj.put(exec, histogramBindings[i].name, o, KJS::Function);
  }
}


void KstBindHistogram::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindHistogram::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


void KstBindHistogram::setRealTimeAutoBin(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setRealTimeAutoBin(value.toBoolean(exec));
}


KJS::Value KstBindHistogram::realTimeAutoBin(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->realTimeAutoBin());
}


void KstBindHistogram::setBins(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned i = 0;
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setNBins(i);
}


KJS::Value KstBindHistogram::bins(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->nBins());
}


KJS::Value KstBindHistogram::xVector(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->vX();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


KJS::Value KstBindHistogram::yVector(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->vY();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


KJS::Value KstBindHistogram::setVector(KJS::ExecState *exec, const KJS::List& args) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstVectorPtr v = extractVector(exec, args[0]);
  if (!v) {
    return KJS::Undefined();
  }

  KstWriteLocker wl(_d);
  _d->setVector(v);
  return KJS::Undefined();
}


KJS::Value KstBindHistogram::setRange(KJS::ExecState *exec, const KJS::List& args) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 2) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly two arguments.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args[0].type() != KJS::NumberType || args[1].type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  double x0, x1;

  KstWriteLocker wl(_d);
  _d->setXRange(x0, x1);
  return KJS::Undefined();
}


KJS::Value KstBindHistogram::xMin(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->xMin());
}


KJS::Value KstBindHistogram::xMax(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->xMax());
}


// vim: ts=2 sw=2 et
