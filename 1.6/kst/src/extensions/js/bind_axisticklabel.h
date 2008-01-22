/***************************************************************************
                              bind_axisticklabel.h
                             -------------------
    begin                : Jul 25 2007
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

#ifndef BIND_AXISTICKLABEL_H
#define BIND_AXISTICKLABEL_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

#include <qguardedptr.h>

#include <kst2dplot.h>

/* @class AxisTickLabel
   @description A class representing a plot axis tick label.
*/
class KstBindAxisTickLabel : public QObject, public KstBinding {
  public:
    KstBindAxisTickLabel(KJS::ExecState *exec, QGuardedPtr<Kst2DPlot> d, bool isX);
    ~KstBindAxisTickLabel();

    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    /* @property string font
       @description Used to set or get the current font used for the label.
    */
    void setFont(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value font(KJS::ExecState *exec) const;

    /* @property number fontSize
       @description Contains the size of the font used to draw the label.
    */
    void setFontSize(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value fontSize(KJS::ExecState *exec) const;

    /* @property number rotation
       @description Contains the rotation angle of the label.  Rotation is
                    clockwise from <i>normal</i> and is stored in degrees.
    */
    void setRotation(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value rotation(KJS::ExecState *exec) const;

    /* @property string type
       @readonly
       @description The type of axis - X or Y presently.
    */
    KJS::Value type(KJS::ExecState *exec) const;

  protected:
    KstBindAxisTickLabel(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    QGuardedPtr<Kst2DPlot> _d;
    bool _xAxis;
};


#endif

