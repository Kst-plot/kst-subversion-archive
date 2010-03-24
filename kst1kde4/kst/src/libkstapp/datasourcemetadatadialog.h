/***************************************************************************
                      datasourcemetadatadialog.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2010 The University of British Columbia
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

#ifndef DATASOURCEMETADATDATADIALOG_H
#define DATASOURCEMETADATDATADIALOG_H

#include "ui_datasourcemetadatadialog.h"
#include "kstdatasource.h"
#include "kststring.h"

class DataSourceMetaDataDialog : public QDialog, public Ui::DataSourceMetaDataDialog
{
  Q_OBJECT
  public:
    DataSourceMetaDataDialog(QWidget* parent = 0, const char* name = 0,
                         bool modal = FALSE, Qt::WindowFlags fl = 0 );
    virtual ~DataSourceMetaDataDialog();

    void setDataSource(KstDataSourcePtr dsp);
    void updateMetadata(const QString& tag);

  private:
    KstDataSourcePtr _dsp;
};

#endif
