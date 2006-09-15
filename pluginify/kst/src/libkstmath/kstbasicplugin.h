/***************************************************************************
                                 kstbasicplugin.h
                             -------------------
    begin                : 09/15/06
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

#ifndef KSTBASICPLUGIN_H
#define KSTBASICPLUGIN_H

#include "kstdataobject.h"
#include "kst_export.h"

class KST_EXPORT KstBasicPlugin : public KstDataObject {
public:
  KstBasicPlugin();
  KstBasicPlugin(const QDomElement &e);
  virtual ~KstBasicPlugin();

  //The implementation of the algorithm the plugin provides.
  //Operates on the inputVectors, inputScalars, and inputStrings
  //to produce the outputVectors, outputScalars, and outputStrings.
  virtual bool algorithm() = 0;

  //String lists of the names of the expected inputs.
  virtual QStringList inputVectors() const = 0;
  virtual QStringList inputScalars() const = 0;
  virtual QStringList inputStrings() const = 0;
  //String lists of the names of the expected outputs.
  virtual QStringList outputVectors() const = 0;
  virtual QStringList outputScalars() const = 0;
  virtual QStringList outputStrings() const = 0;

  //Pure virtual methods inherited from KstDataObject
  //This _must_ equal the 'Name' entry in the .desktop file of
  //the plugin
  virtual QString propertyString() const = 0;

  //Not sure how we'll handle this just yet...
  virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap&) = 0;

  public slots:
  //Pure virtual slots from KstDataObject
  //Each plugin will need to provide an implementation for each
  virtual void showNewDialog() = 0;
  virtual void showEditDialog() = 0;

  public:
  //Returns the respective input object for name
  KstVectorPtr inputVector(const QString& name) const;
  KstScalarPtr inputScalar(const QString& name) const;
  KstStringPtr inputString(const QString& name) const;

  //Returns the respective output object for name
  KstVectorPtr outputVector(const QString& name) const;
  KstScalarPtr outputScalar(const QString& name) const;
  KstStringPtr outputString(const QString& name) const;

  //Pure virtual methods inherited from KstDataObject
  //We do this one ourselves for benefit of all plugins...
  KstObject::UpdateType update(int updateCounter = -1);

  //Regular virtual methods from KstDataObject
  void load(const QDomElement &e);
  void save(QTextStream& ts, const QString& indent = QString::null);

  private:
  bool inputsExist() const;
  bool updateDependentInput(int updateCounter, bool force) const;
};

typedef KstSharedPtr<KstBasicPlugin> KstBasicPluginPtr;
typedef KstObjectList<KstBasicPluginPtr> KstBasicPluginList;

#endif

// vim: ts=2 sw=2 et
