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

#ifndef MATRIXDIALOG_H
#define MATRIXDIALOG_H

#include "datadialog.h"
#include "datatab.h"

#include "ui_matrixtab.h"

#include <QPointer>

#include "kst_export.h"

#include "datasource.h"

namespace Kst {

class KST_EXPORT MatrixTab : public DataTab, Ui::MatrixTab {
  Q_OBJECT
  public:
    enum MatrixMode { DataMatrix, GeneratedMatrix };

    MatrixTab(QWidget *parent = 0);
    virtual ~MatrixTab();

    MatrixMode matrixMode() const { return _mode; }
    void setMatrixMode(MatrixMode mode) { _mode = mode; }

    //DataMatrix mode methods...
    DataSourcePtr dataSource() const;
    void setDataSource(DataSourcePtr dataSource);

    QString file() const;
    void setFile(const QString &file);

    QString field() const;
    void setField(const QString &field);

    void setFieldList(const QStringList &fieldList);

  private Q_SLOTS:
    void readFromSourceChanged();
    void fileNameChanged(const QString &file);
    void showConfigWidget();

  private:
    MatrixMode _mode;
    DataSourcePtr _dataSource;
};

class KST_EXPORT MatrixDialog : public DataDialog {
  Q_OBJECT
  public:
    MatrixDialog(ObjectPtr dataObject, QWidget *parent = 0);
    virtual ~MatrixDialog();

  protected:
    virtual QString tagName() const;
    virtual ObjectPtr createNewDataObject() const;
    virtual ObjectPtr editExistingDataObject() const;

  private:
    ObjectPtr createNewDataMatrix() const;
    ObjectPtr createNewGeneratedMatrix() const;

  private:
    MatrixTab *_matrixTab;
};

}

#endif

// vim: ts=2 sw=2 et
