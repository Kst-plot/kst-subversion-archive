/***************************************************************************
                    kstquickcurvesdialog_i.h  -  Part of KST
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
#ifndef KSTQUICKCURVESDIALOGI_H
#define KSTQUICKCURVESDIALOGI_H

#include <kcompletion.h>
#include "quickcurvesdialog.h"
#include "kstvcurve.h"

class KstQuickCurvesDialogI : public KstQuickCurvesDialog {
    Q_OBJECT
public:
    KstQuickCurvesDialogI(QWidget* parent = 0,
                         const char* name = 0,
                         bool modal = FALSE, WFlags fl = 0 );
    virtual ~KstQuickCurvesDialogI();

public slots:
  /** update the entries in quickCurvesDialog to represent current vectors */
  void update();

  /** calls updateQuickCurvesDialog(), then shows/raises quickCurvesDialog */
  void showQuickCurvesDialog();

  /* draw sample of the curve line */
  void drawCurveLine();

  bool addPlot();
private slots:
  bool apply(bool autolabel);
  void apply() { apply(false); }

signals:
  /** signal that things have changed */
  void docChanged();

private:
  KCompletion *_xaxisCompletion;
  KCompletion *_yaxisCompletion;

};

#endif
