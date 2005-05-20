/***************************************************************************
                                 bind_plot.h
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

#ifndef BIND_PLOT_H
#define BIND_PLOT_H

#include "kstbinding.h"

#include <kst2dplot.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindPlot : public KstBinding {
  public:
    KstBindPlot(KJS::ExecState *exec, Kst2DPlotPtr d);
    KstBindPlot(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindPlot();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    KJS::Value curves(KJS::ExecState *exec) const;
    KJS::Value labels(KJS::ExecState *exec) const;
    KJS::Value legend(KJS::ExecState *exec) const;

  protected:
    KstBindPlot(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    Kst2DPlotPtr _d;
};


#endif

// vim: ts=2 sw=2 et
