/***************************************************************************
                        bind_plotlabelcollection.cpp
                             -------------------
    begin                : Mar 31 2005
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

#include "bind_plotlabelcollection.h"
#include "bind_label.h"

#include <kdebug.h>

using namespace KJSEmbed;

KstBindPlotLabelCollection::KstBindPlotLabelCollection(KJS::ExecState *exec, Kst2DPlotPtr d)
: KstBinding("PlotLabelCollection", false), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindPlotLabelCollection::KstBindPlotLabelCollection(int id)
: KstBinding("PlotLabelCollection Method", id) {
}


KstBindPlotLabelCollection::~KstBindPlotLabelCollection() {
}


struct PlotLabelCollectionBindings {
  const char *name;
  KJS::Value (KstBindPlotLabelCollection::*method)(KJS::ExecState*, const KJS::List&);
};


struct PlotLabelCollectionProperties {
  const char *name;
  void (KstBindPlotLabelCollection::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindPlotLabelCollection::*get)(KJS::ExecState*) const;
};


static PlotLabelCollectionBindings plotLabelCollectionBindings[] = {
  { 0L, 0L }
};


static PlotLabelCollectionProperties plotLabelCollectionProperties[] = {
  { "length", 0L, &KstBindPlotLabelCollection::length },
  { "x", 0L, &KstBindPlotLabelCollection::x },
  { "y", 0L, &KstBindPlotLabelCollection::y },
  { "top", 0L, &KstBindPlotLabelCollection::top },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindPlotLabelCollection::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; plotLabelCollectionProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(plotLabelCollectionProperties[i].name)));
  }

  return rc;
}


bool KstBindPlotLabelCollection::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; plotLabelCollectionProperties[i].name; ++i) {
    if (prop == plotLabelCollectionProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


KJS::Value KstBindPlotLabelCollection::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstBindPlotLabelCollection *imp = dynamic_cast<KstBindPlotLabelCollection*>(self.imp());
  if (!imp) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  return (imp->*plotLabelCollectionBindings[id - 1].method)(exec, args);
}


void KstBindPlotLabelCollection::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();

  for (int i = 0; plotLabelCollectionProperties[i].name; ++i) {
    if (prop == plotLabelCollectionProperties[i].name) {
      if (!plotLabelCollectionProperties[i].set) {
        break;
      }
      (this->*plotLabelCollectionProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindPlotLabelCollection::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; plotLabelCollectionProperties[i].name; ++i) {
    if (prop == plotLabelCollectionProperties[i].name) {
      if (!plotLabelCollectionProperties[i].get) {
        break;
      }
      return (this->*plotLabelCollectionProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindPlotLabelCollection::getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const {
  if (!_d) {
    return KJS::Undefined();
  }

  KstReadLocker rl(_d);
  if (propertyName < unsigned(_d->_labelList.size())) {
    return KJS::Object(new KstBindLabel(exec, _d->_labelList[propertyName]));
  }

  return KJS::Undefined();
}


void KstBindPlotLabelCollection::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; plotLabelCollectionBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindPlotLabelCollection(i + 1));
    obj.put(exec, plotLabelCollectionBindings[i].name, o, KJS::Function);
  }
}


KJS::Value KstBindPlotLabelCollection::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  KstReadLocker rl(_d);
  return KJS::Number(_d->_labelList.size());
}


KJS::Value KstBindPlotLabelCollection::x(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindLabel(exec, _d->XLabel));
}


KJS::Value KstBindPlotLabelCollection::y(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindLabel(exec, _d->YLabel));
}


KJS::Value KstBindPlotLabelCollection::top(KJS::ExecState *exec) const {
  KstReadLocker rl(_d);
  return KJS::Object(new KstBindLabel(exec, _d->TopLabel));
}


// vim: ts=2 sw=2 et
