/***************************************************************************
                   testplugin.cpp
                             -------------------
    begin                : 09/04/06
    copyright            : (C) 2006 The University of Toronto
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
#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#include <kstbasicplugin.h>

class TestPlugin : public KstBasicPlugin {
  Q_OBJECT
  public:
    TestPlugin(QObject *parent, const char *name, const QStringList &args);
    virtual ~TestPlugin();

    virtual bool algorithm();

    virtual QStringList inputVectors() const;
    virtual QStringList inputScalars() const;
    virtual QStringList inputStrings() const;
    virtual QStringList outputVectors() const;
    virtual QStringList outputScalars() const;
    virtual QStringList outputStrings() const;

    virtual QString propertyString() const { return "Test Plugin"; }

    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap&) { return 0; }
};

#endif
