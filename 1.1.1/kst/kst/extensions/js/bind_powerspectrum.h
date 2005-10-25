/***************************************************************************
                             bind_powerspectrum.h
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

#ifndef BIND_PSD_H
#define BIND_PSD_H

#include "kstbinding.h"

#include <kstpsd.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindPowerSpectrum : public KstBinding {
  public:
    KstBindPowerSpectrum(KJS::ExecState *exec, KstPSDPtr d);
    KstBindPowerSpectrum(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindPowerSpectrum();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    void setLength(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value length(KJS::ExecState *exec) const;
    void setRemoveMean(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value removeMean(KJS::ExecState *exec) const;
    void setAverage(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value average(KJS::ExecState *exec) const;
    void setApodize(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value apodize(KJS::ExecState *exec) const;
    void setFrequency(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value frequency(KJS::ExecState *exec) const;
    KJS::Value xVector(KJS::ExecState *exec) const;
    KJS::Value yVector(KJS::ExecState *exec) const;
    void setVUnits(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value vUnits(KJS::ExecState *exec) const;
    void setRUnits(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value rUnits(KJS::ExecState *exec) const;

  protected:
    KstBindPowerSpectrum(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

    KstPSDPtr _d;
};


#endif

// vim: ts=2 sw=2 et
