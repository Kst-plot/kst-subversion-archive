/***************************************************************************
                                bind_axislabel.cpp
                             -------------------
    begin                : Jul 25 2007
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

#include "bind_axislabel.h"

#include <kst.h>
#include <kstdatacollection.h>
#include <kstplotlabel.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

KstBindAxisLabel::KstBindAxisLabel(KJS::ExecState *exec, QGuardedPtr<Kst2DPlot> d, bool isX)
: QObject(), KstBinding("AxisLabel", false), _d(d), _xAxis(isX) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindAxisLabel::KstBindAxisLabel(int id)
: QObject(), KstBinding("AxisLabel Method", id) {
}


KstBindAxisLabel::~KstBindAxisLabel() {
}


struct AxisLabelBindings {
  const char *name;
  KJS::Value (KstBindAxisLabel::*method)(KJS::ExecState*, const KJS::List&);
};


struct AxisLabelProperties {
  const char *name;
  void (KstBindAxisLabel::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindAxisLabel::*get)(KJS::ExecState*) const;
};


static AxisLabelBindings axisLabelBindings[] = {
  { 0L, 0L }
};


static AxisLabelProperties axisLabelProperties[] = {
  { "text", &KstBindAxisLabel::setText, &KstBindAxisLabel::text },
  { "font", &KstBindAxisLabel::setFont, &KstBindAxisLabel::font },
  { "fontSize", &KstBindAxisLabel::setFontSize, &KstBindAxisLabel::fontSize },
  { "type", 0L, &KstBindAxisLabel::type },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindAxisLabel::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; axisLabelProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(axisLabelProperties[i].name)));
  }

  return rc;
}


bool KstBindAxisLabel::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; axisLabelProperties[i].name; ++i) {
    if (prop == axisLabelProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindAxisLabel::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  QString prop = propertyName.qstring();
  for (int i = 0; axisLabelProperties[i].name; ++i) {
    if (prop == axisLabelProperties[i].name) {
      if (!axisLabelProperties[i].set) {
        break;
      }
      (this->*axisLabelProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindAxisLabel::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; axisLabelProperties[i].name; ++i) {
    if (prop == axisLabelProperties[i].name) {
      if (!axisLabelProperties[i].get) {
        break;
      }
      return (this->*axisLabelProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindAxisLabel::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  KstBindAxisLabel *imp = dynamic_cast<KstBindAxisLabel*>(self.imp());
  if (!imp) {
    return createInternalError(exec);
  }

  return (imp->*axisLabelBindings[id - 1].method)(exec, args);
}


void KstBindAxisLabel::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; axisLabelBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindAxisLabel(i + 1));
    obj.put(exec, axisLabelBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindAxisLabel::type(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  return KJS::String(_xAxis ? "X" : "Y");
}


KJS::Value KstBindAxisLabel::text(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }
  KstReadLocker rl(_d);
  if (_xAxis) {
    return KJS::String(_d->xLabel()->text());
  } else {
    return KJS::String(_d->yLabel()->text());
  }
}


void KstBindAxisLabel::setText(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstWriteLocker wl(_d);
  if (_xAxis) {
    _d->xLabel()->setText(value.toString(exec).qstring());
  } else {
    _d->yLabel()->setText(value.toString(exec).qstring());
  }
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


void KstBindAxisLabel::setFont(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstWriteLocker wl(_d);
  if (_xAxis) {
    _d->xLabel()->setFontName(value.toString(exec).qstring());
  } else {
    _d->yLabel()->setFontName(value.toString(exec).qstring());
  }
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindAxisLabel::font(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstReadLocker rl(_d);
  if (_xAxis) {
    return KJS::String(_d->xLabel()->fontName());
  } else {
    return KJS::String(_d->yLabel()->fontName());
  }
  return KJS::String();
}


void KstBindAxisLabel::setFontSize(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned int i = 0;

  if (!_d) {
    return createPropertyInternalError(exec);
  }
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    return createPropertyTypeError(exec);
  }
  KstWriteLocker wl(_d);
  if (_xAxis) {
    _d->xLabel()->setFontSize(i);
  } else {
    _d->yLabel()->setFontSize(i);
  }
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindAxisLabel::fontSize(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }
  KstReadLocker rl(_d);
  if (_xAxis) {
    return KJS::Number(_d->xLabel()->fontSize());
  } else {
    return KJS::Number(_d->yLabel()->fontSize());
  }
  return KJS::Number(0);
}

