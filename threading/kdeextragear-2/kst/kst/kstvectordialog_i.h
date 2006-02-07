/***************************************************************************
                      kstvectordialog_i.h  -  Part of KST
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
#ifndef KSTVECTORDIALOGI_H
#define KSTVECTORDIALOGI_H

#include <kcompletion.h>
#include "kstrvector.h"
#include "vectordialog.h"

class KstVectorDialogI : public KstVectorDialog {
  Q_OBJECT
public:
  KstVectorDialogI(QWidget* parent = 0, const char* name = 0,
                   bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstVectorDialogI();

public slots:
  /** update the entries in the vector dialog to represent current vectors */
  void update(int new_index = -1);

  void browseFile();

  /** Calls update(), then shows/raises the dialog */
  void show_I();
  void show_I(const QString &field);
  void show_New();

  void new_I();
  void edit_I();
  void delete_I();

  static KstVectorDialogI *globalInstance();

signals:
  void modified();
  void vectorCreated(KstVectorPtr v);

private:
  static KstVectorDialogI *_inst;
  KCompletion *_fieldCompletion;
};

#endif
