/***************************************************************************
                               bind_window.h
                             -------------------
    begin                : Mar 30 2005
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

#ifndef BIND_WINDOW_H
#define BIND_WINDOW_H

#include "kstbinding.h"

#include <kstviewwindow.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindWindow : public KstBinding {
  public:
    KstBindWindow(KJS::ExecState *exec, KstViewWindow *d);
    KstBindWindow(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindWindow();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    KJS::Value close(KJS::ExecState *exec, const KJS::List& args);

    // properties
    void setWindowName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value windowName(KJS::ExecState *exec) const;
    KJS::Value plots(KJS::ExecState *exec) const;

  protected:
    KstBindWindow(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:
    QGuardedPtr<KstViewWindow> _d;
};


#endif

// vim: ts=2 sw=2 et
