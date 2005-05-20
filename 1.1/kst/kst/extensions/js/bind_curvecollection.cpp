/***************************************************************************
                           bind_curvecollection.cpp
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

#include "bind_curvecollection.h"
#include "bind_curve.h"

#include <kst.h>
#include <kstdatacollection.h>

#include <kdebug.h>

using namespace KJSEmbed;

KstBindCurveCollection::KstBindCurveCollection(KJS::ExecState *exec, KstVCurveList curves)
: KstBindCollection(exec, "CurveCollection", true) {
  _isPlot = false;
  _curves = curves.tagNames();
}


KstBindCurveCollection::KstBindCurveCollection(KJS::ExecState *exec, Kst2DPlotPtr p)
: KstBindCollection(exec, "CurveCollection", false) {
  _isPlot = true;
  p->readLock();
  _plot = p->tagName();
  p->readUnlock();
}


KstBindCurveCollection::~KstBindCurveCollection() {
}


KJS::Value KstBindCurveCollection::length(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  if (_isPlot) {
    Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(_plot);
    if (!p) {
      return KJS::Number(0);
    }
    KstReadLocker rl(p);
    return KJS::Number(p->Curves.count());
  }
  return KJS::Number(_curves.count());
}


QStringList KstBindCurveCollection::collection(KJS::ExecState *exec) const {
  Q_UNUSED(exec)
  if (_isPlot) {
    Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(_plot);
    if (!p) {
      return QStringList();
    }
    KstReadLocker rl(p);
    return p->Curves.tagNames();
  }

  return _curves;
}


KJS::Value KstBindCurveCollection::extract(KJS::ExecState *exec, const KJS::Identifier& item) const {
  KstVCurveList cl;
  if (_isPlot) {
    Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(_plot);
    if (!p) {
      return KJS::Undefined();
    }
    KstReadLocker rl(p);
    cl = kstObjectSubList<KstBaseCurve,KstVCurve>(p->Curves);
  } else {
    cl = kstObjectSubList<KstDataObject,KstVCurve>(KST::dataObjectList);
  }

  KstVCurvePtr c = *cl.findTag(item.qstring());
  if (c) {
    return KJS::Object(new KstBindCurve(exec, c));
  }
  return KJS::Undefined();
}


KJS::Value KstBindCurveCollection::extract(KJS::ExecState *exec, unsigned item) const {
  KstVCurveList cl;
  if (_isPlot) {
    Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(_plot);
    if (!p) {
      return KJS::Null();
    }
    KstReadLocker rl(p);
    cl = kstObjectSubList<KstBaseCurve,KstVCurve>(p->Curves);
  } else {
    cl = kstObjectSubList<KstDataObject,KstVCurve>(KST::dataObjectList);
  }

  KstVCurvePtr c;
  if (item < cl.count()) {
    c = cl[item];
  }
  if (c) {
    return KJS::Object(new KstBindCurve(exec, c));
  }
  return KJS::Undefined();
}


KJS::Value KstBindCurveCollection::append(KJS::ExecState *exec, const KJS::List& args) {
  if (args.size() != 1) {
    KJS::Object eobj = KJS::Error::create(exec, KJS::SyntaxError);
    exec->setException(eobj);
    return KJS::Undefined();
  }

  if (_isPlot) {
    KstVCurvePtr c = extractVCurve(exec, args[0]);
    if (!c) {
      return KJS::Undefined();
    }

    Kst2DPlotPtr p = *Kst2DPlot::globalPlotList().findTag(_plot);
    if (!p) {
      KJS::Object eobj = KJS::Error::create(exec, KJS::GeneralError);
      exec->setException(eobj);
      return KJS::Undefined();
    }
    KstWriteLocker rl(p);
    if (!p->Curves.contains(c.data())) {
      p->Curves.append(c.data());
      p->setDirty();
      KstApp::inst()->paintAll(P_PAINT);
    }
    return KJS::Undefined();
  }

  return KstBindCollection::append(exec, args);
}


// vim: ts=2 sw=2 et
