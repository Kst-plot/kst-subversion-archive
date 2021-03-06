/***************************************************************************
                       kstviewvectorsdialog_i.h  -  Part of KST
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

#include "viewvectorsdialog.h"
#include "kstvectortable.h"

class KstViewVectorsDialogI : public KstViewVectorsDialog {
  Q_OBJECT
  public:
    KstViewVectorsDialogI(QWidget* parent = 0,
                        const char* name = 0,
                        bool modal = false, WFlags fl = 0 );
    virtual ~KstViewVectorsDialogI();
    KstVectorTable* tableVectors;

    bool hasContent() const;

  public slots:
    void updateViewVectorsDialog();
    void updateViewVectorsDialog(const QString& strVector);
    void showViewVectorsDialog();
    void updateDefaults(int index = 0);

  protected slots:
    virtual void languageChange();  
    virtual void vectorChanged(const QString& str);
};

#endif
