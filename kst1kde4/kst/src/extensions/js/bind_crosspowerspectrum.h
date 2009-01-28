/***************************************************************************
                              bind_crosspowerspectrum.h
                             -------------------
    begin                : Feb 15 2008
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

#ifndef BIND_CROSSPOWERSPECTRUM_H
#define BIND_CROSSPOWERSPECTRUM_H

#include "bind_dataobject.h"

#include <crosspowerspectrum.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

/* @class CrossPowerSpectrum
   @inherits DataObject
   @description This class represents a cross power spectrum object in Kst.
*/

class KstBindCrossPowerSpectrum : public KstBindDataObject {
  public:
    KstBindCrossPowerSpectrum(KJS::ExecState *exec, KstDataObjectPtr d);
    KstBindCrossPowerSpectrum(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindCrossPowerSpectrum();

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

    /* @property Vector v1
       @description Contains the first vector.
    */
    void setV1(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value v1(KJS::ExecState *exec) const;

    /* @property Vector v2
       @description Contains the second vector.
    */
    void setV2(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value v2(KJS::ExecState *exec) const;

    /* @property Scalar length
       @description Contains the base 2 logarithm of the length of the cross power spectrum.  
                    Should be an integer &gt;= 2.
    */
    void setLength(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value length(KJS::ExecState *exec) const;

    /* @property Scalar sample
       @description Contains the sample rate.
    */
    void setSample(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value sample(KJS::ExecState *exec) const;

    /* @property string real
       @description Contains the name of the output vector holding the real part of the cross power spectrum.
    */
    void setReal(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value real(KJS::ExecState *exec) const;

    /* @property string imaginary
       @description Contains the name of the output vector holding the imaginary part of the cross power spectrum.
    */
    void setImaginary(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value imaginary(KJS::ExecState *exec) const;

    /* @property string frequency
       @description Contains the name of the output vector holding the frequencies of the cross power spectrum.
    */
    void setFrequency(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value frequency(KJS::ExecState *exec) const;

    /* @property boolean valid
       @readonly
       @description Returns true if all the inputs and outputs of the cross power spectrum have been set.
    */
    KJS::Value valid(KJS::ExecState *exec) const;

  protected:
    KstBindCrossPowerSpectrum(int id, const char *name = 0L);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    static KstBindDataObject *bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj);
};


#endif
