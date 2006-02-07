/***************************************************************************
                       ksteqdialog_i.h  -  Part of KST
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
#ifndef KSTEQDIALOGI_H
#define KSTEQDIALOGI_H

#include "eqdialog.h"
#include "kstequationcurve.h"

class KstEqDialogI : public KstEqDialog {
  Q_OBJECT
public:
  KstEqDialogI(QWidget* parent = 0, const char* name = 0,
               bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstEqDialogI();
public slots:
  /** update the entries in the eq dialog to represent current eqs */
  void update(int new_index = -1);

  /** Calls update(), then shows/raises the dialog */
  void show_I();
  void show_I(const QString &field);
  void show_New();

  void new_I();
  void edit_I();
  void delete_I();

  static KstEqDialogI *globalInstance();

private:
  static KstEqDialogI *_inst;
};

#endif
