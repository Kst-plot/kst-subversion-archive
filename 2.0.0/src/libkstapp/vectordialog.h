/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VECTORDIALOG_H
#define VECTORDIALOG_H

#include "datadialog.h"
#include "datatab.h"

#include "ui_vectortab.h"

#include <QPointer>

#include "kst_export.h"

#include "datasource.h"

namespace Kst {

class ObjectStore;

class VectorTab : public DataTab, Ui::VectorTab {
  Q_OBJECT

  public:
    enum VectorMode { DataVector, GeneratedVector };

    VectorTab(ObjectStore *store, QWidget *parent = 0);
    virtual ~VectorTab();

    VectorMode vectorMode() const { return _mode; }
    void setVectorMode(VectorMode mode);

    //DataVector mode methods...
    DataSourcePtr dataSource() const;
    void setDataSource(DataSourcePtr dataSource);

    QString file() const;
    void setFile(const QString &file);

    QString field() const;
    void setField(const QString &field);

    void setFieldList(const QStringList &fieldList);

    DataRange *dataRange() const;

    //GeneratedVector methods...
    qreal from() const;
    void setFrom(qreal from);
    bool fromDirty() const;

    qreal to() const;
    void setTo(qreal to);
    bool toDirty() const;

    int numberOfSamples() const;
    void setNumberOfSamples(int numberOfSamples);
    bool numberOfSamplesDirty() const;

    void hideGeneratedOptions();
    void hideDataOptions();
    void enableSingleEditOptions(bool enabled);
    void clearTabValues();

  Q_SIGNALS:
    void sourceChanged();
    void fieldChanged();

  private Q_SLOTS:
    void readFromSourceClicked();
    void generateClicked();
    void fileNameChanged(const QString &file);
    void showConfigWidget();
    void sourceValid(QString filename, int requestID);

  private:
    VectorMode _mode;
    ObjectStore *_store;
    DataSourcePtr _dataSource;
    QString _initField;
    int _requestID;
};

class VectorDialog : public DataDialog {
  Q_OBJECT
  public:
    VectorDialog(ObjectPtr dataObject, QWidget *parent = 0);
    virtual ~VectorDialog();

    void setField(QString field) {_vectorTab->setField(field);}

  protected:
//     virtual QString tagString() const;
    virtual ObjectPtr createNewDataObject();
    virtual ObjectPtr editExistingDataObject() const;

  private:
    ObjectPtr createNewDataVector();
    ObjectPtr createNewGeneratedVector();
    void configureTab(ObjectPtr vector=0);

  private Q_SLOTS:
    void updateButtons();
    void editMultipleMode();
    void editSingleMode();

  private:
    VectorTab *_vectorTab;

};

}

#endif

// vim: ts=2 sw=2 et
