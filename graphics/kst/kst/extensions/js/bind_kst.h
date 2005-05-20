/***************************************************************************
                                 bind_kst.h
                             -------------------
    begin                : Mar 28 2005
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

#ifndef BIND_KST_H
#define BIND_KST_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

class KstJS;

using namespace KJSEmbed;

class KstBindKst : public KstBinding {
  public:
    KstBindKst(KJS::ExecState *exec, KJS::Object *globalObject = 0L, KstJS *ext = 0L);
    ~KstBindKst();

    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    KJS::Value resetInterpreter(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value purge(KJS::ExecState *exec, const KJS::List& args);

    KJS::Value dataSources(KJS::ExecState *exec) const;
    KJS::Value scalars(KJS::ExecState *exec) const;
    KJS::Value strings(KJS::ExecState *exec) const;
    KJS::Value vectors(KJS::ExecState *exec) const;
    KJS::Value windows(KJS::ExecState *exec) const;
    KJS::Value objects(KJS::ExecState *exec) const;
    KJS::Value colors(KJS::ExecState *exec) const;
    KJS::Value extensions(KJS::ExecState *exec) const;
    KJS::Value document(KJS::ExecState *exec) const;
    KJS::Value pluginManager(KJS::ExecState *exec) const;

  protected:
    KstBindKst(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstJS *_ext;
};


#endif

// vim: ts=2 sw=2 et
