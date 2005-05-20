/***************************************************************************
                                 bind_vector.h
                             -------------------
    begin                : Mar 23 2005
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

#ifndef BIND_VECTOR_H
#define BIND_VECTOR_H

#include "kstbinding.h"

#include <kstvector.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindVector : public KstBinding {
  public:
    KstBindVector(KJS::ExecState *exec, KstVectorPtr v);
    KstBindVector(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindVector();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::Value getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const;
    void putPropertyByIndex(KJS::ExecState *exec, unsigned propertyName, const KJS::Value &value, int attr = KJS::None);
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    KJS::Value resize(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value interpolate(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value zero(KJS::ExecState *exec, const KJS::List& args);

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    KJS::Value length(KJS::ExecState *exec) const;
    KJS::Value min(KJS::ExecState *exec) const;
    KJS::Value max(KJS::ExecState *exec) const;
    KJS::Value mean(KJS::ExecState *exec) const;
    KJS::Value numNew(KJS::ExecState *exec) const;
    KJS::Value numShifted(KJS::ExecState *exec) const;
    KJS::Value editable(KJS::ExecState *exec) const;

  protected:
    KstBindVector(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:
    KstVectorPtr _v;
};


#endif

// vim: ts=2 sw=2 et
