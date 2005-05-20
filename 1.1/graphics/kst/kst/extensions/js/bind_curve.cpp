/***************************************************************************
                               bind_curve.cpp
                             -------------------
    begin                : Mar 30 2005
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

#include "bind_curve.h"
#include "bind_point.h"
#include "bind_vector.h"

#include <kstcolorsequence.h>
#include <kstdatacollection.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

using namespace KJSEmbed;

KstBindCurve::KstBindCurve(KJS::ExecState *exec, KstVCurvePtr d)
: KstBinding("Curve"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindCurve::KstBindCurve(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Curve") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Curve", o);
  }
}


KstBindCurve::KstBindCurve(int id)
: KstBinding("Curve Method", id) {
}


KstBindCurve::~KstBindCurve() {
}


KJS::Object KstBindCurve::construct(KJS::ExecState *exec, const KJS::List& args) {
  KstVectorPtr x, y, ex, ey, exm, eym;
  if (args.size() > 0) {
    x = extractVector(exec, args[0]);
    if (!x) {
      return KJS::Object();
    }
  }

  if (args.size() > 1) {
    y = extractVector(exec, args[1]);
    if (!y) {
      return KJS::Object();
    }
  }

  if (args.size() > 2) {
    ex = extractVector(exec, args[2]);
    if (!ex) {
      return KJS::Object();
    }
  }

  if (args.size() > 3) {
    ey = extractVector(exec, args[3]);
    if (!ey) {
      return KJS::Object();
    }
  }

  if (args.size() > 4) {
    exm = extractVector(exec, args[4]);
    if (!exm) {
      return KJS::Object();
    }
  }

  if (args.size() > 5) {
    eym = extractVector(exec, args[5]);
    if (!eym) {
      return KJS::Object();
    }
  }

  if (args.size() > 6) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  QColor color = KstColorSequence::next();
  KstVCurvePtr d = new KstVCurve(QString::null, x, y, ex, ey, exm, eym, color);

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d.data());
  KST::dataObjectList.lock().writeUnlock();

  return KJS::Object(new KstBindCurve(exec, d));
}


struct CurveBindings {
  const char *name;
  KJS::Value (KstBindCurve::*method)(KJS::ExecState*, const KJS::List&);
};


struct CurveProperties {
  const char *name;
  void (KstBindCurve::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindCurve::*get)(KJS::ExecState*) const;
};


static CurveBindings curveBindings[] = {
  { "point", &KstBindCurve::point },
  { "xErrorPoint", &KstBindCurve::xErrorPoint },
  { "yErrorPoint", &KstBindCurve::yErrorPoint },
  { "xMinusErrorPoint", &KstBindCurve::xMinusErrorPoint },
  { "yMinusErrorPoint", &KstBindCurve::yMinusErrorPoint },
  { 0L, 0L }
};


static CurveProperties curveProperties[] = {
  { "tagName", &KstBindCurve::setTagName, &KstBindCurve::tagName },
  { "color", &KstBindCurve::setColor, &KstBindCurve::color },
  { "xVector", &KstBindCurve::setXVector, &KstBindCurve::xVector },
  { "yVector", &KstBindCurve::setYVector, &KstBindCurve::yVector },
  { "xErrorVector", &KstBindCurve::setXErrorVector, &KstBindCurve::xErrorVector },
  { "yErrorVector", &KstBindCurve::setYErrorVector, &KstBindCurve::yErrorVector },
  { "xMinusErrorVector", &KstBindCurve::setXMinusErrorVector, &KstBindCurve::xMinusErrorVector },
  { "yMinusErrorVector", &KstBindCurve::setYMinusErrorVector, &KstBindCurve::yMinusErrorVector },
  { "samplesPerFrame", 0L, &KstBindCurve::samplesPerFrame },
  { "ignoreAutoScale", &KstBindCurve::setIgnoreAutoScale, &KstBindCurve::ignoreAutoScale },
  { "hasPoints", &KstBindCurve::setHasPoints, &KstBindCurve::hasPoints },
  { "hasLines", &KstBindCurve::setHasLines, &KstBindCurve::hasLines },
  { "hasBars", &KstBindCurve::setHasBars, &KstBindCurve::hasBars },
  { "lineWidth", &KstBindCurve::setLineWidth, &KstBindCurve::lineWidth },
  { "lineStyle", &KstBindCurve::setLineStyle, &KstBindCurve::lineStyle },
  { "barStyle", &KstBindCurve::setBarStyle, &KstBindCurve::barStyle },
  { "pointDensity", &KstBindCurve::setPointDensity, &KstBindCurve::pointDensity },
  { "topLabel", 0L, &KstBindCurve::topLabel },
  { "xLabel", 0L, &KstBindCurve::xLabel },
  { "yLabel", 0L, &KstBindCurve::yLabel },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindCurve::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; curveProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(curveProperties[i].name)));
  }

  return rc;
}


bool KstBindCurve::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; curveProperties[i].name; ++i) {
    if (prop == curveProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindCurve::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; curveProperties[i].name; ++i) {
    if (prop == curveProperties[i].name) {
      if (!curveProperties[i].set) {
        break;
      }
      (this->*curveProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindCurve::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; curveProperties[i].name; ++i) {
    if (prop == curveProperties[i].name) {
      if (!curveProperties[i].get) {
        break;
      }
      return (this->*curveProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindCurve::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindCurve *imp = dynamic_cast<KstBindCurve*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*curveBindings[id - 1].method)(exec, args);
}


void KstBindCurve::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; curveBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindCurve(i + 1));
    obj.put(exec, curveBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindCurve::point(KJS::ExecState *exec, const KJS::List& args) {
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

  unsigned i = 0;
  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstReadLocker rl(_d);
  double x, y;
  _d->point(i, x, y);
  return KJS::Object(new KstBindPoint(exec, x, y));
}


KJS::Value KstBindCurve::xErrorPoint(KJS::ExecState *exec, const KJS::List& args) {
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

  unsigned i = 0;
  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstReadLocker rl(_d);
  double x, y, e;
  _d->getEXPoint(i, x, y, e);
  return KJS::Number(e);
}


KJS::Value KstBindCurve::yErrorPoint(KJS::ExecState *exec, const KJS::List& args) {
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

  unsigned i = 0;
  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstReadLocker rl(_d);
  double x, y, e;
  _d->getEYPoint(i, x, y, e);
  return KJS::Number(e);
}


KJS::Value KstBindCurve::xMinusErrorPoint(KJS::ExecState *exec, const KJS::List& args) {
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

  unsigned i = 0;
  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstReadLocker rl(_d);
  double x, y, e;
  _d->getEXMinusPoint(i, x, y, e);
  return KJS::Number(e);
}


KJS::Value KstBindCurve::yMinusErrorPoint(KJS::ExecState *exec, const KJS::List& args) {
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

  unsigned i = 0;
  if (args[0].type() != KJS::NumberType || !args[0].toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstReadLocker rl(_d);
  double x, y, e;
  _d->getEYMinusPoint(i, x, y, e);
  return KJS::Number(e);
}


void KstBindCurve::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindCurve::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


void KstBindCurve::setColor(KJS::ExecState *exec, const KJS::Value& value) {
  QVariant cv = KJSEmbed::convertToVariant(exec, value);
  if (!cv.canCast(QVariant::Color)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker rl(_d);
  _d->setColor(cv.toColor());
}


KJS::Value KstBindCurve::color(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJSEmbed::convertToValue(exec, _d->color());
}


void KstBindCurve::setXVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr vp = extractVector(exec, value);
  if (vp) {
    KstWriteLocker wl(_d);
    _d->setXVector(vp);
  }
}


KJS::Value KstBindCurve::xVector(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->xVector();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


void KstBindCurve::setYVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr vp = extractVector(exec, value);
  if (vp) {
    KstWriteLocker wl(_d);
    _d->setYVector(vp);
  }
}


KJS::Value KstBindCurve::yVector(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->yVector();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


void KstBindCurve::setXErrorVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr vp = extractVector(exec, value);
  if (vp) {
    KstWriteLocker wl(_d);
    _d->setXError(vp);
  }
}


KJS::Value KstBindCurve::xErrorVector(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->xErrorVector();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


void KstBindCurve::setYErrorVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr vp = extractVector(exec, value);
  if (vp) {
    KstWriteLocker wl(_d);
    _d->setYError(vp);
  }
}


KJS::Value KstBindCurve::yErrorVector(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->yErrorVector();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


void KstBindCurve::setXMinusErrorVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr vp = extractVector(exec, value);
  if (vp) {
    KstWriteLocker wl(_d);
    _d->setXMinusError(vp);
  }
}


KJS::Value KstBindCurve::xMinusErrorVector(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->xMinusErrorVector();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


void KstBindCurve::setYMinusErrorVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr vp = extractVector(exec, value);
  if (vp) {
    KstWriteLocker wl(_d);
    _d->setYMinusError(vp);
  }
}


KJS::Value KstBindCurve::yMinusErrorVector(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  KstVectorPtr vp = _d->yMinusErrorVector();
  if (!vp) {
    return KJS::Undefined();
  }

  return KJS::Object(new KstBindVector(exec, vp));
}


KJS::Value KstBindCurve::samplesPerFrame(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->samplesPerFrame());
}


void KstBindCurve::setIgnoreAutoScale(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setIgnoreAutoScale(value.toBoolean(exec));
}


KJS::Value KstBindCurve::ignoreAutoScale(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->ignoreAutoScale());
}


void KstBindCurve::setHasPoints(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setHasPoints(value.toBoolean(exec));
}


KJS::Value KstBindCurve::hasPoints(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->hasPoints());
}


void KstBindCurve::setHasLines(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setHasLines(value.toBoolean(exec));
}


KJS::Value KstBindCurve::hasLines(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->hasLines());
}


void KstBindCurve::setHasBars(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setHasBars(value.toBoolean(exec));
}


KJS::Value KstBindCurve::hasBars(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->hasBars());
}


void KstBindCurve::setLineWidth(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned i = 0;
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setLineWidth(i);
}


KJS::Value KstBindCurve::lineWidth(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->lineWidth());
}


void KstBindCurve::setPointDensity(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned i = 0;
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setPointDensity(i);
}


KJS::Value KstBindCurve::pointDensity(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->pointDensity());
}


void KstBindCurve::setLineStyle(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned i = 0;
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setLineStyle(i);
}


KJS::Value KstBindCurve::lineStyle(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->lineStyle());
}


void KstBindCurve::setBarStyle(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned i = 0;
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setBarStyle(i);
}


KJS::Value KstBindCurve::barStyle(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->barStyle());
}


KJS::Value KstBindCurve::topLabel(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->topLabel());
}


KJS::Value KstBindCurve::xLabel(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->xLabel());
}


KJS::Value KstBindCurve::yLabel(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->yLabel());
}


// vim: ts=2 sw=2 et
