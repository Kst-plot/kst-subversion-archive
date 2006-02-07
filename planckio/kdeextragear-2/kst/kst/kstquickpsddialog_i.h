/***************************************************************************
                       kstquickpsddialog_i.h  -  Part of KST
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
#ifndef KSTQUICKPSDDIALOGI_H
#define KSTQUICKPSDDIALOGI_H

#include "quickpsddialog.h"
#include "kstpsdcurve.h"

class KstQuickPSDDialogI : public KstQuickPSDDialog {
    Q_OBJECT
public:
    KstQuickPSDDialogI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstQuickPSDDialogI();

public slots:
  /** update the entries in quickPSDDialog to represent current vectors */
  void update();

  /** calls update(), then shows/raises quickPSDDialog */
  void showQuickPSDDialog();

  /* set the internal variable for the curve color */
  void setCurveColor(const QColor &c);
  QColor curveColor();

  /* draw sample of the curve line */
  void drawCurveLine();

  void changeColor();

  void addPlot();
private:
  QColor CurveColor;
private slots:
  void apply() { apply(false); }
  void apply(bool autolabel);
  void updateActiveEntry(int);
signals:
  /** signal that things have changed */
  void docChanged();
};

#endif
