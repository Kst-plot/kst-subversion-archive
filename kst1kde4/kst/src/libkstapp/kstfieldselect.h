/***************************************************************************
                       kstfieldselect.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2008 The University of British Columbia
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
#ifndef KSTFIELDSELECTI_H
#define KSTFIELDSELECTI_H

#include "ui_fieldselect.h"
#include "kstdatasource.h"

class KstFieldSelect: public QDialog, public Ui::FieldSelect {
  Q_OBJECT
  public:
    KstFieldSelect(QWidget* parent = 0, const char *name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    virtual ~KstFieldSelect();

    void setURL(const QString& url);
    QString selection( ) { return _selection; }

  signals:

  public slots:

  private slots:
    void OKFieldSelect();
    void CancelFieldSelect();
    void vectorSubset(const QString& filter);
    void search();

  protected:

  private:
    void fillFields();

// xxx    QDict<QListWidgetItem> _fields;
    KstDataSourcePtr _ds;
    QString _url;
    QString _selection;
};

#endif
