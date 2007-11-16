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

#include <kstamatrix.h>
#include <kstdatacollection.h>
#include <kstdataobjectcollection.h>
#include <kstimage.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

KstBindImage::KstBindImage(KJS::ExecState *exec, KstImagePtr d, const char *name)
: KstBindDataObject(exec, d.data(), name ? name : "Image") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindImage::KstBindImage(KJS::ExecState *exec, KJS::Object *globalObject, const char *name)
: KstBindDataObject(exec, globalObject, name ? name : "Image") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (!globalObject) {
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


KstBindImage::KstBindImage(int id, const char *name)
: KstBindDataObject(id, name ? name : "Image Method") {
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
  { 0L, 0L }
};


static ImageProperties imageProperties[] = {
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

#undef makeImage
