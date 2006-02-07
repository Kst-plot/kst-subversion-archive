/***************************************************************************
                       kstviewscalarsdialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2004 The University of British Columbia
                           (C) 2004 Andrew R. Walker
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

#include "viewscalarsdialog.h"

class KstViewScalarsDialogI : public KstViewScalarsDialog {
    Q_OBJECT
public:
    KstViewScalarsDialogI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstViewScalarsDialogI();

public slots:
  /** update the entries in changenptsDialog to represent current vectors */
  void updateViewScalarsDialog();

  /** calls updateChangeNptsDialog(), then shows and raises changeNptsDialog */
  void showViewScalarsDialog();

  /** Update fields based on vector at index */
  void updateDefaults(int index=0);

private slots:

signals:
  /** signal that vectors have changed */
  void docChanged();
};


#endif // KSTVIEWSCALARSDIALOGI_H
