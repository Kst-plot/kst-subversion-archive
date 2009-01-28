/***************************************************************************
                                 bind_vectorview.h
                             -------------------
    begin                : Mar 13 2008
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

#ifndef BIND_VECTORVIEW_H
#define BIND_VECTORVIEW_H

#include "bind_dataobject.h"

#include <kstvectorview.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

/* @class VectorView
   @inherits DataObject
   @description Represents a vectorView.
*/
class KstBindVectorView : public KstBindDataObject {
  public:
    /* @constructor
       @arg Vector x The x-vector.
       @arg Vector y The y-vector.
       @description Creates a new vector view with the given x-vector and y-vector.
    */
    KstBindVectorView(KJS::ExecState *exec, KstVectorViewPtr v, const char *name = 0L);
    KstBindVectorView(KJS::ExecState *exec, KJS::Object *globalObject = 0L, const char *name = 0L);
    ~KstBindVectorView();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    int methodCount() const;
    int propertyCount() const;

    /* @property Vector xVector
       @description the x-vector.
    */
    void setXVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xVector(KJS::ExecState *exec) const;

    /* @property Vector yVector
       @description the y-vector.
    */
    void setYVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yVector(KJS::ExecState *exec) const;

    /* @property Vector flagVector
       @description the flag-vector. Those points corresponding to non-zero values of the flag vector
        are removed from the output vectors.
    */
    void setFlagVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value flagVector(KJS::ExecState *exec) const;

    /* @property Scalar xMin
       @description the value for the minimum x-value for the view range.
    */
    void setXMin(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xMin(KJS::ExecState *exec) const;

    /* @property Scalar xMax
       @description the value for the maximum x-value for the view range.
    */
    void setXMax(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xMax(KJS::ExecState *exec) const;

    /* @property Scalar yMin
       @description the value for the minimum y-value for the view range.
    */
    void setYMin(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yMin(KJS::ExecState *exec) const;

    /* @property Scalar yMax
       @description the value for the maximum y-value for the view range.
    */
    void setYMax(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yMax(KJS::ExecState *exec) const;

    /* @property boolean useXMin
       @description If true, the minimum x-value is respected.
    */
    void setUseXMin(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value useXMin(KJS::ExecState *exec) const;

    /* @property boolean useXMax
       @description If true, the maximum x-value is respected.
    */
    void setUseXMax(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value useXMax(KJS::ExecState *exec) const;

    /* @property boolean useYMin
       @description If true, the minimum y-value is respected.
    */
    void setUseYMin(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value useYMin(KJS::ExecState *exec) const;

    /* @property boolean useYMax
       @description If true, the maximum y-value is respected.
    */
    void setUseYMax(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value useYMax(KJS::ExecState *exec) const;

    /* @property number interpolateTo
       @description the value for the maximum y-value for the view range. Must be one of:
                    <ul>
                    <li>0 - interpolate to the x-vector</li>
                    <li>1 - interpolate to the y-vector</li>
                    <li>2 - interpolate to the high resolution vector</li>
                    <li>3 - interpolate to the low resolution vector</li>
                    </ul>
    */
    void setInterpolateTo(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value interpolateTo(KJS::ExecState *exec) const;

  protected:
    KstBindVectorView(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    static KstBindDataObject *bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj);
};


#endif
