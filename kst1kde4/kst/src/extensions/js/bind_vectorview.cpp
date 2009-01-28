/***************************************************************************
                                bind_vectorview.cpp
                             -------------------
    begin                : Mar 13 2008
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

#include "bind_scalar.h"
#include "bind_vector.h"
#include "bind_vectorview.h"

#include <kstdataobjectcollection.h>

#include <kdebug.h>

static const QString& IN_FLAGVECTOR = KGlobal::staticQString("IN_FLAGVECTOR");


KstBindVectorView::KstBindVectorView(KJS::ExecState *exec, KstVectorViewPtr v, const char *name)
: KstBindDataObject(exec, v.data(), name ? name : "VectorView") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindVectorView::KstBindVectorView(KJS::ExecState *exec, KJS::Object *globalObject, const char *name)
: KstBindDataObject(exec, globalObject, name ? name : "VectorView") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("VectorView", KstBindVectorView::bindFactory);
  }
}


KstBindDataObject *KstBindVectorView::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  KstVectorViewPtr v = kst_cast<KstVectorView>(obj);
  if (v) {
    return new KstBindVectorView(exec, v);
  }
  return 0L;
}


KstBindVectorView::KstBindVectorView(int id)
: KstBindDataObject(id, "VectorView Method") {
}


KstBindVectorView::~KstBindVectorView() {
  kdDebug() << "Destroy a KstBindVectorView" << endl;
}


KJS::Object KstBindVectorView::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 2) {
    return createSyntaxError(exec);
  }

  KstScalarPtr scalar;
  KstVectorPtr flag;
  KstVectorPtr vX = extractVector(exec, args[0]);
  KstVectorPtr vY = extractVector(exec, args[1]);

  if (!vX) {
    return createTypeError(exec, 0);
  } else if (!vY) {
    return createTypeError(exec, 1);
  }

  KstVectorViewPtr vv = new KstVectorView(QString::null, vX, vY, KstVectorView::InterpY, true, scalar, true, scalar, true, scalar, true, scalar, flag);

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(vv.data());
  KST::dataObjectList.lock().unlock();

  return KJS::Object(new KstBindVectorView(exec, vv));
}


struct VectorViewBindings {
  const char *name;
  KJS::Value (KstBindVectorView::*method)(KJS::ExecState*, const KJS::List&);
};


struct VectorViewProperties {
  const char *name;
  void (KstBindVectorView::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindVectorView::*get)(KJS::ExecState*) const;
};


static VectorViewBindings vectorViewBindings[] = {
  { 0L, 0L }
};


static VectorViewProperties vectorViewProperties[] = {
  { "xVector", &KstBindVectorView::setXVector, &KstBindVectorView::xVector },
  { "yVector", &KstBindVectorView::setYVector, &KstBindVectorView::yVector },
  { "flagVector", &KstBindVectorView::setFlagVector, &KstBindVectorView::flagVector },
  { "xMin", &KstBindVectorView::setXMin, &KstBindVectorView::xMin },
  { "xMax", &KstBindVectorView::setXMax, &KstBindVectorView::xMax },
  { "yMin", &KstBindVectorView::setYMin, &KstBindVectorView::yMin },
  { "yMax", &KstBindVectorView::setYMax, &KstBindVectorView::yMax },
  { "useXMin", &KstBindVectorView::setUseXMin, &KstBindVectorView::useXMin },
  { "useXMax", &KstBindVectorView::setUseXMax, &KstBindVectorView::useXMax },
  { "useYMin", &KstBindVectorView::setUseYMin, &KstBindVectorView::useYMin },
  { "useYMax", &KstBindVectorView::setUseYMax, &KstBindVectorView::useYMax },
  { "interpolateTo", &KstBindVectorView::setInterpolateTo, &KstBindVectorView::interpolateTo },
  { 0L, 0L, 0L }
};


int KstBindVectorView::methodCount() const {
  return sizeof vectorViewBindings + KstBindDataObject::methodCount();
}


int KstBindVectorView::propertyCount() const {
  return sizeof vectorViewProperties + KstBindDataObject::propertyCount();
}


KJS::ReferenceList KstBindVectorView::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindDataObject::propList(exec, recursive);

  for (int i = 0; vectorViewProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(vectorViewProperties[i].name)));
  }

  return rc;
}


bool KstBindVectorView::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; vectorViewProperties[i].name; ++i) {
    if (prop == vectorViewProperties[i].name) {
      return true;
    }
  }

  return KstBindObject::hasProperty(exec, propertyName);
}


KJS::Value KstBindVectorView::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindVectorView *imp = dynamic_cast<KstBindVectorView*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*vectorViewBindings[id - start - 1].method)(exec, args);
  }

  return KstBindDataObject::call(exec, self, args);
}


void KstBindVectorView::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();

  for (int i = 0; vectorViewProperties[i].name; ++i) {
    if (prop == vectorViewProperties[i].name) {
      if (!vectorViewProperties[i].set) {
        break;
      }
      (this->*vectorViewProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindVectorView::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; vectorViewProperties[i].name; ++i) {
    if (prop == vectorViewProperties[i].name) {
      if (!vectorViewProperties[i].get) {
        break;
      }
      return (this->*vectorViewProperties[i].get)(exec);
    }
  }

  return KstBindObject::get(exec, propertyName);
}


#define makeVectorView(X) dynamic_cast<KstVectorView*>(const_cast<KstObject*>(X.data()))

void KstBindVectorView::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindObject::methodCount();
  for (int i = 0; vectorViewBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindVectorView(i + start + 1));
    obj.put(exec, vectorViewBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindVectorView::xVector(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = d->vX();
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setXVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setXVector(v);
      d->setDirty();
      d->setRecursed(false);
      if (d->recursion()) {
        d->setRecursed(true);

        createGeneralError(exec, i18n("Command resulted in a recursion."));
      }
    }
  }
}


KJS::Value KstBindVectorView::yVector(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = d->vY();
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setYVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setYVector(v);
      d->setDirty();
      d->setRecursed(false);
      if (d->recursion()) {
        d->setRecursed(true);

        createGeneralError(exec, i18n("Command resulted in a recursion."));
      }
    }
  }
}


KJS::Value KstBindVectorView::flagVector(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->inputVectors().find(IN_FLAGVECTOR));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setFlagVector(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setFlagVector(v);
      d->setDirty();
    }
  }
}


KJS::Value KstBindVectorView::xMin(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = d->xMinScalar();
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setXMin(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setXminScalar(s);
      d->setDirty();
    }
  }
}


KJS::Value KstBindVectorView::xMax(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = d->xMaxScalar();
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setXMax(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setXmaxScalar(s);
      d->setDirty();
    }
  }
}


KJS::Value KstBindVectorView::yMin(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = d->yMinScalar();
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setYMin(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setYminScalar(s);
      d->setDirty();
    }
  }
}


KJS::Value KstBindVectorView::yMax(KJS::ExecState *exec) const {
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = d->yMaxScalar();
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindVectorView::setYMax(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstVectorViewPtr d = makeVectorView(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setYmaxScalar(s);
      d->setDirty();
    }
  }
}


KJS::Value KstBindVectorView::useXMin(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->useXmin());
  }
  return KJS::Undefined();
}


void KstBindVectorView::setUseXMin(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setUseXmin(value.toBoolean(exec));
    d->setDirty();
  }
}


KJS::Value KstBindVectorView::useXMax(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->useXmax());
  }
  return KJS::Undefined();
}


void KstBindVectorView::setUseXMax(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setUseXmax(value.toBoolean(exec));
    d->setDirty();
  }
}


KJS::Value KstBindVectorView::useYMin(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->useYmin());
  }
  return KJS::Undefined();
}


void KstBindVectorView::setUseYMin(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setUseYmin(value.toBoolean(exec));
    d->setDirty();
  }
}


KJS::Value KstBindVectorView::useYMax(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->useYmax());
  }
  return KJS::Undefined();
}


void KstBindVectorView::setUseYMax(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setUseYmax(value.toBoolean(exec));
    d->setDirty();
  }
}


KJS::Value KstBindVectorView::interpolateTo(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->interp());
  }
  return KJS::Undefined();
}


void KstBindVectorView::setInterpolateTo(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned val;
  if (value.type() != KJS::NumberType || !value.toUInt32(val)) {
    return createPropertyTypeError(exec);
  }

  KstVectorViewPtr d = makeVectorView(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setInterp((KstVectorView::InterpType)val);
    d->setDirty();
  }
}

#undef makeVectorView

