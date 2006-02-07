/**************************************************************************
        kstlabeldialog_i.h - source file: inherits designer dialog
                             -------------------
    begin                :  2003
    copyright            : (C) 2003 by Barth Netterfield
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

#ifndef KSTLABELDIALOGI_H
#define KSTLABELDIALOGI_H

#include "kstlabeldialog.h"
#include "kstplot.h"

class KstLabelDialogI : public KstLabelDialog {
    Q_OBJECT
public:
    KstLabelDialogI(QWidget* parent = 0,
                    const char* name = 0,
                    bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstLabelDialogI();
  void showI(int i_plot, int i_label, double in_x, double in_y);
  void updateI();
public slots:
  void apply();
  void ok();
  void deleteL();
private:
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
