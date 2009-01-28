/***************************************************************************
                               bind_legend.cpp
                               ---------------
    begin                : Nov 08 2005
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

#include "bind_legend.h"
#include "bind_curvecollection.h"

#include <kst.h>
#include <kstbasecurve.h>
#include <kstviewwindow.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

KstBindLegend::KstBindLegend(KJS::ExecState *exec, KstViewLegendPtr d, const char *name)
: KstBindBorderedViewObject(exec, d.data(), name ? name : "Legend") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindLegend::KstBindLegend(KJS::ExecState *exec, KJS::Object *globalObject, const char *name)
: KstBindBorderedViewObject(exec, globalObject, name ? name : "Legend") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindBorderedViewObject::addFactory("Legend", KstBindLegend::bindFactory);
  }
}


KstBindViewObject *KstBindLegend::bindFactory(KJS::ExecState *exec, KstViewObjectPtr obj) {
  KstViewLegendPtr v = kst_cast<KstViewLegend>(obj);
  if (v) {
    return new KstBindLegend(exec, v);
  }
  return 0L;
}


KstBindLegend::KstBindLegend(int id, const char *name)
: KstBindBorderedViewObject(id, name ? name : "Legend Method") {
}


KstBindLegend::~KstBindLegend() {
}


KJS::Object KstBindLegend::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() == 0 || args.size() > 2) {
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

  QString txt;
  if (args.size() == 2) {
    if (args[1].type() != KJS::StringType) {
      return createTypeError(exec, 1);
    }
    txt = args[1].toString(exec).qstring();
  }

  KstViewLegendPtr b = new KstViewLegend;
  view->appendChild(b.data());
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
  return KJS::Object(new KstBindLegend(exec, b));
}


struct LegendBindings {
  const char *name;
  KJS::Value (KstBindLegend::*method)(KJS::ExecState*, const KJS::List&);
};


struct LegendProperties {
  const char *name;
  void (KstBindLegend::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindLegend::*get)(KJS::ExecState*) const;
};


static LegendBindings legendBindings[] = {
  { "addCurve", &KstBindLegend::addCurve },
  { "removeCurve", &KstBindLegend::removeCurve },
  { 0L, 0L }
};


static LegendProperties legendProperties[] = {
  { "font", &KstBindLegend::setFont, &KstBindLegend::font },
  { "fontSize", &KstBindLegend::setFontSize, &KstBindLegend::fontSize },
  { "textColor", &KstBindLegend::setTextColor, &KstBindLegend::textColor },
  { "vertical", &KstBindLegend::setVertical, &KstBindLegend::vertical },
  { "curves", 0L, &KstBindLegend::curves },
  { "title", &KstBindLegend::setTitle, &KstBindLegend::title },
  { 0L, 0L, 0L }
};


int KstBindLegend::methodCount() const {
  return sizeof legendBindings + KstBindBorderedViewObject::methodCount();
}


int KstBindLegend::propertyCount() const {
  return sizeof legendProperties + KstBindBorderedViewObject::propertyCount();
}


KJS::ReferenceList KstBindLegend::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindBorderedViewObject::propList(exec, recursive);

  for (int i = 0; legendProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(legendProperties[i].name)));
  }

  return rc;
}


bool KstBindLegend::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; legendProperties[i].name; ++i) {
    if (prop == legendProperties[i].name) {
      return true;
    }
  }

  return KstBindBorderedViewObject::hasProperty(exec, propertyName);
}


void KstBindLegend::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindBorderedViewObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; legendProperties[i].name; ++i) {
    if (prop == legendProperties[i].name) {
      if (!legendProperties[i].set) {
        break;
      }
      (this->*legendProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindBorderedViewObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindLegend::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindBorderedViewObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; legendProperties[i].name; ++i) {
    if (prop == legendProperties[i].name) {
      if (!legendProperties[i].get) {
        break;
      }
      return (this->*legendProperties[i].get)(exec);
    }
  }

  return KstBindBorderedViewObject::get(exec, propertyName);
}


KJS::Value KstBindLegend::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  int start = KstBindBorderedViewObject::methodCount();
  if (id > start) {
    KstBindLegend *imp = dynamic_cast<KstBindLegend*>(self.imp());
    if (!imp) {
      return createInternalError(exec);
    }

    return (imp->*legendBindings[id - start - 1].method)(exec, args);
  } 

  return KstBindBorderedViewObject::call(exec, self, args);
}


void KstBindLegend::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindBorderedViewObject::methodCount();
  for (int i = 0; legendBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindLegend(i + start + 1));
    obj.put(exec, legendBindings[i].name, o, KJS::Function);
  }
}


#define makeLegend(X) dynamic_cast<KstViewLegend*>(const_cast<KstObject*>(X.data()))

KJS::Value KstBindLegend::addCurve(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  KstBaseCurvePtr curve;
  curve = extractVCurve(exec, args[0], false);
  if (curve) {
    KstViewLegendPtr d = makeLegend(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->addCurve(curve);
      KstApp::inst()->paintAll(KstPainter::P_PAINT);
    }
  } else {
    return createTypeError(exec, 0);
  }

  return KJS::Undefined();
}


KJS::Value KstBindLegend::removeCurve(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    return createSyntaxError(exec);
  }

  KstBaseCurvePtr curve;
  curve = extractVCurve(exec, args[0], false);
  if (curve) {
    KstViewLegendPtr d = makeLegend(_d);
    if (d) {
      KstWriteLocker wl(d);
      d->removeCurve(curve);
      KstApp::inst()->paintAll(KstPainter::P_PAINT);
    }
  } else {
    return createTypeError(exec, 0);
  }

  return KJS::Undefined();
}


void KstBindLegend::setFont(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setFontName(value.toString(exec).qstring());
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindLegend::font(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->fontName());
  }
  return KJS::String();
}


void KstBindLegend::setFontSize(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned int i = 0;
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    return createPropertyTypeError(exec);
  }
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setFontSize(i);
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindLegend::fontSize(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Number(d->fontSize());
  }
  return KJS::Number(0);
}


void KstBindLegend::setTextColor(KJS::ExecState *exec, const KJS::Value& value) {
  QVariant cv = KJSEmbed::convertToVariant(exec, value);
  if (!cv.canCast(QVariant::Color)) {
    return createPropertyTypeError(exec);
  }
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstWriteLocker rl(d);
    d->setForegroundColor(cv.toColor());
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindLegend::textColor(KJS::ExecState *exec) const {
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJSEmbed::convertToValue(exec, d->foregroundColor());
  }
  return KJSEmbed::convertToValue(exec, QColor());
}


void KstBindLegend::setVertical(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    return createPropertyTypeError(exec);
  }
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setVertical(value.toBoolean(exec));
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindLegend::vertical(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Boolean(d->vertical());
  }
  return KJS::Boolean(false);
}


KJS::Value KstBindLegend::curves(KJS::ExecState *exec) const {
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::Object(new KstBindCurveCollection(exec, d));
  }
  return KJS::Null();
}


void KstBindLegend::setTitle(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstWriteLocker wl(d);
    d->setTitle(value.toString(exec).qstring());
    KstApp::inst()->paintAll(KstPainter::P_PAINT);
  }
}


KJS::Value KstBindLegend::title(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstViewLegendPtr d = makeLegend(_d);
  if (d) {
    KstReadLocker rl(d);
    return KJS::String(d->title());
  }
  return KJS::Undefined();
}

#undef makeLegend
