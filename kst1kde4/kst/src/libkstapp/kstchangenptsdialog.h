/***************************************************************************
                       kstchangenptsdialog.h  -  Part of KST
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

#ifndef KSTCHANGENPTSDIALOGI_H
#define KSTCHANGENPTSDIALOGI_H

#include "ui_changenptsdialog.h"

class KstChangeNptsDialog : public QDialog, public Ui::KstChangeNptsDialog {
    Q_OBJECT
public:
    KstChangeNptsDialog(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, Qt::WindowFlags fl = 0 );
    virtual ~KstChangeNptsDialog();

public slots:
  bool updateChangeNptsDialog();
  void showChangeNptsDialog();
  void selectNone();
  void selectAll();
  void updateDefaults(int index=0);

private slots:
  void emitDocChanged();
  void applyNptsChange();
  void OKNptsChange();
  void updateTimeCombo();
  void modifiedRange();
  void changedSelection();

signals:
  void docChanged();

private:
  bool _modifiedRange;
};

#endif
