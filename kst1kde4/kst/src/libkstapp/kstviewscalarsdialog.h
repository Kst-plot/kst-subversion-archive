/***************************************************************************
                       kstviewscalarsdialog_i.h  -  Part of KST
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

#include "ui_viewscalarsdialog.h"
//#include "kstscalartable.h"
#include "kstscalarlistview.h"

class KstViewScalarsDialog : public QDialog, public Ui::KstViewScalarsDialog {
  Q_OBJECT
  public:
    KstViewScalarsDialog(QWidget* parent = 0, const char* name = 0,
                          bool modal = false, Qt::WindowFlags fl = 0 );
    virtual ~KstViewScalarsDialog();

    bool hasContent() const;

    KListViewSearchLineWidget *_searchWidget;
    KstScalarListView *_listViewScalars;

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
