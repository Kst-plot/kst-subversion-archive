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
#include "kstscalartable.h"

class KstViewScalarsDialogI : public KstViewScalarsDialog {
  Q_OBJECT
public:
  KstViewScalarsDialogI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstViewScalarsDialogI();
  KstScalarTable* tableScalars;
  QPushButton* Cancel;

protected:
  QHBoxLayout* layout2;

protected slots:
  virtual void languageChange();

public slots:
  void updateViewScalarsDialog();
  void showViewScalarsDialog();
  void updateDefaults(int index=0);

private slots:

signals:
  /** signal that vectors have changed */
  void docChanged();
};


#endif // KSTVIEWSCALARSDIALOGI_H
// vim: ts=2 sw=2 et
