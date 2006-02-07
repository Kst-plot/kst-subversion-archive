/***************************************************************************
                       kstplotdialog_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2000-2002 C. Barth Netterfield
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
#ifndef KSTPLOTEDITI_H
#define KSTPLOTEDITI_H

#include "plotdialog.h"
#include "kstdoc.h"
#include <kcombobox.h>

#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qlabel.h>

#include "kstbasecurve.h"

#define LABELS_TAB 2
#define LIMITS_TAB 1

class KstPlotDialogI : public KstPlotDialog {
    Q_OBJECT
public:
    KstPlotDialogI(KstDoc *doc, QWidget* parent = 0, const char* name = 0,
                   bool modal = FALSE, WFlags fl = 0);
    virtual ~KstPlotDialogI();

public slots:
  /** Update info in the plot dialog */
  void update(int new_index = -1);

  /** Calls update(), then shows/raises the dialog */
  void show_I();
  void new_I();
  void edit_I();
  void delete_I();

  /** update sample labels */
  void updateSampNumLabel();
  void updateSampTopLabel();
  void updateSampXLabel();
  void updateSampYLabel();
  void updateLabels();

  void setScalarDestTopLabel() {ScalarDest = TopLabelText;}
  void setScalarDestXLabel() {ScalarDest = XAxisText;}
  void setScalarDestYLabel() {ScalarDest = YAxisText;}
  void insertCurrentScalar() {ScalarDest->insert(ScalarList->currentText());}

  void applyLabels(KstPlot *plot);
  void applyLimits(KstPlot *plot);

  void applyAutoLabels();

  void changePosition(int ci);

  void removeDisplayedCurve();
  void addDisplayedCurve();

  void updateButtons();
signals:
  void docChanged();
private:
  KstDoc *doc;
  KstLabel *SampleLabel;
  QLineEdit *ScalarDest;
};

#endif
