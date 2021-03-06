/***************************************************************************
                       kstviewscalarsdialog.h  -  Part of KST
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

#ifndef KSTVIEWSCALARSDIALOGI_H
#define KSTVIEWSCALARSDIALOGI_H

#include <QDialog>

#include "ui_kstviewscalarsdialog4.h"

#include <k3listviewsearchline.h>

//#include "kstscalartable.h"
#include "kstscalarlistview.h"

class KstViewScalarsDialogI : public QDialog, public Ui::KstViewScalarsDialog {
  Q_OBJECT
  public:
    KstViewScalarsDialogI(QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    virtual ~KstViewScalarsDialogI();
    K3ListViewSearchLineWidget *searchWidget;
    KstScalarListView *listViewScalars;

    bool hasContent() const;

  protected slots:
    virtual void languageChange();

  public slots:
    void updateViewScalarsDialog();
    void showViewScalarsDialog();
    void updateDefaults(int index = 0);

  signals:
    /** signal that vectors have changed */
    void docChanged();
};

#endif
// vim: ts=2 sw=2 et
