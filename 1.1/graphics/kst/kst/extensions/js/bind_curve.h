/***************************************************************************
                                bind_curve.h
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

#ifndef BIND_CURVE_H
#define BIND_CURVE_H

#include "kstbinding.h"

#include <kstvcurve.h>

#include <kjs/interpreter.h>
#include <kjs/object.h>

using namespace KJSEmbed;

class KstBindCurve : public KstBinding {
  public:
    KstBindCurve(KJS::ExecState *exec, KstVCurvePtr d);
    KstBindCurve(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindCurve();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions
    KJS::Value point(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value xErrorPoint(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value yErrorPoint(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value xMinusErrorPoint(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value yMinusErrorPoint(KJS::ExecState *exec, const KJS::List& args);

    // properties
    void setTagName(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value tagName(KJS::ExecState *exec) const;
    void setColor(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value color(KJS::ExecState *exec) const;
    void setXVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xVector(KJS::ExecState *exec) const;
    void setYVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yVector(KJS::ExecState *exec) const;
    void setXErrorVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xErrorVector(KJS::ExecState *exec) const;
    void setYErrorVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yErrorVector(KJS::ExecState *exec) const;
    void setXMinusErrorVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value xMinusErrorVector(KJS::ExecState *exec) const;
    void setYMinusErrorVector(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value yMinusErrorVector(KJS::ExecState *exec) const;
    KJS::Value samplesPerFrame(KJS::ExecState *exec) const;
    void setIgnoreAutoScale(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value ignoreAutoScale(KJS::ExecState *exec) const;
    void setHasPoints(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value hasPoints(KJS::ExecState *exec) const;
    void setHasLines(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value hasLines(KJS::ExecState *exec) const;
    void setHasBars(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value hasBars(KJS::ExecState *exec) const;
    void setLineWidth(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value lineWidth(KJS::ExecState *exec) const;
    void setLineStyle(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value lineStyle(KJS::ExecState *exec) const;
    void setBarStyle(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value barStyle(KJS::ExecState *exec) const;
    void setPointDensity(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value pointDensity(KJS::ExecState *exec) const;
    KJS::Value topLabel(KJS::ExecState *exec) const;
    KJS::Value xLabel(KJS::ExecState *exec) const;
    KJS::Value yLabel(KJS::ExecState *exec) const;

  protected:
    KstBindCurve(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);

  public:
    KstVCurvePtr _d;
};


#endif

// vim: ts=2 sw=2 et
