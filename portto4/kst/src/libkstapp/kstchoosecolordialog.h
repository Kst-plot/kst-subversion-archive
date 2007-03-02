/**************************************************************************
        kstchoosecolordialog.h - source file: inherits designer dialog
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

#ifndef KSTCHOOSECOLORDIALOGI_H
#define KSTCHOOSECOLORDIALOGI_H

#include <QDialog>

#include "ui_kstchoosecolordialog4.h"

#include <qlineedit.h>

#include <kcolorcombo.h>

class KstChooseColorDialogI : public QDialog {
  Q_OBJECT
  public:
    KstChooseColorDialogI(QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    virtual ~KstChooseColorDialogI();
    
    void updateChooseColorDialog();
    void showChooseColorDialog();
    
  public slots:
    void applyColors();
    
  private:
    QGridLayout* grid;
    
    // list of the current textfields and colorcombos
    QValueList<QLineEdit*> lineEdits;
    QValueList<KColorCombo*> colorCombos;
    QColor getColorForFile(const QString &fileName);
    
    void cleanColorGroup();
};

#endif
// vim: ts=2 sw=2 et
