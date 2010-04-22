/***************************************************************************
                                 kstcplugin.h
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
#include "kst_export.h"


/*  Usage notes:
 *   - Output vectors are created internally, but may be renamed.
 *   - Input vectors must be set before setPlugin(), else it will fail.
 */

class KST_EXPORT KstCPlugin : public KstDataObject {
  public:
    KstCPlugin();
    KstCPlugin(const QDomElement &e);
    virtual ~KstCPlugin();

    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual bool slaveVectorsUsed() const;
    virtual bool isValid() const;
    virtual bool validate();
    virtual bool setModule(KstPluginPtr plugin);
    virtual bool setPlugin(KstPluginPtr plugin);
    virtual void setTagName(const QString& tag);
    virtual void setTagName(const KstObjectTag& tag);
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap);
    virtual UpdateType update(int update_counter);
    virtual QString propertyString() const;
    virtual Kind kind() const;

    virtual const KstCurveHintList* curveHints() const;

    KstPluginPtr plugin() const;
    QString label(int precision) const;
    QString lastError() const;
    void createFitScalars();
 
  protected:
    static void countScalarsAndVectors(const QList<Plugin::Data::IOValue>& table, unsigned& scalars, unsigned& vectors);

    virtual void showNewDialog();
    virtual void showEditDialog();

    KstPluginPtr _plugin;
    QString _lastError;
    unsigned _inScalarCnt;
    unsigned _inArrayCnt;
    unsigned _inStringCnt;
    unsigned _inPid;
    unsigned _outScalarCnt;
    unsigned _outArrayCnt;
    unsigned _outStringCnt;
    void *_localData;

    int *_inArrayLens;
    int *_outArrayLens;
    double *_inScalars;
    double *_outScalars;
    double **_inVectors;
    double **_outVectors;
    char **_inStrings;
    char **_outStrings;

  private:
    void commonConstructor();
    void allocateParameters();
    void freeParameters();
};

typedef QExplicitlySharedDataPointer<KstCPlugin> KstCPluginPtr;
typedef KstObjectList<KstCPluginPtr> KstCPluginList;

#endif
