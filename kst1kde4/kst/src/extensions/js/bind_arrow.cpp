/***************************************************************************
                               bind_arrow.cpp
                               ---------------
    begin                : Jun 14 2005
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

#include "bind_arrow.h"

#include <kst.h>
#include <kstviewwindow.h>

#include <kdebug.h>

KstBindArrow::KstBindArrow(KJS::ExecState *exec, KstViewArrowPtr d, const char *name)
: KstBindLine(exec, d.data(), name ? name : "Arrow") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindArrow::KstBindArrow(KJS::ExecState *exec, KJS::Object *globalObject, const char *name)
: KstBindLine(exec, globalObject, name ? name : "Arrow") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindViewObject::addFactory("Arrow", KstBindArrow::bindFactory);
  }
}


KstBindViewObject *KstBindArrow::bindFactory(KJS::ExecState *exec, KstViewObjectPtr obj) {
  KstViewArrowPtr v = kst_cast<KstViewArrow>(obj);
  if (v) {
    return new KstBindArrow(exec, v);
  }
  return 0L;
}


KstBindArrow::KstBindArrow(int id, const char *name)
: KstBindLine(id, name ? name : "Arrow Method") {
}


KstBindArrow::~KstBindArrow() {
}


KJS::Object KstBindArrow::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  KstViewObjectPtr view = extractViewObject(exec, args[0]);
  if (!view) {
    KstViewWindow *w = extractWindow(exec, args[0]);
    if (w) {
      view = w->view();
    } else {
      return createTypeError(exec, 0);
    }
  }

  KstViewArrowPtr b = new KstViewArrow;
  view->appendChild(b.data());
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
  return KJS::Object(new KstBindArrow(exec, b));
}


struct ArrowBindings {
  const char *name;
  KJS::Value (KstBindArrow::*method)(KJS::ExecState*, const KJS::List&);
};


struct ArrowProperties {
  const char *name;
  void (KstBindArrow::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindArrow::*get)(KJS::ExecState*) const;
};


static ArrowBindings arrowBindings[] = {
  { 0L, 0L }
};


static ArrowProperties arrowProperties[] = {
  { "fromArrow", &KstBindArrow::setFromArrow, &KstBindArrow::fromArrow },
  { "toArrow", &KstBindArrow::setToArrow, &KstBindArrow::toArrow },
  { "fromArrowScaling", &KstBindArrow::setFromArrowScaling, &KstBindArrow::fromArrowScaling },
  { "toArrowScaling", &KstBindArrow::setToArrowScaling, &KstBindArrow::toArrowScaling },
  { 0L, 0L, 0L }
};


int KstBindArrow::methodCount() const {
  return sizeof arrowBindings + KstBindLine::methodCount();
}


int KstBindArrow::propertyCount() const {
  return sizeof arrowProperties + KstBindLine::propertyCount();
}


KJS::ReferenceList KstBindArrow::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindLine::propList(exec, recursive);

  for (int i = 0; arrowProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(arrowProperties[i].name)));
  }

  return rc;
}


bool KstBindArrow::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; arrowProperties[i].name; ++i) {
    if (prop == arrowProperties[i].name) {
      return true;
    }
  }

  return KstBindLine::hasProperty(exec, propertyName);
}


void KstBindArrow::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindLine::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; arrowProperties[i].name; ++i) {
    if (prop == arrowProperties[i].name) {
      if (!arrowProperties[i].set) {
        break;
      }
      (this->*arrowProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindLine::put(exec, propertyName, value, attr);
}


KJS::Value KstBindArrow::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindLine::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; arrowProperties[i].name; ++i) {
    if (prop == arrowProperties[i].name) {
      if (!arrowProperties[i].get) {
        break;
      }
      return (this->*arrowProperties[i].get)(exec);
    }
  }

  return KstBindLine::get(exec, propertyName);
}


KJS::Value KstBindArrow::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindLine::methodCount();
  if (id > start) {
    KstBindArrow *imp = dynamic_cast<KstBindArrow*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*arrowBindings[id - start - 1].method)(exec, args);
  } 

  return KstBindLine::call(exec, self, args);
}


void KstBindArrow::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindLine::methodCount();
  for (int i = 0; arrowBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindArrow(i + start + 1));
    obj.put(exec, arrowBindings[i].name, o, KJS::Function);
  }
}

#define makeArrow(X) dynamic_cast<KstViewArrow*>(const_cast<KstObject*>(X.data()))

void KstBindArrow::setFromArrow(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }

  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstWriteLocker wl(d);

    d->setHasFromArrow(value.toBoolean(exec));
    _d->setDirty();
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindArrow::fromArrow(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->hasFromArrow());
  }

  return KJS::Undefined();
}


void KstBindArrow::setToArrow(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }

  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstWriteLocker wl(d);

    d->setHasToArrow(value.toBoolean(exec));
    _d->setDirty();
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindArrow::toArrow(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->hasToArrow());
  }

  return KJS::Undefined();
}


void KstBindArrow::setFromArrowScaling(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }

  if (value.type() !=  KJS::NumberType) {
    return createPropertyTypeError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstWriteLocker wl(d);

    d->setFromArrowScaling(value.toNumber(exec));
    _d->setDirty();
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindArrow::fromArrowScaling(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->fromArrowScaling());
  }

  return KJS::Undefined();
}


void KstBindArrow::setToArrowScaling(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }

  if (value.type() !=  KJS::NumberType) {
    return createPropertyTypeError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstWriteLocker wl(d);

    d->setToArrowScaling(value.toNumber(exec));
    _d->setDirty();
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindArrow::toArrowScaling(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstViewArrowPtr d = makeArrow(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->toArrowScaling());
  }

  return KJS::Undefined();
}

#undef makeArrow
