/***************************************************************************
                                bind_scalar.h
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

#ifndef BIND_SCALAR_H
#define BIND_SCALAR_H

#include "kstbinding.h"

#include <kstscalar.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindScalar : public KstBinding {
  public:
    KstBindScalar(KJS::ExecState *exec, KstScalarPtr s);
    KstBindScalar(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindScalar();

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
    KstBindScalar(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstScalarPtr _s;
};


#endif

// vim: ts=2 sw=2 et
