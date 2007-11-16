/***************************************************************************
                                bind_image.h
                             -------------------
    begin                : Nov 15 2007
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

#ifndef BIND_IMAGE_H
#define BIND_IMAGE_H

#include "bind_dataobject.h"
#include "bind_object.h"

#include <kstimage.h>
#include <kstmatrix.h>

/* @class Image
   @description .....
*/
class KstBindImage : public KstBindDataObject {
  public:
    /* @constructor
       @arg type name description
       @optarg type name description
       @description ....
    */
    KstBindImage(KJS::ExecState *exec, KstImagePtr d, const char *name = 0L);
    KstBindImage(KJS::ExecState *exec, KJS::Object *globalObject = 0L, const char *name = 0L);
    ~KstBindImage();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    int methodCount() const;
    int propertyCount() const;

  protected:
    KstBindImage(int id, const char *name = 0L);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    static KstBindDataObject *bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj);
};

#endif
