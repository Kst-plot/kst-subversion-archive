/***************************************************************************
                                kstbinding.h
                             -------------------
    begin                : Mar 23 2005
    copyright            : (C) 2005 The University of Toronto
                           (C) 2007 The University of British Columbia
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

#ifndef KSTBINDING_H
#define KSTBINDING_H

#include <kjs/object.h>
#include <kjs/interpreter.h>

#include <kst2dplot.h>
#include <kstbasicplugin.h>
#include <kstdatasource.h>
#include <kstvcurve.h>
#include <kstvector.h>
#include <plugin.h>

class KstViewWindow;

#define KST_BINDING_NOCONSTRUCTOR -1
#define KST_BINDING_CONSTRUCTOR   0

class KstBinding : public KJS::ObjectImp {
  public:
    KstBinding(const QString& name, bool hasConstructor = true);
    virtual ~KstBinding();

    QString typeName() const;
    virtual KJS::UString toString(KJS::ExecState *exec) const;

    bool implementsConstruct() const;
    bool implementsCall() const;

    virtual bool inherits(const char*);

    int id() const;

    virtual int methodCount() const;
    virtual int propertyCount() const;

    KstDataSourcePtr extractDataSource(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstDataObjectPtr extractDataObject(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstPluginPtr extractPluginModule(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstBasicPluginPtr extractBasicPluginModule(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstVectorPtr extractVector(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstScalarPtr extractScalar(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstStringPtr extractString(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstVCurvePtr extractVCurve(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstMatrixPtr extractMatrix(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstViewWindow *extractWindow(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    Kst2DPlotPtr extractPlot(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstViewObjectPtr extractViewObject(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;
    KstBaseCurveList extractCurveList(KJS::ExecState*, const KJS::Value&, bool doThrow = true) const;

  protected:
    QString name() const { return _name; }
    KstBinding(const QString& name, int id);

    void addStackInfoFromContext(const KJS::Context& context, QString& error) const;
    void addStackInfo(KJS::ExecState *exec, QString& error) const;

    KJS::Object createTypeError(KJS::ExecState *exec, int argumentNumber) const;
    void createPropertyTypeError(KJS::ExecState *exec) const;
    KJS::Object createSyntaxError(KJS::ExecState *exec) const;
    KJS::Object createGeneralError(KJS::ExecState *exec, const QString& error) const;
    void createPropertyGeneralError(KJS::ExecState *exec, const QString& error) const;
    KJS::Object createInternalError(KJS::ExecState *exec) const;
    void createPropertyInternalError(KJS::ExecState *exec) const;
    KJS::Object createRangeError(KJS::ExecState *exec, int argumentNumber) const;
    void createPropertyRangeError(KJS::ExecState *exec) const;

  private:
    QString _name;
    int _id;
};

#endif
