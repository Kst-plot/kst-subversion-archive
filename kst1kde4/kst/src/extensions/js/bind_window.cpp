/***************************************************************************
                              bind_window.cpp
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

#include "bind_window.h"
#include "bind_plotcollection.h"
#include "bind_viewobject.h"

#include <kst.h>
#include <kstviewwindow.h>

#include <kdebug.h>

KstBindWindow::KstBindWindow(KJS::ExecState *exec, KstViewWindow *d)
: KstBinding("Window"), _d(d) {
  KJS::Object o(this);
  addBindings(exec, o);
}


KstBindWindow::KstBindWindow(KJS::ExecState *exec, KJS::Object *globalObject)
: KstBinding("Window") {
  KJS::Object o(this);
  addBindings(exec, o);
  if (globalObject) {
    globalObject->put(exec, "Window", o);
  } else {
    _d = new KstViewWindow;
  }
}


KstBindWindow::KstBindWindow(int id)
: KstBinding("Window Method", id) {
}


KstBindWindow::~KstBindWindow() {
}


KJS::Object KstBindWindow::construct(KJS::ExecState *exec, const KJS::List& args) {
  QString name;
  if (args.size() > 1) {
    return createSyntaxError(exec);
  }

  if (args.size() == 1) {
    if (args[0].type() != KJS::StringType) {
      return createTypeError(exec, 0);
    }
    name = args[0].toString(exec).qstring();
  }

  name = KstApp::inst()->newWindow(name);
  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(name));
  if (!w) {
    return createGeneralError(exec, i18n("Failed to create new tab."));
  }

  return KJS::Object(new KstBindWindow(exec, w));
}


struct WindowBindings {
  const char *name;
  KJS::Value (KstBindWindow::*method)(KJS::ExecState*, const KJS::List&);
};


struct WindowProperties {
  const char *name;
  void (KstBindWindow::*set)(KJS::ExecState*, const KJS::Value&);
  KJS::Value (KstBindWindow::*get)(KJS::ExecState*) const;
};


static WindowBindings windowBindings[] = {
  { "close", &KstBindWindow::close },
  { "repaint", &KstBindWindow::repaint },
  { 0L, 0L }
};


static WindowProperties windowProperties[] = {
  { "name", &KstBindWindow::setWindowName, &KstBindWindow::windowName },
  { "plots", 0L, &KstBindWindow::plots },
  { "view", 0L, &KstBindWindow::view },
  { "columns", &KstBindWindow::setColumns, &KstBindWindow::columns },
  { 0L, 0L, 0L }
};


KJS::ReferenceList KstBindWindow::propList(KJS::ExecState *exec, bool recursive) {
  KJS::ReferenceList rc = KstBinding::propList(exec, recursive);

  for (int i = 0; windowProperties[i].name; ++i) {
    rc.append(KJS::Reference(this, KJS::Identifier(windowProperties[i].name)));
  }

  return rc;
}


bool KstBindWindow::hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  QString prop = propertyName.qstring();
  for (int i = 0; windowProperties[i].name; ++i) {
    if (prop == windowProperties[i].name) {
      return true;
    }
  }

  return KstBinding::hasProperty(exec, propertyName);
}


void KstBindWindow::put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr) {
  if (!_d) {
    KstBinding::put(exec, propertyName, value, attr);
    return;
  }

  QString prop = propertyName.qstring();
  for (int i = 0; windowProperties[i].name; ++i) {
    if (prop == windowProperties[i].name) {
      if (!windowProperties[i].set) {
        break;
      }
      (this->*windowProperties[i].set)(exec, value);
      return;
    }
  }

  KstBinding::put(exec, propertyName, value, attr);
}


KJS::Value KstBindWindow::get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const {
  if (!_d) {
    return KstBinding::get(exec, propertyName);
  }

  QString prop = propertyName.qstring();
  for (int i = 0; windowProperties[i].name; ++i) {
    if (prop == windowProperties[i].name) {
      if (!windowProperties[i].get) {
        break;
      }
      return (this->*windowProperties[i].get)(exec);
    }
  }

  return KstBinding::get(exec, propertyName);
}


KJS::Value KstBindWindow::call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args) {
  int id = this->id();
  if (id <= 0) {
    return createInternalError(exec);
  }

  KstBindWindow *imp = dynamic_cast<KstBindWindow*>(self.imp());
  if (!imp) {
    return createInternalError(exec);
  }

  return (imp->*windowBindings[id - 1].method)(exec, args);
}


void KstBindWindow::addBindings(KJS::ExecState *exec, KJS::Object& obj) {
  for (int i = 0; windowBindings[i].name != 0L; ++i) {
    KJS::Object o = KJS::Object(new KstBindWindow(i + 1));
    obj.put(exec, windowBindings[i].name, o, KJS::Function);
  }
}


void KstBindWindow::setWindowName(KJS::ExecState *exec, const KJS::Value& value) {
  if (value.type() != KJS::StringType) {
    return createPropertyTypeError(exec);
  }
  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(value.toString(exec).qstring()));
  if (w) {
    return createPropertyGeneralError(exec, i18n("A window of this name already exists."));
  }
  _d->setCaption(value.toString(exec).qstring());
}


KJS::Value KstBindWindow::windowName(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  return KJS::String(_d->caption());
}


KJS::Value KstBindWindow::repaint(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  if (!_d) {
    return createInternalError(exec);
  }
  _d->view()->paint(KstPainter::P_PAINT);
  return KJS::Undefined();
}


KJS::Value KstBindWindow::close(KJS::ExecState *exec, const KJS::List& args) {
  Q_UNUSED(args)
  if (!_d) {
    return createInternalError(exec);
  }
  _d->view()->children().clear();

  //
  // the code following used to read:
  //    _d->close();
  // but this simply ends up POSTing a KMdiViewCloseEvent to the application.
  // As a result of this the window will be closed at some later time.
  // What we really want to do is close the window immediately to avoid race conditions,
  // so we SEND the message instead...
  //
  KMdiViewCloseEvent* ce = new KMdiViewCloseEvent(_d);
  QApplication::sendEvent(KstApp::inst(), ce);

  return KJS::Undefined();
}


void KstBindWindow::setColumns(KJS::ExecState *exec, const KJS::Value& value) {
  unsigned cols = 1;
  if (value.type() != KJS::NumberType || !value.toUInt32(cols)) {
    return createPropertyTypeError(exec);
  }

  if (!_d) {
    return createPropertyInternalError(exec);
  }

  KstTopLevelViewPtr tlv = _d->view();
  if (!tlv) {
    return createPropertyInternalError(exec);
  }

  KstWriteLocker wl(tlv);
  tlv->setOnGrid(true);
  tlv->setColumns(cols);
  tlv->cleanup(cols);
  tlv->paint(KstPainter::P_PAINT);
}


KJS::Value KstBindWindow::columns(KJS::ExecState *exec) const {
  if (!_d) {
    return createInternalError(exec);
  }

  KstTopLevelViewPtr tlv = _d->view();
  if( !tlv) {
    return createInternalError(exec);
  }

  KstReadLocker rl(tlv);
  return KJS::Number(tlv->columns());
}


KJS::Value KstBindWindow::plots(KJS::ExecState *exec) const {
  return KJS::Object(new KstBindPlotCollection(exec, _d));
}


KJS::Value KstBindWindow::view(KJS::ExecState *exec) const {
  return KJS::Object(KstBindViewObject::bind(exec, _d->view().data()));
}

