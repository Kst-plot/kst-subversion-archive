/***************************************************************************
                              bind_datavector.h
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

#ifndef BIND_RVECTOR_H
#define BIND_RVECTOR_H

#include "kstbinding.h"

#include <kstrvector.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindDataVector : public KstBinding {
  public:
    KstBindDataVector(KJS::ExecState *exec, KstRVectorPtr s);
    KstBindDataVector(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindDataVector();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    KJS::Value reload(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value changeFile(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value changeFrames(KJS::ExecState *exec, const KJS::List& args);

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    KJS::Value valid(KJS::ExecState *exec) const;
    KJS::Value skip(KJS::ExecState *exec) const;
    KJS::Value boxcar(KJS::ExecState *exec) const;
    KJS::Value readToEnd(KJS::ExecState *exec) const;
    KJS::Value countFromEnd(KJS::ExecState *exec) const;
    KJS::Value skipLength(KJS::ExecState *exec) const;
    KJS::Value length(KJS::ExecState *exec) const;
    KJS::Value startFrame(KJS::ExecState *exec) const;
    KJS::Value startFrameRequested(KJS::ExecState *exec) const;
    KJS::Value frames(KJS::ExecState *exec) const;
    KJS::Value framesRequested(KJS::ExecState *exec) const;
    KJS::Value samplesPerFrame(KJS::ExecState *exec) const;
    KJS::Value field(KJS::ExecState *exec) const;
    KJS::Value dataSource(KJS::ExecState *exec) const;

  protected:
    KstBindDataVector(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:
    KstRVectorPtr _v;
};


#endif

// vim: ts=2 sw=2 et
