/***************************************************************************
                            bind_curvecollection.h
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

#ifndef BIND_CURVECOLLECTION_H
#define BIND_CURVECOLLECTION_H

#include "bind_collection.h"

#include <kst2dplot.h>
#include <kstvcurve.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindCurveCollection : public KstBindCollection {
  public:
    KstBindCurveCollection(KJS::ExecState *exec, Kst2DPlotPtr p);
    KstBindCurveCollection(KJS::ExecState *exec, KstVCurveList curves);
    ~KstBindCurveCollection();

    virtual KJS::Value length(KJS::ExecState *exec) const;

    virtual KJS::Value append(KJS::ExecState *exec, const KJS::List& args);
    virtual QStringList collection(KJS::ExecState *exec) const;
    virtual KJS::Value extract(KJS::ExecState *exec, const KJS::Identifier& item) const;
    virtual KJS::Value extract(KJS::ExecState *exec, unsigned item) const;

  protected:
    QStringList _curves;
    QString _plot;
    bool _isPlot;
};


#endif

// vim: ts=2 sw=2 et
