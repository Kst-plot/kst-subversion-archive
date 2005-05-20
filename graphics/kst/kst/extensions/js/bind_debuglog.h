/***************************************************************************
                               bind_debuglog.h
                             -------------------
    begin                : Apr 07 2005
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

#ifndef BIND_DEBUGLOG_H
#define BIND_DEBUGLOG_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindDebugLog : public KstBinding {
  public:
    KstBindDebugLog(KJS::ExecState *exec);
    ~KstBindDebugLog();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::Value getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // properties
    KJS::Value length(KJS::ExecState *exec) const;
    KJS::Value text(KJS::ExecState *exec) const;

  protected:
    KstBindDebugLog(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
};


#endif

// vim: ts=2 sw=2 et
