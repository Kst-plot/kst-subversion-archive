/***************************************************************************
                               kstbinding.cpp
                             -------------------
    begin                : Mar 23 2005
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

#include "kstbinding.h"

#include "bind_curve.h"
#include "bind_datasource.h"
#include "bind_datavector.h"
#include "bind_vector.h"
#include "bind_window.h"

#include <kst.h>
#include <kstdatacollection.h>
#include <kstviewwindow.h>

using namespace KJSEmbed;

KstBinding::KstBinding(const QString& name, bool hasConstructor)
: JSProxy(JSProxy::BuiltinProxy), _name(name), _id(hasConstructor ? KST_BINDING_CONSTRUCTOR : KST_BINDING_NOCONSTRUCTOR) {
}


KstBinding::KstBinding(const QString& name, int id)
: JSProxy(JSProxy::BuiltinProxy), _name(name), _id(id) {
}


KstBinding::~KstBinding() {
}


QString KstBinding::typeName() const {
  return _name;
}


KJS::UString KstBinding::toString(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  return KJS::UString(_name.latin1());
}


bool KstBinding::implementsConstruct() const {
  return _id == KST_BINDING_CONSTRUCTOR;
}


bool KstBinding::implementsCall() const {
  return _id != KST_BINDING_CONSTRUCTOR && _id != KST_BINDING_NOCONSTRUCTOR;
}


bool KstBinding::inherits(const char *c) {
  return _name == c;
}


int KstBinding::id() const {
  return _id;
}


KstDataSourcePtr KstBinding::extractDataSource(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstDataSourcePtr dp;
        KstBindDataSource *imp = dynamic_cast<KstBindDataSource*>(value.toObject(exec).imp());
        if (imp) {
          dp = imp->_s;
        }
        if (!dp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
          exec->setException(eobj);
        }
        return dp;
      }
    case KJS::StringType:
      {
        KST::dataSourceList.lock().readLock();
        KstDataSourcePtr dp = *KST::dataSourceList.findFileName(value.toString(exec).qstring());
        KST::dataSourceList.lock().readUnlock();
        if (dp) {
          return dp;
        }
        dp = KstDataSource::loadSource(value.toString(exec).qstring());
        if (dp) {
          return dp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstVectorPtr KstBinding::extractVector(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstVectorPtr vp;
        KstBindVector *imp = dynamic_cast<KstBindVector*>(value.toObject(exec).imp());
        if (imp) {
          vp = imp->_v;
        } else {
          KstBindDataVector *imp = dynamic_cast<KstBindDataVector*>(value.toObject(exec).imp());
          if (imp) {
            vp = imp->_v;
          }
        }
        if (!vp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
          exec->setException(eobj);
        }
        return vp;
      }
    case KJS::StringType:
      {
        KST::vectorList.lock().readLock();
        KstVectorPtr vp = *KST::vectorList.findTag(value.toString(exec).qstring());
        KST::vectorList.lock().readUnlock();
        if (vp) {
          return vp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstVCurvePtr KstBinding::extractVCurve(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstVCurvePtr vp;
        KstBindCurve *imp = dynamic_cast<KstBindCurve*>(value.toObject(exec).imp());
        if (imp) {
          vp = imp->_d;
        }
        if (!vp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
          exec->setException(eobj);
        }
        return vp;
      }
    case KJS::StringType:
      {
        KST::dataObjectList.lock().readLock();
        KstVCurvePtr vp = kst_cast<KstVCurve>(*KST::dataObjectList.findTag(value.toString(exec).qstring()));
        KST::dataObjectList.lock().readUnlock();
        if (vp) {
          return vp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstViewWindow *KstBinding::extractWindow(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstBindWindow *imp = dynamic_cast<KstBindWindow*>(value.toObject(exec).imp());
        if (!imp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
          exec->setException(eobj);
        }
        return imp->_d;
      }
    case KJS::StringType:
      {
        KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(value.toString(exec).qstring()));
        if (w) {
          return w;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError);
        exec->setException(eobj);
      }
      return 0L;
  }
}


// vim: ts=2 sw=2 et
