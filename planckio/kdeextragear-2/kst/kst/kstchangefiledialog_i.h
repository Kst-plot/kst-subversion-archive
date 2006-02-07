/**************************************************************************
        kstchangefiledialog_i.h - source file: inherits designer dialog
                             -------------------
    begin                :  2001
    copyright            : (C) 2000-2003 by Barth Netterfield
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

#ifndef KSTCHANGEFILEDIALOGI_H
#define KSTCHANGEFILEDIALOGI_H

#include "changefiledialog.h"

#include <qlistbox.h>
#include <qlabel.h>

class KstChangeFileDialogI : public KstChangeFileDialog {
    Q_OBJECT
public:
    KstChangeFileDialogI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstChangeFileDialogI();

public slots:
  /** update the entries in changeFileDialog to represent current vectors */
  void updateChangeFileDialog();

  /** calls updateChangeFileDialog(), then shows and raises changeFileDialog */
  void showChangeFileDialog();

  /** select all in selection box */
  void selectAll();

private slots:
  void browseFile();
  void applyFileChange();

signals:
  /** signal that vectors have changed */
  void docChanged();
};

#endif
