/***************************************************************************
                             bind_binnedmap.cpp
                             -------------------
    begin                : Feb 18 2008
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

#include "bind_binnedmap.h"
#include "bind_matrix.h"
#include "bind_scalar.h"
#include "bind_vector.h"

#include <kstdatacollection.h>
#include <kstdataobjectcollection.h>
#include <kstobject.h>
#include <kstobjectcollection.h>
#include <kdebug.h>
#include <qobject.h>

static const QString& VECTOR_X = KGlobal::staticQString("Vector X");
static const QString& VECTOR_Y = KGlobal::staticQString("Vector Y");
static const QString& VECTOR_Z = KGlobal::staticQString("Vector Z");
static const QString& MAP = KGlobal::staticQString("Binned Map");
static const QString& HITSMAP = KGlobal::staticQString("Hits Map");
static const QString& XMIN = KGlobal::staticQString("xMin");
static const QString& XMAX = KGlobal::staticQString("xMax");
static const QString& YMIN = KGlobal::staticQString("yMin");
static const QString& YMAX = KGlobal::staticQString("yMax");
static const QString& NX = KGlobal::staticQString("nX");
static const QString& NY = KGlobal::staticQString("nY");
static const QString& AUTOBIN = KGlobal::staticQString("autobin");

KstBindBinnedMap::KstBindBinnedMap(KJS::ExecState *exec, KstDataObjectPtr d)
: KstBindDataObject(exec, d.data(), "BinnedMap") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindBinnedMap::KstBindBinnedMap(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindDataObject(exec, globalObject, "BinnedMap") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("BinnedMap", KstBindBinnedMap::bindFactory);
  }
}


KstBindDataObject *KstBindBinnedMap::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  if (obj) {
    return new KstBindBinnedMap(exec, obj);
  }

  return 0L;
}


KstBindBinnedMap::KstBindBinnedMap(int id, const char *name)
: KstBindDataObject(id, name ? name : "BinnedMap Method") {
}


KstBindBinnedMap::~KstBindBinnedMap() {
}


KJS::Object KstBindBinnedMap::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstDataObjectPtr d = KstDataObject::createPlugin(QString("Binned Map"));
  return KJS::Object(new KstBindBinnedMap(exec, d));
}


struct BinnedMapBindings {
  const char *name;
  KJS::Value (KstBindBinnedMap::*method)(KJS::ExecState*, const KJS::List&);
};


struct BinnedMapProperties {
  const char *name;
  void (KstBindBinnedMap::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindBinnedMap::*get)(KJS::ExecState*) const;
};


static BinnedMapBindings binnedMapBindings[] = {
  { "validate", &KstBindBinnedMap::validate },
  { 0L, 0L }
};


static BinnedMapProperties binnedMapProperties[] = {
  { "x", &KstBindBinnedMap::setX, &KstBindBinnedMap::x },
  { "y", &KstBindBinnedMap::setY, &KstBindBinnedMap::y },
  { "z", &KstBindBinnedMap::setZ, &KstBindBinnedMap::z },
  { "binnedMap", &KstBindBinnedMap::setBinnedMap, &KstBindBinnedMap::binnedMap },
  { "hitsMap", &KstBindBinnedMap::setHitsMap, &KstBindBinnedMap::hitsMap },
  { "xFrom", &KstBindBinnedMap::setXFrom, &KstBindBinnedMap::xFrom },
  { "xTo", &KstBindBinnedMap::setXTo, &KstBindBinnedMap::xTo },
  { "yFrom", &KstBindBinnedMap::setYFrom, &KstBindBinnedMap::yFrom },
  { "yTo", &KstBindBinnedMap::setYTo, &KstBindBinnedMap::yTo },
  { "nX", &KstBindBinnedMap::setNX, &KstBindBinnedMap::nX },
  { "nY", &KstBindBinnedMap::setNY, &KstBindBinnedMap::nY },
  { "autobin", &KstBindBinnedMap::setAutobin, &KstBindBinnedMap::autobin },
  { "valid", 0L, &KstBindBinnedMap::valid },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindBinnedMap::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindDataObject::propList(exec, recursive);

  for (int i = 0; binnedMapProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(binnedMapProperties[i].name)));
  }

  return rc;
}


bool KstBindBinnedMap::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; binnedMapProperties[i].name; ++i) {
    if (prop == binnedMapProperties[i].name) {
      return true;
    }
  }

  return KstBindDataObject::hasProperty(exec, propertyName);
}


void KstBindBinnedMap::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindDataObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; binnedMapProperties[i].name; ++i) {
    if (prop == binnedMapProperties[i].name) {
      if (!binnedMapProperties[i].set) {
        break;
      }
      (this->*binnedMapProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindDataObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindBinnedMap::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindDataObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; binnedMapProperties[i].name; ++i) {
    if (prop == binnedMapProperties[i].name) {
      if (!binnedMapProperties[i].get) {
        break;
      }
      return (this->*binnedMapProperties[i].get)(exec);
    }
  }

  return KstBindDataObject::get(exec, propertyName);
}


KJS::Value KstBindBinnedMap::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindDataObject::methodCount();
  if (id > start) {
    KstBindBinnedMap *imp = dynamic_cast<KstBindBinnedMap*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*binnedMapBindings[id - start - 1].method)(exec, args);
  }

  return KstBindDataObject::call(exec, self, args);
}


void KstBindBinnedMap::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindDataObject::methodCount();
  for (int i = 0; binnedMapBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindBinnedMap(i + start + 1));
    obj.put(exec, binnedMapBindings[i].name, o, KJS::Function);
  }
}

#define makeDataObject(X) dynamic_cast<KstDataObject*>(const_cast<KstObject*>(X.data()))

KJS::Value KstBindBinnedMap::validate(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 0) {
    return createSyntaxError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    if (d->isValid() && 
        *(d->outputMatrices().find(MAP)) &&
        *(d->outputMatrices().find(HITSMAP)) ) {

      KST::dataObjectList.lock().writeLock();
      KST::dataObjectList.append(d.data());
      KST::dataObjectList.lock().unlock();

      return KJS::Boolean(true);
    }
  }

  return KJS::Boolean(false);
}


void KstBindBinnedMap::setX(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (v) {
        d->inputVectors()[VECTOR_X] = v;
      } else {
        d->inputVectors().remove(VECTOR_X);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::x(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->inputVectors().find(VECTOR_X));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setY(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (v) {
        d->inputVectors()[VECTOR_Y] = v;
      } else {
        d->inputVectors().remove(VECTOR_Y);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::y(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->inputVectors().find(VECTOR_Y));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setZ(KJS::ExecState *exec, const KJS::Value& value) {
  KstVectorPtr v = extractVector(exec, value);
  if (v) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (v) {
        d->inputVectors()[VECTOR_Z] = v;
      } else {
        d->inputVectors().remove(VECTOR_Z);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::z(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstVectorPtr v = *(d->inputVectors().find(VECTOR_Z));
    if (v) {
      return KJS::Object(new KstBindVector(exec, v));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setBinnedMap(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstWriteLocker rl(d);
    QString name = value.toString(exec).qstring();
    QString tname;

    if (name.isEmpty()) {
      tname = i18n("map");
    } else {
      tname = name;
    }

    KstWriteLocker blockMatrixUpdates(&KST::matrixList.lock());
    KstMatrixPtr m = new KstMatrix(KstObjectTag(tname, d->tag()), d );
    d->outputMatrices().insert(MAP, m);
  }
}


KJS::Value KstBindBinnedMap::binnedMap(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstMatrixPtr m = *(d->outputMatrices().find(MAP));
    if (m) {
      return KJS::Object(new KstBindMatrix(exec, m));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setHitsMap(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstWriteLocker rl(d);
    QString name = value.toString(exec).qstring();
    QString tname;

    if (name.isEmpty()) {
      tname = i18n("hits map");
    } else {
      tname = name;
    }
    KstWriteLocker blockMatrixUpdates(&KST::matrixList.lock());
    KstMatrixPtr m = new KstMatrix(KstObjectTag(tname, d->tag()), d.data());
    d->outputMatrices().insert(HITSMAP, m);
  }
}


KJS::Value KstBindBinnedMap::hitsMap(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstMatrixPtr m = *(d->outputMatrices().find(HITSMAP));
    if (m) {
      return KJS::Object(new KstBindMatrix(exec, m));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setXFrom(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[XMIN] = s;
      } else {
        d->inputScalars().remove(XMIN);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::xFrom(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(XMIN));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setXTo(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[XMAX] = s;
      } else {
        d->inputScalars().remove(XMAX);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::xTo(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(XMAX));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setYFrom(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[YMIN] = s;
      } else {
        d->inputScalars().remove(YMIN);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::yFrom(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(YMIN));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setYTo(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[YMAX] = s;
      } else {
        d->inputScalars().remove(YMAX);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::yTo(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(YMAX));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setNX(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[NX] = s;
      } else {
        d->inputScalars().remove(NX);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::nX(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(NX));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setNY(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[NY] = s;
      } else {
        d->inputScalars().remove(NY);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::nY(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(NY));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


void KstBindBinnedMap::setAutobin(KJS::ExecState *exec, const KJS::Value& value) {
  KstScalarPtr s = extractScalar(exec, value);
  if (s) {
    KstDataObjectPtr d = makeDataObject(_d);
    if (d) {
      KstWriteLocker rl(d);
      if (s) {
        d->inputScalars()[AUTOBIN] = s;
      } else {
        d->inputScalars().remove(AUTOBIN);
      }
      d->setDirty();
    }
  }
}


KJS::Value KstBindBinnedMap::autobin(KJS::ExecState *exec) const {
  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    KstScalarPtr s = *(d->inputScalars().find(AUTOBIN));
    if (s) {
      return KJS::Object(new KstBindScalar(exec, s));
    }
  }
  return KJS::Object();
}


KJS::Value KstBindBinnedMap::valid(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  KstDataObjectPtr d = makeDataObject(_d);
  if (d) {
    KstReadLocker rl(d);
    if (d->isValid() && 
        *(d->outputMatrices().find(MAP)) &&
        *(d->outputMatrices().find(HITSMAP)) ) {
      return KJS::Boolean(true);
    }
  }
  return KJS::Boolean(false);
}

#undef makeDataObject

