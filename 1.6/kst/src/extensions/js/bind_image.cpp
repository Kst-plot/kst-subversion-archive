/***************************************************************************
                               bind_image.cpp
                             -------------------
    begin                : Nov 15 2007
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

#include "bind_image.h"
#include "bind_matrix.h"

#include <kstamatrix.h>
#include <kstdatacollection.h>
#include <kstdataobjectcollection.h>
#include <kstimage.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

KstBindImage::KstBindImage(KJS::ExecState *exec, KstImagePtr d)
: KstBindDataObject(exec, d.data(), "Image") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindImage::KstBindImage(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBindDataObject(exec, globalObject, "Image") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindDataObject::addFactory("Image", KstBindImage::bindFactory);
  }
}


KstBindDataObject *KstBindImage::bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj) {
  KstImagePtr i = kst_cast<KstImage>(obj);
  if (i) {
    return new KstBindImage(exec, i);
  }
  return 0L;
}


KstBindImage::KstBindImage(int id)
: KstBindDataObject(id, "Image Method") {
}


KstBindImage::~KstBindImage() {
}


KJS::Object KstBindImage::construct(KJS::ExecState *exec, const KJS::List& args) {
  KstMatrixPtr matrix;

  if (args.size() > 0) {
    matrix = extractMatrix(exec, args[0]);
    if (!matrix) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
  }

  KstImagePtr image = new KstImage(QString::null, matrix, 10, QColor(0,0,0), 1);

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(image.data());
  KST::dataObjectList.lock().unlock();

  return KJS::Object(new KstBindImage(exec, image));
}


struct ImageBindings {
  const char *name;
  KJS::Value (KstBindImage::*method)(KJS::ExecState*, const KJS::List&);
};


struct ImageProperties {
  const char *name;
  void (KstBindImage::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindImage::*get)(KJS::ExecState*) const;
};


static ImageBindings imageBindings[] = {
  { "minMaxThreshold", &KstBindImage::minMaxThreshold },
  { "smartThreshold", &KstBindImage::smartThreshold },
  { 0L, 0L }
};


static ImageProperties imageProperties[] = {
  { "matrix", &KstBindImage::setMatrix, &KstBindImage::matrix },
  { "map", &KstBindImage::setMap, &KstBindImage::map },
  { "palette", &KstBindImage::setPalette, &KstBindImage::palette },
  { "lowerThreshold", &KstBindImage::setLowerThreshold, &KstBindImage::lowerThreshold },
  { "upperThreshold", &KstBindImage::setUpperThreshold, &KstBindImage::upperThreshold },
  { "autoThreshold", &KstBindImage::setAutoThreshold, &KstBindImage::autoThreshold },
  { "numContours", &KstBindImage::setNumContours, &KstBindImage::numContours },
  { "contourWeight", &KstBindImage::setContourWeight, &KstBindImage::contourWeight },
  { "contourColor", &KstBindImage::setContourColor, &KstBindImage::contourColor },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindImage::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindObject::propList(exec, recursive);

  for (int i = 0; imageProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(imageProperties[i].name)));
  }

  return rc;
}


bool KstBindImage::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; imageProperties[i].name; ++i) {
    if (prop == imageProperties[i].name) {
      return true;
    }
  }

  return KstBindDataObject::hasProperty(exec, propertyName);
}


void KstBindImage::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindDataObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; imageProperties[i].name; ++i) {
    if (prop == imageProperties[i].name) {
      if (!imageProperties[i].set) {
        break;
      }
      (this->*imageProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindDataObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindImage::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; imageProperties[i].name; ++i) {
    if (prop == imageProperties[i].name) {
      if (!imageProperties[i].get) {
        break;
      }
      return (this->*imageProperties[i].get)(exec);
    }
  }

  return KstBindDataObject::get(exec, propertyName);
}


KJS::Value KstBindImage::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindImage *imp = dynamic_cast<KstBindImage*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*imageBindings[id - 1].method)(exec, args);
}


void KstBindImage::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; imageBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindImage(i + 1));
    obj.put(exec, imageBindings[i].name, o, KJS::Function);
  }
}

#define makeImage(X) dynamic_cast<KstImage*>(const_cast<KstObject*>(X.data()))

int KstBindImage::methodCount() const {
  return sizeof imageBindings + KstBindObject::methodCount();
}


int KstBindImage::propertyCount() const {
  return sizeof imageProperties + KstBindObject::propertyCount();
}


KJS::Value KstBindImage::minMaxThreshold(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)

  KstImagePtr d = makeImage(_d);
  if (!d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires no arguments.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstReadLocker rl(d);

  d->setThresholdToMinMax();
  return KJS::Undefined();
}


KJS::Value KstBindImage::smartThreshold(KJS::ExecState *exec, const KJS::List& args) {
  KstImagePtr d = makeImage(_d);
  if (!d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args[0].type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  double per = args[0].toNumber(exec);
  KstReadLocker rl(d);

  d->setThresholdToSpikeInsensitive(per);
  return KJS::Undefined();
}


void KstBindImage::setMatrix(KJS::ExecState *exec, const KJS::Value& value) {
  KstMatrixPtr mp = extractMatrix(exec, value);
  if (mp) {
    KstImagePtr d = makeImage(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->setMatrix(mp);
    }
  }
}


KJS::Value KstBindImage::matrix(KJS::ExecState *exec) const {
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    KstMatrixPtr mp = d->matrix();
    if (mp) {
      return KJS::Object(new KstBindMatrix(exec, mp));
    }
  }
  return KJS::Null();
}


void KstBindImage::setMap(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned i = 0;

  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }

  if (i > 2) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Value is out of range.");
    exec->setException(eobj);
    return;
  }

  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    switch (i) {
      case 0:
        d->setHasContourMap(false);
        d->setHasColorMap(true);
        break;
      case 1:
        d->setHasContourMap(true);
        d->setHasColorMap(false);
        break;
      case 2:
        d->setHasContourMap(true);
        d->setHasColorMap(true);
        break;
    }
  }
}


KJS::Value KstBindImage::map(KJS::ExecState *exec) const {
  Q_UNUSED(exec)

  int val = 0;
  KstImagePtr d = makeImage(_d);
  if (d) {
    if (d->hasContourMap() && d->hasColorMap()) {
      val = 2;
    } else if (d->hasContourMap()) {
      val = 1;
    } else if (d->hasColorMap()) {
      val = 0;
    }
  }

  return KJS::Number(val);
}


void KstBindImage::setPalette(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  QString pal = value.toString(exec).qstring();
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setPalette(pal);
  }
}


KJS::Value KstBindImage::palette(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->paletteName());
  }
  return KJS::Undefined();
}


void KstBindImage::setLowerThreshold(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  double val = value.toNumber(exec);
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setLowerThreshold(val);
  }
}


KJS::Value KstBindImage::lowerThreshold(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->lowerThreshold());
  }
  return KJS::Number(0.0);
}


void KstBindImage::setUpperThreshold(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  double val = value.toNumber(exec);
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setUpperThreshold(val);
  }
}


KJS::Value KstBindImage::upperThreshold(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->upperThreshold());
  }
  return KJS::Number(0.0);
}


void KstBindImage::setAutoThreshold(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  bool val = value.toBoolean(exec);
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setAutoThreshold(val);
  }
}


KJS::Value KstBindImage::autoThreshold(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->upperThreshold());
  }
  return KJS::Boolean(false);
}


void KstBindImage::setNumContours(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }

  int val = value.toInt32(exec);
  if (val <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Value is out of range.");
    exec->setException(eobj);
    return;
  }

  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setNumContourLines(val);
  }
}


KJS::Value KstBindImage::numContours(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->numContourLines());
  }
  return KJS::Number(0);
}


void KstBindImage::setContourWeight(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::NumberType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  int val = value.toInt32(exec);
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setContourWeight(val);
  }
}


KJS::Value KstBindImage::contourWeight(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->contourWeight());
  }
  return KJS::Number(0);
}


void KstBindImage::setContourColor(KJS::ExecState *exec, const KJS::Value& value) {
  QVariant cv = KJSEmbed::convertToVariant(exec, value);
  if (!cv.canCast(QVariant::Color)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstWriteLocker rl(d);
    d->setContourColor(cv.toColor());
  }
}


KJS::Value KstBindImage::contourColor(KJS::ExecState *exec) const {
  KstImagePtr d = makeImage(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJSEmbed::convertToValue(exec, d->contourColor());
  }

  return KJSEmbed::convertToValue(exec, QColor());
}

#undef makeImage
