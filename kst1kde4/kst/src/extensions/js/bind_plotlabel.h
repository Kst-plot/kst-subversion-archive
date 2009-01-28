/***************************************************************************
                              bind_plotlabel.h
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

#ifndef BIND_PLOTLABEL_H
#define BIND_PLOTLABEL_H

#include "kstbinding.h"

#include <kjs/interpreter.h>
#include <kjs/object.h>

#include <qguardedptr.h>

#include <kst2dplot.h>

/* @class PlotLabel
   @description A class representing a plot label.
*/
class KstBindPlotLabel : public QObject, public KstBinding {
  public:
    KstBindPlotLabel(KJS::ExecState *exec, Kst2DPlotPtr d);
    ~KstBindPlotLabel();

    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;

    // member functions

    /* @property string text
       @description Contains the text contents of the label.  This may include
                    carriage returns (\n), scalar references of the form
                    <i>[scalar_name]</i>, and some basic LaTeX.
    */
    void setText(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value text(KJS::ExecState *exec) const;

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

    /* @property number justification
       @description The horizontal justification for the label. This is a bit field.
                    The values are as follows:
                    <ul>
                    <li>0 - Justify None</li>
                    <li>1 - Justify Left / Top</li>
                    <li>2 - Justify Right / Bottom</li>
                    <li>3 - Justify Center</li>
                    </ul>
    */
    void setJustification(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value justification(KJS::ExecState *exec) const;

    /* @property number dataPrecision
       @description Contains the number of digits of precision to display data
                    values in the label.  Restricted to a value between 0 and
                    16 inclusive.
    */
    void setDataPrecision(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value dataPrecision(KJS::ExecState *exec) const;

  protected:
    KstBindPlotLabel(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    QGuardedPtr<Kst2DPlot> _d;
};


#endif

