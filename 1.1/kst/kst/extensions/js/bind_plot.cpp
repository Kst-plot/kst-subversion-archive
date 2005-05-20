/***************************************************************************
                                bind_plot.cpp
                             -------------------
    begin                : Mar 30 2005
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

#include "bind_plot.h"
#include "bind_curvecollection.h"
#include "bind_legend.h"
#include "bind_plotlabelcollection.h"

#include <kst2dplot.h>
#include <ksttoplevelview.h>
#include <kstviewwindow.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindPlot::KstBindPlot(KJS::ExecState *exec, Kst2DPlotPtr d)
: KstBinding("Plot"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPlot::KstBindPlot(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Plot") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Plot", o);
  }
}


KstBindPlot::KstBindPlot(int id)
: KstBinding("Plot Method", id) {
}


KstBindPlot::~KstBindPlot() {
}


KJS::Object KstBindPlot::construct(KJS::ExecState *exec, const KJS::List& args) {
  KstViewWindow *w = 0L;
  if (args.size() == 1) {
    w = extractWindow(exec, args[0]);
    if (!w) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
  }

  if (!w) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  QString n = w->createPlot<Kst2DPlot>(KST::suggestPlotName(), false);
  Kst2DPlotPtr p = *w->view()->findChildrenType<Kst2DPlot>(true).findTag(n);
  if (!p) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Object();
  }

  w->view()->paint(P_PAINT);

  return KJS::Object(new KstBindPlot(exec, p));
}


struct PlotBindings {
  const char *name;
  KJS::Value (KstBindPlot::*method)(KJS::ExecState*, const KJS::List&);
};


struct PlotProperties {
  const char *name;
  void (KstBindPlot::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindPlot::*get)(KJS::ExecState*) const;
};


static PlotBindings plotBindings[] = {
  { 0L, 0L }
};


static PlotProperties plotProperties[] = {
  { "tagName", &KstBindPlot::setTagName, &KstBindPlot::tagName },
  { "curves", 0L, &KstBindPlot::curves },
  { "labels", 0L, &KstBindPlot::labels },
  { "legend", 0L, &KstBindPlot::legend },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPlot::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; plotProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(plotProperties[i].name)));
  }

  return rc;
}


bool KstBindPlot::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; plotProperties[i].name; ++i) {
    if (prop == plotProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindPlot::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; plotProperties[i].name; ++i) {
    if (prop == plotProperties[i].name) {
      if (!plotProperties[i].set) {
        break;
      }
      (this->*plotProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPlot::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; plotProperties[i].name; ++i) {
    if (prop == plotProperties[i].name) {
      if (!plotProperties[i].get) {
        break;
      }
      return (this->*plotProperties[i].get)(exec);
    }
  }
  
  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindPlot::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindPlot *imp = dynamic_cast<KstBindPlot*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*plotBindings[id - 1].method)(exec, args);
}


void KstBindPlot::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; plotBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPlot(i + 1));
    obj.put(exec, plotBindings[i].name, o, KJS::Function);
  }
}


void KstBindPlot::setTagName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return;
  }
  KstWriteLocker wl(_d);
  _d->setTagName(value.toString(exec).qstring());
}


KJS::Value KstBindPlot::tagName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::String(_d->tagName());
}


KJS::Value KstBindPlot::curves(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindCurveCollection(exec, _d));
}


KJS::Value KstBindPlot::labels(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindPlotLabelCollection(exec, _d));
}


KJS::Value KstBindPlot::legend(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindLegend(exec, _d->Legend));
}

// vim: ts=2 sw=2 et
