/***************************************************************************
                                bind_plotlabel.cpp
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

#include "bind_plotlabel.h"

#include <kst.h>
#include <kstdatacollection.h>
#include <kstplotlabel.h>

#include <kdebug.h>
#include <kjsembed/jsbinding.h>

KstBindPlotLabel::KstBindPlotLabel(KJS::ExecState *exec, Kst2DPlotPtr d)
: QObject(), KstBinding("PlotLabel", false), _d(d.data()) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPlotLabel::KstBindPlotLabel(int id)
: QObject(), KstBinding("PlotLabel Method", id) {
}


KstBindPlotLabel::~KstBindPlotLabel() {
}


struct PlotLabelBindings {
  const char *name;
  KJS::Value (KstBindPlotLabel::*method)(KJS::ExecState*, const KJS::List&);
};


struct PlotLabelProperties {
  const char *name;
  void (KstBindPlotLabel::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindPlotLabel::*get)(KJS::ExecState*) const;
};


static PlotLabelBindings plotLabelBindings[] = {
  { 0L, 0L }
};


static PlotLabelProperties plotLabelProperties[] = {
  { "text", &KstBindPlotLabel::setText, &KstBindPlotLabel::text },
  { "font", &KstBindPlotLabel::setFont, &KstBindPlotLabel::font },
  { "fontSize", &KstBindPlotLabel::setFontSize, &KstBindPlotLabel::fontSize },
  { "justification", &KstBindPlotLabel::setJustification, &KstBindPlotLabel::justification },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPlotLabel::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; plotLabelProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(plotLabelProperties[i].name)));
  }

  return rc;
}


bool KstBindPlotLabel::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; plotLabelProperties[i].name; ++i) {
    if (prop == plotLabelProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindPlotLabel::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  QString prop = propertyName.qstring();
  for (int i = 0; plotLabelProperties[i].name; ++i) {
    if (prop == plotLabelProperties[i].name) {
      if (!plotLabelProperties[i].set) {
        break;
      }
      (this->*plotLabelProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPlotLabel::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; plotLabelProperties[i].name; ++i) {
    if (prop == plotLabelProperties[i].name) {
      if (!plotLabelProperties[i].get) {
        break;
      }
      return (this->*plotLabelProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindPlotLabel::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindPlotLabel *imp = dynamic_cast<KstBindPlotLabel*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*plotLabelBindings[id - 1].method)(exec, args);
}


void KstBindPlotLabel::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; plotLabelBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPlotLabel(i + 1));
    obj.put(exec, plotLabelBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindPlotLabel::text(KJS::ExecState *exec) const {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }
  KstReadLocker rl(_d);
  return KJS::String(_d->topLabel()->text());
}


void KstBindPlotLabel::setText(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return;
  }
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->topLabel()->setText(value.toString(exec).qstring());
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


void KstBindPlotLabel::setFont(KJS::ExecState *exec, const KJS::Value& value) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return;
  }
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->topLabel()->setFontName(value.toString(exec).qstring());
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindPlotLabel::font(KJS::ExecState *exec) const {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::String();
  }

  KstReadLocker rl(_d);
  return KJS::String(_d->topLabel()->fontName());
}


void KstBindPlotLabel::setFontSize(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned int i = 0;
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return;
  }
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->topLabel()->setFontSize(i);
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindPlotLabel::fontSize(KJS::ExecState *exec) const {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Number(0);
  }
  KstReadLocker rl(_d);
  return KJS::Number(_d->topLabel()->fontSize());
}


void KstBindPlotLabel::setJustification(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned int i = 0;
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return;
  }
  if (value.type() != KJS::NumberType || !value.toUInt32(i)) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->topLabel()->setJustification(i);
  _d->setDirty();
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
}


KJS::Value KstBindPlotLabel::justification(KJS::ExecState *exec) const {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Number(0);
  }
  KstReadLocker rl(_d);
  return KJS::Number(_d->topLabel()->justification());
}
