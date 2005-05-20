/***************************************************************************
                               bind_document.h
                             -------------------
    begin                : Apr 04 2005
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

#ifndef BIND_DOCUMENT_H
#define BIND_DOCUMENT_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

class KstJS;

using namespace KJSEmbed;

class KstBindDocument : public KstBinding {
  public:
    KstBindDocument(KJS::ExecState *exec);
    ~KstBindDocument();

    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState*, const KJS::Identifier&, const KJS::Value&, int);

    KJS::Value save(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value newDocument(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value load(KJS::ExecState *exec, const KJS::List& args);

    KJS::Value text(KJS::ExecState *exec) const;
    KJS::Value name(KJS::ExecState *exec) const;
    void setName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value modified(KJS::ExecState *exec) const;
    void setModified(KJS::ExecState *exec, const KJS::Value& value);

  protected:
    KstBindDocument(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
};


#endif

// vim: ts=2 sw=2 et
