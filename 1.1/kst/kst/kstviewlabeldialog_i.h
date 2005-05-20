/**************************************************************************
        kstviewlabeldialog_i.h - source file: inherits designer dialog
                             -------------------
    begin                :  2004
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

#ifndef KSTVIEWLABELDIALOGI_H
#define KSTVIEWLABELDIALOGI_H

#include "kstviewlabel.h"
#include "kstviewlabeldialog.h"

class KstViewLabelDialogI : public KstViewLabelDialog {
  Q_OBJECT
  public:
    KstViewLabelDialogI(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstViewLabelDialogI();

    void showI(KstViewLabel* pViewLabel);
    void updateI();
  public slots:
    void apply();
    void ok();
    void deleteL();
  private:
    KstViewLabel* _pViewLabel;
    double _x;
    double _y;
    int _i_plot;
    int _i_label;
    bool _editing;

    void applyAsNew();
    void applyEdits();
  signals:
    void applied();
};

#endif
// vim: ts=2 sw=2 et
