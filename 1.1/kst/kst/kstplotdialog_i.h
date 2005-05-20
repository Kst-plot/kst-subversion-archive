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

class KstDoc;
class KstLabel;
class KMdiChildView;

#define CONTENT_TAB   0
#define LABELS_TAB    1
#define LEGEND_TAB    1
#define AXES_TAB      2
#define LIMITS_TAB    3
#define MARKERS_TAB   4

class KstPlotDialogI : public KstPlotDialog {
  Q_OBJECT
  public:
    KstPlotDialogI(KstDoc *doc, QWidget *parent = 0, const char *name = 0,
                   bool modal = false, WFlags fl = 0);
    virtual ~KstPlotDialogI();

  public slots:
    /** Update info in the plot dialog */
    void updateWindow();
    void update(int new_index = -1);
    void newTab();

    /** Calls update(), then shows/raises the dialog */
    void show_I(const QString& strWindow, const QString& strPlot);
    void show_I();
    void new_I();
    void edit_I();
    void delete_I();

    void updateCurveLists();
    void updateScalarCombo();
    void updateWindowList();
    void updatePlotList();
    void updatePlotMarkers();
    void updateGridSettings();
    void changeWindow();

    void updateButtons();
    void updateAxesButtons();
    void updateXTimezoneButtons();
    void updateYTimezoneButtons();
    
    void setScalarDestTopLabel() { ScalarDest = TopLabelText; }
    void setScalarDestXLabel() { ScalarDest = XAxisText; }
    void setScalarDestYLabel() { ScalarDest = YAxisText; }
    void insertCurrentScalar();

    void applyAppearance(Kst2DPlotPtr plot);
    void applyAxes(Kst2DPlotPtr plot);
    void applyRange(Kst2DPlotPtr plot);
    void applyPlotMarkers(Kst2DPlotPtr plot);

    void applyAutoLabels();

    void removeDisplayedCurve();
    void addDisplayedCurve();
    void removeDisplayedCurve(QListBoxItem*);
    void addDisplayedCurve(QListBoxItem*);

    /** plot markers **/
    void addPlotMarker();
    void removePlotMarker();
    void removeAllPlotMarkers();

    void newWindow();

  signals:
    void docChanged();
  private:
    void applySettings(KMdiChildView *c, Kst2DPlotPtr plot);
    void fill2DPlotList(Kst2DPlotList& plots);
    bool checkPlotName();
    void setMajorSpacing( QComboBox* majorTicks, int spacing);

    KstDoc *doc;
    KstLabelPtr SampleLabel;
    QLineEdit *ScalarDest;
    QString _windowName;
    QString _plotName;
};

#endif
// vim: ts=2 sw=2 et
