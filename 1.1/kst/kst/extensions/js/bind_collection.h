/***************************************************************************
                              bind_collection.h
                             -------------------
    begin                : Mar 31 2005
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

#ifndef BIND_COLLECTION_H
#define BIND_COLLECTION_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindCollection : public KstBinding {
  public:
    KstBindCollection(KJS::ExecState *exec, const QString& name, bool readOnly = true);
    ~KstBindCollection();

    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::Value getPropertyByIndex(KJS::ExecState *exec, unsigned propertyName) const;
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    // Read-only right now
    //void putPropertyByIndex(KJS::ExecState *exec, unsigned propertyName, const KJS::Value &value, int attr = KJS::None);
    //void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // default throws an exception
    virtual KJS::Value append(KJS::ExecState *exec, const KJS::List& args);
    virtual KJS::Value length(KJS::ExecState *exec) const;
    virtual QStringList collection(KJS::ExecState *exec) const;
    virtual KJS::Value extract(KJS::ExecState *exec, const KJS::Identifier& item) const;
    virtual KJS::Value extract(KJS::ExecState *exec, unsigned item) const;


  protected:
    KstBindCollection(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    bool _readOnly;
};


#endif

// vim: ts=2 sw=2 et
