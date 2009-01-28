/***************************************************************************
                                bind_axisticklabel.cpp
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

#include "bind_axisticklabel.h"

#include <kst.h>
#include <kstdatacollection.h>
#include <kstplotlabel.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

KstBindAxisTickLabel::KstBindAxisTickLabel(KJS::ExecState *exec, QGuardedPtr<Kst2DPlot> d, bool isX)
: QObject(), KstBinding("AxisTickLabel", false), _d(d), _xAxis(isX) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindAxisTickLabel::KstBindAxisTickLabel(int id)
: QObject(), KstBinding("AxisTickLabel Method", id) {
}


KstBindAxisTickLabel::~KstBindAxisTickLabel() {
}


struct AxisTickLabelBindings {
  const char *name;
  KJS::Value (KstBindAxisTickLabel::*method)(KJS::ExecState*, const KJS::List&);
};


struct AxisTickLabelProperties {
  const char *name;
  void (KstBindAxisTickLabel::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindAxisTickLabel::*get)(KJS::ExecState*) const;
};


static AxisTickLabelBindings axisTickLabelBindings[] = {
  { 0L, 0L }
};


static AxisTickLabelProperties axisTickLabelProperties[] = {
  { "font", &KstBindAxisTickLabel::setFont, &KstBindAxisTickLabel::font },
  { "fontSize", &KstBindAxisTickLabel::setFontSize, &KstBindAxisTickLabel::fontSize },
  { "rotation", &KstBindAxisTickLabel::setRotation, &KstBindAxisTickLabel::rotation },
  { "type", 0L, &KstBindAxisTickLabel::type },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindAxisTickLabel::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; axisTickLabelProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(axisTickLabelProperties[i].name)));
  }

  return rc;
}


bool KstBindAxisTickLabel::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; axisTickLabelProperties[i].name; ++i) {
    if (prop == axisTickLabelProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindAxisTickLabel::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  QString prop = propertyName.qstring();
  for (int i = 0; axisTickLabelProperties[i].name; ++i) {
    if (prop == axisTickLabelProperties[i].name) {
      if (!axisTickLabelProperties[i].set) {
        break;
      }
      (this->*axisTickLabelProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindAxisTickLabel::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; axisTickLabelProperties[i].name; ++i) {
    if (prop == axisTickLabelProperties[i].name) {
      if (!axisTickLabelProperties[i].get) {
        break;
      }
      return (this->*axisTickLabelProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindAxisTickLabel::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  KstBindAxisTickLabel *imp = dynamic_cast<KstBindAxisTickLabel*>(self.imp());
  if (!imp) {
    return createInternalError(exec);
  }

  return (imp->*axisTickLabelBindings[id - 1].method)(exec, args);
}


void KstBindAxisTickLabel::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; axisTickLabelBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindAxisTickLabel(i + 1));
    obj.put(exec, axisTickLabelBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindAxisTickLabel::type(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  return KJS::String(_xAxis ? "X" : "Y");
}


void KstBindAxisTickLabel::setFont(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstWriteLocker wl(_d);
  if (_xAxis) {
    _d->xTickLabel()->setFontName(value.toString(exec).qstring());
    _d->fullTickLabel()->setFontName(value.toString(exec).qstring());
  } else {
    _d->yTickLabel()->setFontName(value.toString(exec).qstring());
    _d->fullTickLabel()->setFontName(value.toString(exec).qstring());
  }
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindAxisTickLabel::font(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstReadLocker rl(_d);
  if (_xAxis) {
    return KJS::String(_d->xTickLabel()->fontName());
  } else {
    return KJS::String(_d->yTickLabel()->fontName());
  }
  return KJS::String();
}


void KstBindAxisTickLabel::setFontSize(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned int i = 0;

  if (!_d) {
    return createPropertyInternalError(exec);
  }
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    return createPropertyTypeError(exec);
  }
  KstWriteLocker wl(_d);
  if (_xAxis) {
    _d->xTickLabel()->setFontSize(i);
    _d->fullTickLabel()->setFontSize(i);
  } else {
    _d->yTickLabel()->setFontSize(i);
    _d->fullTickLabel()->setFontSize(i);
  }
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindAxisTickLabel::fontSize(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }
  KstReadLocker rl(_d);
  if (_xAxis) {
    return KJS::Number(_d->xTickLabel()->fontSize());
  } else {
    return KJS::Number(_d->yTickLabel()->fontSize());
  }
  return KJS::Number(0);
}


void KstBindAxisTickLabel::setRotation(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    return createPropertyInternalError(exec);
  }
  if (value.type() != KJS::NumberType) {
    return createPropertyTypeError(exec);
  }
  KstWriteLocker wl(_d);
  if (_xAxis) {
    _d->xTickLabel()->setRotation(value.toNumber(exec));
  } else {
    _d->yTickLabel()->setRotation(value.toNumber(exec));
  }
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindAxisTickLabel::rotation(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }
  KstReadLocker rl(_d);
  if (_xAxis) {
    return KJS::Number(_d->xTickLabel()->rotation());
  } else {
    return KJS::Number(_d->yTickLabel()->rotation());
  }
  return KJS::Number(0);
}
