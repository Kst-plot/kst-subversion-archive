/***************************************************************************
                                bind_plugin.h
                             -------------------
    begin                : Apr 12 2005
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

#ifndef BIND_PLUGIN_H
#define BIND_PLUGIN_H

#include "kstbinding.h"

#include <kstplugin.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindPlugin : public KstBinding {
  public:
    KstBindPlugin(KJS::ExecState *exec, KstPluginPtr d);
    KstBindPlugin(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindPlugin();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    void setModule(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    KJS::Value module(KJS::ExecState *exec) const;
    KJS::Value lastError(KJS::ExecState *exec) const;
    KJS::Value valid(KJS::ExecState *exec) const;

  protected:
    KstBindPlugin(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstPluginPtr _d;
};


#endif

// vim: ts=2 sw=2 et
