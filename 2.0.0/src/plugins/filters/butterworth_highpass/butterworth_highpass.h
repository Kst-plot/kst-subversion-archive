/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2008 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BUTTERWORTHHIGHPASSPLUGIN_H
#define BUTTERWORTHHIGHPASSPLUGIN_H

#include <QFile>

#include <basicplugin.h>
#include <dataobjectplugin.h>

class FilterButterworthHighPassSource : public Kst::BasicPlugin {
  Q_OBJECT

  public:
    virtual QString _automaticDescriptiveName() const;

    Kst::VectorPtr vector() const;
    Kst::ScalarPtr orderScalar() const;
    Kst::ScalarPtr cutoffScalar() const;

    virtual void change(Kst::DataObjectConfigWidget *configWidget);

    void setupOutputs();
    virtual bool algorithm();

    virtual QStringList inputVectorList() const;
    virtual QStringList inputScalarList() const;
    virtual QStringList inputStringList() const;
    virtual QStringList outputVectorList() const;
    virtual QStringList outputScalarList() const;
    virtual QStringList outputStringList() const;

    virtual void saveProperties(QXmlStreamWriter &s);

  protected:
    FilterButterworthHighPassSource(Kst::ObjectStore *store);
    ~FilterButterworthHighPassSource();

  friend class Kst::ObjectStore;


};


class ButterworthHighPassPlugin : public QObject, public Kst::DataObjectPluginInterface {
    Q_OBJECT
    Q_INTERFACES(Kst::DataObjectPluginInterface)
  public:
    virtual ~ButterworthHighPassPlugin() {}

    virtual QString pluginName() const;
    virtual QString pluginDescription() const;

    virtual DataObjectPluginInterface::PluginTypeID pluginType() const { return Filter; }

    virtual bool hasConfigWidget() const { return true; }

    virtual Kst::DataObject *create(Kst::ObjectStore *store, Kst::DataObjectConfigWidget *configWidget, bool setupInputsOutputs = true) const;

    virtual Kst::DataObjectConfigWidget *configWidget(QSettings *settingsObject) const;
};

#endif
// vim: ts=2 sw=2 et
