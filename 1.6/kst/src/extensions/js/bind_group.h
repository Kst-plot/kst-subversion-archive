/***************************************************************************
                              bind_group.h
                             --------------
    begin                : Nov 23 2007
    copyright            : (C) 2007 The University of British Columbia
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

#ifndef BIND_GROUP_H
#define BIND_GROUP_H

#include "bind_viewobject.h"

#include <kstplotgroup.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

/* @class Group
   @inherits ViewObject
   @collection ViewObjectCollection
   @description A customizable group object.
*/
class KstBindGroup : public KstBindViewObject {
  public:
    /* @constructor
       @arg ViewObject parent The parent to place the new arrow in.  May also
                              be a string containing the name of an existing
                              ViewObject.
       @description Creates a new arrow and places it in the ViewObject <i>parent</i>.
    */
    /* @constructor
       @arg Window window The window to place the new arrow in.  May also be a
                          string containing the name of an existing Window.
       @description Creates a new arrow and places it in the Window <i>window</i>.
    */
    KstBindGroup(KJS::ExecState *exec, KstPlotGroupPtr d, const char *name = 0L);
    KstBindGroup(KJS::ExecState *exec, KJS::Object *globalObject = 0L, const char *name = 0L);
    ~KstBindGroup();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    int methodCount() const;
    int propertyCount() const;

    /* @method append
       @arg Object newObj The new object to append to the collection.
       @description Appends a new object to the end of the collection.
    */
    // default throws an exception
    virtual KJS::Value append(KJS::ExecState *exec, const KJS::List& args);

    /* @method prepend
       @arg Object newObj The new object to prepend to the collection.
       @description Prepends a new object to the start of the collection.
    */
    // default throws an exception
    virtual KJS::Value prepend(KJS::ExecState *exec, const KJS::List& args);

    /* @method ungroup
       @arg
       @description Removes all objects from the collection.
    */
    // default throws an exception
    virtual KJS::Value ungroup(KJS::ExecState *exec, const KJS::List& args);

  protected:
    KstBindGroup(int id, const char *name = 0L);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    static KstBindViewObject *bindFactory(KJS::ExecState *exec, KstViewObjectPtr obj);
};

#endif

