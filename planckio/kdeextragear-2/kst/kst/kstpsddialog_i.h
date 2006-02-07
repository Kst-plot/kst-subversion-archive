/***************************************************************************
                       kstpsddialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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

#ifndef KSTPSDDIALOGI_H
#define KSTPSDDIALOGI_H

#include "psddialog.h"
#include "kstpsdcurve.h"

class KstPsdDialogI : public KstPsdDialog {
  Q_OBJECT
public:
  KstPsdDialogI(QWidget* parent = 0, const char* name = 0,
                bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstPsdDialogI();
public slots:
  /** update the entries in the psd dialog to represent current psds */
  void update(int new_index = -1);

  /** Calls update(), then shows/raises the dialog */
  void show_I();
  void show_I(const QString &field);
  void show_New();

  void new_I();
  void edit_I();
  void delete_I();

  static KstPsdDialogI *globalInstance();
private:
  static KstPsdDialogI *_inst;
};

#endif
