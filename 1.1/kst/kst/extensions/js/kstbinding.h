/***************************************************************************
                                kstbinding.h
                             -------------------
    begin                : Mar 23 2005
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

#ifndef KSTBINDING_H
#define KSTBINDING_H

#include <kjsembed/jsbinding.h>
#include <kjsembed/jsproxy.h>

#include <kstdatasource.h>
#include <kstvcurve.h>
#include <kstvector.h>

class KstViewWindow;

#define KST_BINDING_NOCONSTRUCTOR -1
#define KST_BINDING_CONSTRUCTOR   0

class KstBinding : public KJSEmbed::JSProxy {
  public:
    KstBinding(const QString& name, bool hasConstructor = true);
    virtual ~KstBinding();

    QString typeName() const;
    virtual KJS::UString toString(KJS::ExecState *exec) const;

    bool implementsConstruct() const;
    bool implementsCall() const;

    virtual bool inherits(const char*);

    int id() const;

    static KstDataSourcePtr extractDataSource(KJS::ExecState*, const KJS::Value&, bool doThrow = true);
    static KstVectorPtr extractVector(KJS::ExecState*, const KJS::Value&, bool doThrow = true);
    static KstVCurvePtr extractVCurve(KJS::ExecState*, const KJS::Value&, bool doThrow = true);
    static KstViewWindow *extractWindow(KJS::ExecState*, const KJS::Value&, bool doThrow = true);

  protected:
    KstBinding(const QString& name, int id);

  private:
    QString _name;
    int _id;
};


#endif

// vim: ts=2 sw=2 et
