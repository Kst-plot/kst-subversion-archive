/***************************************************************************
                             bind_pluginmodule.h
                             -------------------
    begin                : Mar 29 2005
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

#ifndef BIND_PLUGINMODULE_H
#define BIND_PLUGINMODULE_H

#include "kstbinding.h"

#include <plugin.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindPluginModule : public KstBinding {
  public:
    KstBindPluginModule(KJS::ExecState *exec, const Plugin::Data& d);
    ~KstBindPluginModule();

    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // properties
//    KJS::Value xmlFile(KJS::ExecState *exec) const;
//    KJS::Value libraryFile(KJS::ExecState *exec) const;
    KJS::Value usesLocalData(KJS::ExecState *exec) const;
    KJS::Value isFit(KJS::ExecState *exec) const;
    KJS::Value isFilter(KJS::ExecState *exec) const;
    KJS::Value name(KJS::ExecState *exec) const;
    KJS::Value readableName(KJS::ExecState *exec) const;
    KJS::Value author(KJS::ExecState *exec) const;
    KJS::Value description(KJS::ExecState *exec) const;
    KJS::Value version(KJS::ExecState *exec) const;
    KJS::Value inputs(KJS::ExecState *exec) const;
    KJS::Value outputs(KJS::ExecState *exec) const;

  protected:
    KstBindPluginModule(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    Plugin::Data _d;
};


#endif

// vim: ts=2 sw=2 et
