/***************************************************************************
                          bind_plotlabelcollection.h
                             -------------------
    begin                : Apr 08 2005
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

#ifndef BIND_PLOTCOLLECTION_H
#define BIND_PLOTCOLLECTION_H

#include "kstbinding.h"

#include <kst2dplot.h>
#include <kstlabel.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindPlotLabelCollection : public KstBinding {
  public:
    KstBindPlotLabelCollection(KJS::ExecState *exec, Kst2DPlotPtr d);
    ~KstBindPlotLabelCollection();

    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::Value getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    //KJS::Value resize(KJS::ExecState *exec, const KJS::List& args);

    // properties
    KJS::Value length(KJS::ExecState *exec) const;
    KJS::Value top(KJS::ExecState *exec) const;
    KJS::Value x(KJS::ExecState *exec) const;
    KJS::Value y(KJS::ExecState *exec) const;

  protected:
    KstBindPlotLabelCollection(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:
    Kst2DPlotPtr _d;
};


#endif

// vim: ts=2 sw=2 et
