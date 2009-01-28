/***************************************************************************
                              bind_binnedmap.h
                             -------------------
    begin                : Feb 18 2008
    copyright            : (C) 2008 The University of British Columbia
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

#ifndef BIND_BINNEDMAP_H
#define BIND_BINNEDMAP_H

#include "bind_dataobject.h"

#include <binnedmap.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

/* @class BinnedMap
   @inherits DataObject
   @description This class represents a binned map object in Kst.
*/

class KstBindBinnedMap : public KstBindDataObject {
  public:
    /* @constructor
       @description Main constructor for the BinnedMap class.
    */
    KstBindBinnedMap(KJS::ExecState *exec, KstDataObjectPtr d);
    KstBindBinnedMap(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindBinnedMap();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    /* @method validate
       @description Validates the plugin.
    */
    KJS::Value validate(KJS::ExecState *exec, const KJS::List& args);

    /* @property Vector x
       @description Contains the x vector.
    */
    void setX(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value x(KJS::ExecState *exec) const;

    /* @property Vector y
       @description Contains the y vector.
    */
    void setY(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value y(KJS::ExecState *exec) const;

    /* @property Vector z
       @description Contains the z vector.
    */
    void setZ(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value z(KJS::ExecState *exec) const;

    /* @property string binnedMap
       @description Contains the name of binned map matrix.
    */
    void setBinnedMap(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value binnedMap(KJS::ExecState *exec) const;

    /* @property string hitsMap
       @description Contains the name of the hits map matrix.
    */
    void setHitsMap(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value hitsMap(KJS::ExecState *exec) const;

    /* @property Scalar xFrom
       @description Contains the from value of the x vector.
    */
    void setXFrom(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xFrom(KJS::ExecState *exec) const;

    /* @property Scalar xTo
       @description Contains the to value of the x vector.
    */
    void setXTo(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xTo(KJS::ExecState *exec) const;

    /* @property Scalar yFrom
       @description Contains the from value of the y vector.
    */
    void setYFrom(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yFrom(KJS::ExecState *exec) const;

    /* @property Scalar yTo
       @description Contains the to value of the y vector.
    */
    void setYTo(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yTo(KJS::ExecState *exec) const;

    /* @property Scalar nX
       @description Contains the number of x bins.
    */
    void setNX(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value nX(KJS::ExecState *exec) const;

    /* @property Scalar nY
       @description Contains the number of y bins.
    */
    void setNY(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value nY(KJS::ExecState *exec) const;

    /* @property Scalar autobin
       @description Contains the autobin setting. Zero for no auto-binning and non-zero for auto-binning.
    */
    void setAutobin(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value autobin(KJS::ExecState *exec) const;

    /* @property boolean valid
       @readonly
       @description Returns true if all the inputs and outputs of the binned map have been set.
    */
    KJS::Value valid(KJS::ExecState *exec) const;

  protected:
    KstBindBinnedMap(int id, const char *name = 0L);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    static KstBindDataObject *bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj);
};


#endif
