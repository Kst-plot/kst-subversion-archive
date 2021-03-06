/***************************************************************************
                               kstbinding.cpp
                             -------------------
    begin                : Mar 23 2005
    copyright            : (C) 2005 The University of Toronto
                           (C) 2007 The University of British Columbia
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
#include "bind_curvecollection.h"
#include "bind_datasource.h"
#include "bind_datavector.h"
#include "bind_matrix.h"
#include "bind_object.h"
#include "bind_plot.h"
#include "bind_pluginmodule.h"
#include "bind_scalar.h"
#include "bind_string.h"
#include "bind_vector.h"
#include "bind_window.h"

#include <kst.h>
#include <kstdatacollection.h>
#include <kstdataobjectcollection.h>
#include <kstviewwindow.h>
#include <plugincollection.h>

KstBinding::KstBinding(const QString& name, bool hasConstructor)
: KJS::ObjectImp(), _name(name), _id(hasConstructor ? KST_BINDING_CONSTRUCTOR : KST_BINDING_NOCONSTRUCTOR) {
}


KstBinding::KstBinding(const QString& name, int id)
: KJS::ObjectImp(), _name(name), _id(id) {
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
          dp = kst_cast<KstDataSource>(imp->_d);
        }
        if (!dp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract dataSource.");
          exec->setException(eobj);
        }
        return dp;
      }
    case KJS::StringType:
      {
        KST::dataSourceList.lock().readLock();
        KstDataSourcePtr dp = *KST::dataSourceList.findFileName(value.toString(exec).qstring());
        KST::dataSourceList.lock().unlock();
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
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract dataSource.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstPluginPtr KstBinding::extractPluginModule(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstPluginPtr dp;
        KstBindPluginModule *imp = dynamic_cast<KstBindPluginModule*>(value.toObject(exec).imp());

        if (imp) {
          Plugin::Data d = imp->_d;
          PluginCollection *pc = PluginCollection::self();
          dp = pc->plugin(d._name);
        }

        if (!dp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract plugin.");
          exec->setException(eobj);
        }

        return dp;
      }

      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract plugin.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstBasicPluginPtr KstBinding::extractBasicPluginModule(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstBasicPluginPtr bp;
        KstBindPluginModule *imp = dynamic_cast<KstBindPluginModule*>(value.toObject(exec).imp());

        if (imp) {
          KstDataObjectPtr ptr = KstDataObject::plugin(imp->name(exec).toString(exec).qstring());
          if (ptr) {
            bp = kst_cast<KstBasicPlugin>(ptr);
          }
        }

        if (!bp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract basic plugin.");
          exec->setException(eobj);
        }

        return bp;
      }

      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract basic plugin.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstDataObjectPtr KstBinding::extractDataObject(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstDataObjectPtr dp;
        KstBindDataObject *imp = dynamic_cast<KstBindDataObject*>(value.toObject(exec).imp());
        if (imp) {
          dp = kst_cast<KstDataObject>(imp->_d);
        }
        if (!dp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract dataObject.");
          exec->setException(eobj);
        }
        return dp;
      }
    case KJS::StringType:
      {
        KST::dataObjectList.lock().readLock();
        KstDataObjectPtr dp = *KST::dataObjectList.findTag(value.toString(exec).qstring());
        KST::dataObjectList.lock().unlock();
        if (dp) {
          return dp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract dataObject.");
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
          vp = kst_cast<KstVector>(imp->_d);
        } else {
          KstBindDataVector *imp = dynamic_cast<KstBindDataVector*>(value.toObject(exec).imp());
          if (imp) {
            vp = kst_cast<KstVector>(imp->_d);
          }
        }
        if (!vp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract vector.");
          exec->setException(eobj);
        }
        return vp;
      }
    case KJS::StringType:
      {
        KST::vectorList.lock().readLock();
        KstVectorPtr vp = *KST::vectorList.findTag(value.toString(exec).qstring());
        KST::vectorList.lock().unlock();
        if (vp) {
          return vp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract vector.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstScalarPtr KstBinding::extractScalar(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstScalarPtr sp;
        KstBindScalar *imp = dynamic_cast<KstBindScalar*>(value.toObject(exec).imp());
        if (imp) {
          sp = kst_cast<KstScalar>(imp->_d);
        }
        if (!sp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract scalar.");
          exec->setException(eobj);
        }
        return sp;
      }
    case KJS::StringType:
      {
        KST::scalarList.lock().readLock();
        KstScalarPtr sp = *KST::scalarList.findTag(value.toString(exec).qstring());
        KST::scalarList.lock().unlock();
        if (sp) {
          return sp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract scalar.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstStringPtr KstBinding::extractString(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstStringPtr sp;
        KstBindString *imp = dynamic_cast<KstBindString*>(value.toObject(exec).imp());
        if (imp) {
          sp = kst_cast<KstString>(imp->_d);
        }
        if (!sp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract string.");
          exec->setException(eobj);
        }
        return sp;
      }
    case KJS::StringType:
      {
        KST::stringList.lock().readLock();
        KstStringPtr sp = *KST::stringList.findTag(value.toString(exec).qstring());
        KST::stringList.lock().unlock();
        if (sp) {
          return sp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract scalar.");
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
          vp = kst_cast<KstVCurve>(imp->_d);
        }
        if (!vp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract curve.");
          exec->setException(eobj);
        }
        return vp;
      }
    case KJS::StringType:
      {
        KST::dataObjectList.lock().readLock();
        KstVCurvePtr vp = kst_cast<KstVCurve>(*KST::dataObjectList.findTag(value.toString(exec).qstring()));
        KST::dataObjectList.lock().unlock();
        if (vp) {
          return vp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract curve.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstMatrixPtr KstBinding::extractMatrix(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstMatrixPtr mp;
        KstBindMatrix *imp = dynamic_cast<KstBindMatrix*>(value.toObject(exec).imp());
        if (imp) {
          mp = kst_cast<KstMatrix>(imp->_d);
        } else {
          KstBindMatrix *imp = dynamic_cast<KstBindMatrix*>(value.toObject(exec).imp());
          if (imp) {
            mp = kst_cast<KstMatrix>(imp->_d);
          }
        }
        if (!mp && doThrow) {
          KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract matrix.");
          exec->setException(eobj);
        }
        return mp;
      }
    case KJS::StringType:
      {
        KST::matrixList.lock().readLock();
        KstMatrixPtr mp = *KST::matrixList.findTag(value.toString(exec).qstring());
        KST::matrixList.lock().unlock();
        if (mp) {
          return mp;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract matrix.");
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
        if (!imp) {
          if (doThrow) {
            KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract window.");
            exec->setException(eobj);
          }
          return 0L;
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
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract window.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


Kst2DPlotPtr KstBinding::extractPlot(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstBindPlot *imp = dynamic_cast<KstBindPlot*>(value.toObject(exec).imp());
        if (!imp) {
          if (doThrow) {
            KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract plot.");
            exec->setException(eobj);
          }
          return 0L;
        }
        return kst_cast<Kst2DPlot>(static_cast<KstBindViewObject*>(imp)->_d);
      }
    case KJS::StringType:
      {
        Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(value.toString(exec).qstring());
        if (p) {
          return p;
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract plot.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstViewObjectPtr KstBinding::extractViewObject(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstBindViewObject *imp = dynamic_cast<KstBindViewObject*>(value.toObject(exec).imp());
        if (!imp) {
          KstViewWindow *w = extractWindow(exec, value, false);
          if (!w) {
            if (doThrow) {
              KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract viewObject.");
              exec->setException(eobj);
            }
            return 0L;
          }
          return w->view().data();
        }
        return kst_cast<KstViewObject>(imp->_d);
      }
    case KJS::StringType:
      {
        KstViewWindow *w = extractWindow(exec, value, false);
        if (w) {
          return w->view().data();
        }
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract viewObject.");
        exec->setException(eobj);
      }
      return 0L;
  }
}


KstBaseCurveList KstBinding::extractCurveList(KJS::ExecState *exec, const KJS::Value& value, bool doThrow) {
  KstBaseCurveList rc;
  switch (value.type()) {
    case KJS::ObjectType:
      {
        KstBindCurveCollection *imp = dynamic_cast<KstBindCurveCollection*>(value.toObject(exec).imp());
        if (!imp) {
          if (doThrow) {
            KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract curve list.");
            exec->setException(eobj);
          }
          return rc;
        }
        if (imp->_isPlot) {
          Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(imp->_plot);
          if (p) {
            for (KstBaseCurveList::ConstIterator i = p->Curves.begin(); i != p->Curves.end(); ++i) {
              rc += *i;
            }
          }
        } else if (imp->_legend) {
          for (KstBaseCurveList::ConstIterator i = imp->_legend->curves().begin(); i != imp->_legend->curves().end(); ++i) {
            rc += *i;
          }
        } else {
          KstBaseCurveList cl = kstObjectSubList<KstDataObject,KstBaseCurve>(KST::dataObjectList);
          for (KstBaseCurveList::ConstIterator i = cl.begin(); i != cl.end(); ++i) {
            (*i)->readLock();
            if (imp->_curves.contains((*i)->tagName())) {
                rc += *i;
            }
            (*i)->unlock();
          }
        }
        return rc;
      }
      // fall through and throw
    default:
      if (doThrow) {
        KJS::Object eobj = KJS::Error::create(exec, KJS::TypeError, "Failed to extract curve list.");
        exec->setException(eobj);
      }
  }
  return rc;
}


int KstBinding::methodCount() const {
  return 0;
}


int KstBinding::propertyCount() const {
  return 0;
}

