/**************************************************************************
        kstchoosecolordialog_i.h - source file: inherits designer dialog
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

#include <qcolor.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kcolorcombo.h>

#include "choosecolordialog.h"
#include "kstvcurve.h"

class KstChooseColorDialogI : public KstChooseColorDialog {
Q_OBJECT
public:
  KstChooseColorDialogI(QWidget* parent = 0,
                        const char* name = 0,
                        bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstChooseColorDialogI();

  void updateChooseColorDialog();
  void showChooseColorDialog();
  QColor getColorForCurve(const KstVCurvePtr curve);
  QColor getColorForCurve(const KstVectorPtr xVector, const KstVectorPtr yVector);
  bool override();

public slots:
  void applyColors();

private:
  QColor getColorForFile(const QString &fileName);
  void cleanColorGroup();

  QGridLayout* _grid;
  QValueList<QLineEdit*> _lineEdits;
  QValueList<KColorCombo*> _colorCombos;
  QMap<QString, QColor> _fileColors;
  bool _xSelected;
  bool _override;
};

#endif

