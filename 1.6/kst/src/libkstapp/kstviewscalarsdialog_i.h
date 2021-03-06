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

#include <klistviewsearchline.h>

#include "viewscalarsdialog.h"
//#include "kstscalartable.h"
#include "kstscalarlistview.h"

class KstViewScalarsDialogI : public KstViewScalarsDialog {
  Q_OBJECT
  public:
    KstViewScalarsDialogI(QWidget* parent = 0,
        const char* name = 0,
        bool modal = false, WFlags fl = 0 );
    virtual ~KstViewScalarsDialogI();
    KListViewSearchLineWidget *searchWidget;
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
