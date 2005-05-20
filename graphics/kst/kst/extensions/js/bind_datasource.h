/***************************************************************************
                              bind_datasource.h
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

#ifndef BIND_DATASOURCE_H
#define BIND_DATASOURCE_H

#include "kstbinding.h"

#include <kstdatasource.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindDataSource : public KstBinding {
  public:
    KstBindDataSource(KJS::ExecState *exec, KstDataSourcePtr s);
    KstBindDataSource(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindDataSource();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    KJS::Value isValidField(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value fieldList(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value samplesPerFrame(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value frameCount(KJS::ExecState *exec, const KJS::List& args);

    // properties
    KJS::Value valid(KJS::ExecState *exec) const;
    KJS::Value empty(KJS::ExecState *exec) const;
    KJS::Value completeFieldList(KJS::ExecState *exec) const;
    KJS::Value fileName(KJS::ExecState *exec) const;
    KJS::Value fileType(KJS::ExecState *exec) const;
    KJS::Value source(KJS::ExecState *exec) const;
    KJS::Value metaData(KJS::ExecState *exec) const;

  protected:
    friend class KstBinding;
    friend class KstBindDataVector;
    KstBindDataSource(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstDataSourcePtr _s;
};


#endif

// vim: ts=2 sw=2 et
