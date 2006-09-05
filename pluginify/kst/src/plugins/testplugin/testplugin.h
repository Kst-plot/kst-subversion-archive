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

#include <kstdataobject.h>

class TestPlugin : public KstDataObject {
  Q_OBJECT
public:

    TestPlugin(QObject *parent, const char *name, const QStringList &args);
    virtual ~TestPlugin();

    virtual QString name() const { return _name; }
    virtual QString xmlFile() const { return _xmlFile; }

    virtual KstObject::UpdateType update(int)
    {
        return UPDATE;
    }
    virtual QString propertyString() const
    {
        return "";
    }
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap&)
    {
        return 0;
    }

protected slots:
    virtual void _showDialog()
    {
    }

private:
    QString _name;
    QString _xmlFile;
};

#endif
