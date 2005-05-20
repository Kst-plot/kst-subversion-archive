/***************************************************************************
                               bind_legend.cpp
                             -------------------
    begin                : Apr 12 2005
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

#include <kst.h>
#include <kst2dplot.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindLegend::KstBindLegend(KJS::ExecState *exec, KstLegendPtr d)
: KstBinding("Legend", false), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindLegend::KstBindLegend(int id)
: KstBinding("Legend Method", id) {
}


KstBindLegend::~KstBindLegend() {
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
  { 0L, 0L }
};


static LegendProperties legendProperties[] = {
  { "enabled", &KstBindLegend::setEnabled, &KstBindLegend::enabled },
  { "front", &KstBindLegend::setFront, &KstBindLegend::front },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindLegend::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

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

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindLegend::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
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

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindLegend::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
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
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindLegend::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindLegend *imp = dynamic_cast<KstBindLegend*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*legendBindings[id - 1].method)(exec, args);
}


void KstBindLegend::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; legendBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindLegend(i + 1));
    obj.put(exec, legendBindings[i].name, o, KJS::Function);
  }
}


void KstBindLegend::setEnabled(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setShow(value.toBoolean(exec));
  KstApp::inst()->paintAll(P_PAINT);
}


KJS::Value KstBindLegend::enabled(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->getShow());
}


void KstBindLegend::setFront(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::BooleanType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setFront(value.toBoolean(exec));
  KstApp::inst()->paintAll(P_PAINT);
}


KJS::Value KstBindLegend::front(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Boolean(_d->getFront());
}


// vim: ts=2 sw=2 et
