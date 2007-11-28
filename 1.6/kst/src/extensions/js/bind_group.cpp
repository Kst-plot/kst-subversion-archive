/***************************************************************************
                               bind_group.cpp
                               ---------------
    begin                : Nov 23 2007
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

#include "bind_group.h"

#include <kst.h>
#include <kstviewwindow.h>

#include <kdebug.h>

KstBindGroup::KstBindGroup(KJS::ExecState *exec, KstPlotGroupPtr d, const char *name)
: KstBindViewObject(exec, d.data(), name ? name : "Group") {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindGroup::KstBindGroup(KJS::ExecState *exec, KJS::Object *globalObject, const char *name)
: KstBindViewObject(exec, globalObject, name ? name : "Group") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    KstBindViewObject::addFactory("Group", KstBindGroup::bindFactory);
  }
}


KstBindViewObject *KstBindGroup::bindFactory(KJS::ExecState *exec, KstViewObjectPtr obj) {
  KstPlotGroupPtr g = kst_cast<KstPlotGroup>(obj);
  if (g) {
    return new KstBindGroup(exec, g);
  }
  return 0L;
}


KstBindGroup::KstBindGroup(int id, const char *name)
: KstBindViewObject(id, name ? name : "Group Method") {
}


KstBindGroup::~KstBindGroup() {
}


KJS::Object KstBindGroup::construct(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Object();
  }

  KstViewObjectPtr view = extractViewObject(exec, args[0]);
  if (!view) {
    KstViewWindow *w = extractWindow(exec, args[0]);
    if (w) {
      view = w->view();
    } else {
      KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
      exec->setException(eobj);
      return KJS::Object();
    }
  }

  KstPlotGroupPtr b = new KstPlotGroup;
  view->appendChild(b.data());
  KstApp::inst()->paintAll(KstPainter::P_PAINT);
  return KJS::Object(new KstBindGroup(exec, b));
}


struct GroupBindings {
  const char *name;
  KJS::Value (KstBindGroup::*method)(KJS::ExecState*, const KJS::List&);
};


struct GroupProperties {
  const char *name;
  void (KstBindGroup::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindGroup::*get)(KJS::ExecState*) const;
};


static GroupBindings groupBindings[] = {
  { "append", &KstBindGroup::append },
  { "prepend", &KstBindGroup::prepend },
  { "remove", &KstBindGroup::remove },
  { "ungroup", &KstBindGroup::ungroup },
  { 0L, 0L }
};


static GroupProperties groupProperties[] = {
  { 0L, 0L, 0L }
};


int KstBindGroup::methodCount() const {
  return sizeof groupBindings + KstBindViewObject::methodCount();
}


int KstBindGroup::propertyCount() const {
  return sizeof groupProperties + KstBindViewObject::propertyCount();
}


KJS::ReferenceList KstBindGroup::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBindViewObject::propList(exec, recursive);

  for (int i = 0; groupProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(groupProperties[i].name)));
  }

  return rc;
}


bool KstBindGroup::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; groupProperties[i].name; ++i) {
    if (prop == groupProperties[i].name) {
      return true;
    }
  }

  return KstBindViewObject::hasProperty(exec, propertyName);
}


void KstBindGroup::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBindViewObject::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; groupProperties[i].name; ++i) {
    if (prop == groupProperties[i].name) {
      if (!groupProperties[i].set) {
        break;
      }
      (this->*groupProperties[i].set)(exec, value);
      return;
    }
  }

  KstBindViewObject::put(exec, propertyName, value, attr);
}


KJS::Value KstBindGroup::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBindViewObject::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; groupProperties[i].name; ++i) {
    if (prop == groupProperties[i].name) {
      if (!groupProperties[i].get) {
        break;
      }
      return (this->*groupProperties[i].get)(exec);
    }
  }

  return KstBindViewObject::get(exec, propertyName);
}


KJS::Value KstBindGroup::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  int start = KstBindViewObject::methodCount();
  if (id > start) {
    KstBindGroup *imp = dynamic_cast<KstBindGroup*>(self.imp());
    if (!imp) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
      exec->setException(eobj);
      return KJS::Undefined();
    }

    return (imp->*groupBindings[id - start - 1].method)(exec, args);
  }

  return KstBindViewObject::call(exec, self, args);
}


void KstBindGroup::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  int start = KstBindViewObject::methodCount();
  for (int i = 0; groupBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindGroup(i + start + 1));
    obj.put(exec, groupBindings[i].name, o, KJS::Function);
  }
}

#define makeGroup(X) dynamic_cast<KstPlotGroup*>(const_cast<KstObject*>(X.data()))

KJS::Value KstBindGroup::append(KJS::ExecState *exec, const KJS::List& args) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Null();
  }

  KstViewObjectPtr view = extractViewObject(exec, args[0]);
  if (!view) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstPlotGroupPtr d = makeGroup(_d);
  if (d) {
    if (d->tagName() != view->tagName()) {
      if( d->parent() == view->parent()) {
        KstWriteLocker wl(d);
        QRect geom;

        if (d->children().count() > 0) {
          geom = d->geometry();
          geom |= view->geometry();
        } else {
          geom = view->geometry();
        }

        view->setSelected(false);
        view->setFocus(false);
        view->detach();

        d->modifyGeometry(geom);
        d->appendChild(view);

        KstApp::inst()->paintAll(KstPainter::P_PAINT);
      } else {
        KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Object to be added to group must be at the same level as the group");
        exec->setException(eobj);
        return KJS::Undefined();
      }
    } else {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Unable to add group to itself");
      exec->setException(eobj);
      return KJS::Undefined();
    }
  }

  return KJS::Undefined();
}


KJS::Value KstBindGroup::prepend(KJS::ExecState *exec, const KJS::List& args) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires exactly one argument.");
    exec->setException(eobj);
    return KJS::Null();
  }

  KstViewObjectPtr view = extractViewObject(exec, args[0]);
  if (!view) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  KstPlotGroupPtr d = makeGroup(_d);
  if (d) {
    if (d->tagName() != view->tagName()) {
      if( d->parent() == view->parent()) {
        KstWriteLocker wl(d);
        QRect geom;

        if (d->children().count() > 0) {
          geom = d->geometry();
          geom |= view->geometry();
        } else {
          geom = view->geometry();
        }

        view->setSelected(false);
        view->setFocus(false);
        view->detach();

        d->modifyGeometry(geom);
        d->prependChild(view);

        KstApp::inst()->paintAll(KstPainter::P_PAINT);
      } else {
        KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Object to be added to group must be at the same level as the group");
        exec->setException(eobj);
        return KJS::Undefined();
      }
    } else {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError, "Unable to add group to itself");
      exec->setException(eobj);
      return KJS::Undefined();
    }
  }

  return KJS::Undefined();
}


KJS::Value KstBindGroup::ungroup(KJS::ExecState *exec, const KJS::List& args) {
  if (!_d) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (args.size() != 0) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError, "Requires no arguments.");
    exec->setException(eobj);
    return KJS::Null();
  }

  KstPlotGroupPtr d = makeGroup(_d);
  if (d) {
    KstViewObjectPtr parent = d->parent();
    KstViewObjectPtr vo = kst_cast<KstViewObject>(d);

    if (vo) {
      if (parent) {
        KstWriteLocker wl(d);

        d->flatten();
        parent->appendChild(vo);

        KstApp::inst()->paintAll(KstPainter::P_PAINT);
      }
    }
  }

  return KJS::Undefined();
}

#undef makeGroup
