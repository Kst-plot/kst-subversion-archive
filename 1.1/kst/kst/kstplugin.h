/***************************************************************************
                                 kstplugin.h
                             -------------------
    begin                : May 15 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef KSTPLUGIN_H
#define KSTPLUGIN_H

#include "kstdataobject.h"
#include "plugin.h"


/*  Usage notes:
 *   - Output vectors are created internally, but may be renamed.
 *   - Input vectors must be set before setPlugin(), else it will fail.
 */

class KstPlugin : public KstDataObject {
  public:
    KstPlugin();
    KstPlugin(const QDomElement &e);
    virtual ~KstPlugin();

    virtual UpdateType update(int update_counter);

    virtual void save(QTextStream &ts, const QString& indent = QString::null);

    virtual bool slaveVectorsUsed() const;
    virtual bool isValid() const;

    virtual bool setPlugin(KstSharedPtr<Plugin> plugin);
    KstSharedPtr<Plugin> plugin() const;

    virtual QString propertyString() const;

    virtual const KstCurveHintList* curveHints() const;

    QString lastError() const;

    void createFitScalars();

  protected:
    static void countScalarsAndVectors(const QValueList<Plugin::Data::IOValue>& table, unsigned& scalars, unsigned& vectors);
    virtual void _showDialog();
    KstSharedPtr<Plugin> _plugin;
    unsigned _inScalarCnt, _inArrayCnt, _inStringCnt, _outScalarCnt;
    unsigned _inPid, _outArrayCnt, _outStringCnt;
    void *_localData;
    QString _lastError;

    int *_inArrayLens, *_outArrayLens;
    double *_inScalars, *_outScalars;
    double **_inVectors, **_outVectors;
    char **_inStrings;
    char **_outStrings;

  private:
    void commonConstructor();
    void allocateParameters();
    void freeParameters();
};

typedef KstSharedPtr<KstPlugin> KstPluginPtr;
typedef KstObjectList<KstPluginPtr> KstPluginList;

#endif

// vim: ts=2 sw=2 et
