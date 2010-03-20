/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTCHOOSECOLORDIALOG_H
#define KSTCHOOSECOLORDIALOG_H

#include <qcolor.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kcolorcombo.h>

#include "ui_choosecolordialog.h"
#include "kstvcurve.h"

class KST_EXPORT KstChooseColorDialog : public QDialog, public Ui::KstChooseColorDialog {
Q_OBJECT
public:
  KstChooseColorDialog(QWidget* parent = 0,
                        const char* name = 0,
                        bool modal = FALSE, WFlags fl = 0 );
  virtual ~KstChooseColorDialog();

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

