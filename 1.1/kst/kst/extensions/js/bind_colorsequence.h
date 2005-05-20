/***************************************************************************
                            bind_colorsequence.h
                             -------------------
    begin                : Apr 11 2005
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

#ifndef BIND_COLORSEQUENCE_H
#define BIND_COLORSEQUENCE_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindColorSequence : public KstBinding {
  public:
    KstBindColorSequence(KJS::ExecState *exec);
    ~KstBindColorSequence();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    KJS::Value next(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value tooClose(KJS::ExecState *exec, const KJS::List& args);

    // properties

  protected:
    KstBindColorSequence(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
};


#endif

// vim: ts=2 sw=2 et
