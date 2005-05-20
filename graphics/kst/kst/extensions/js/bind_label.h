/***************************************************************************
                                bind_label.h
                             -------------------
    begin                : Apr 06 2005
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

#ifndef BIND_LABEL_H
#define BIND_LABEL_H

#include "kstbinding.h"

#include <kstlabel.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindLabel : public KstBinding {
  public:
    KstBindLabel(KJS::ExecState *exec, KstLabelPtr d);
    KstBindLabel(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindLabel();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    // properties
    void setText(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value text(KJS::ExecState *exec) const;
    void setRotation(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value rotation(KJS::ExecState *exec) const;
    void setInterpreted(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value interpreted(KJS::ExecState *exec) const;
    void setScalarReplacement(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value scalarReplacement(KJS::ExecState *exec) const;

  protected:
    KstBindLabel(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstLabelPtr _d;
};


#endif

// vim: ts=2 sw=2 et
