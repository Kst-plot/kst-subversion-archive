/***************************************************************************
                       kstviewvectorsdialog.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
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

#ifndef KSTVIEWVECTORSDIALOGI_H
#define KSTVIEWVECTORSDIALOGI_H

#include "ui_viewvectorsdialog.h"
#include "kstvectortable.h"

class KstViewVectorsDialog : public QDialog, public Ui::KstViewVectorsDialog {
  Q_OBJECT
  public:
    KstViewVectorsDialog(QWidget* parent = 0,
                        const char* name = 0,
                        bool modal = false, Qt::WindowFlags fl = 0 );
    virtual ~KstViewVectorsDialog();
    KstVectorTable* tableVectors;

    bool hasContent() const;

  public slots:
    void updateViewVectorsDialog();
    void updateViewVectorsDialog(const QString& strVector);
    void showViewVectorsDialog();
    void showViewVectorsDialog(const QString& strVector);
    void updateDefaults(int index = 0);

  protected slots:
    virtual void languageChange();
    virtual void vectorChanged(const QString& str);
};

#endif
