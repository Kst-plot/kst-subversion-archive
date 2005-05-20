/***************************************************************************
                                bind_string.h
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

#ifndef BIND_STRING_H
#define BIND_STRING_H

#include "kstbinding.h"

#include <kststring.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindString : public KstBinding {
  public:
    KstBindString(KJS::ExecState *exec, KstStringPtr s);
    KstBindString(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindString();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    void setValue(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value value(KJS::ExecState *exec) const;

  protected:
    KstBindString(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstStringPtr _s;
};


#endif

// vim: ts=2 sw=2 et
