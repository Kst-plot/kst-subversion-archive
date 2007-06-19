/***************************************************************************
                       kst2dplotwidget_i.h  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2007 The University of British Columbia
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
#ifndef KST2DPLOTWIDGETI_H
#define KST2DPLOTWIDGETI_H

#include "view2dplotwidget.h"

class Kst2dPlotWidget: public View2DPlotWidget {
  Q_OBJECT

  public:
    Kst2dPlotWidget(QWidget* parent = 0, const char *name = 0, WFlags fl = 0);
    virtual ~Kst2dPlotWidget();

  public:
    void populateEditMultiple(const Kst2DPlot *plot);

  signals:
    void changed();

  public slots:
    void generateDefaultLabels();
    void updateButtons();
    void addDisplayedCurve();
    void removeDisplayedCurve();
    void fillMarkerLineCombo();
    void updateAxesButtons();
    void updateScalarCombo();
    void updatePlotMarkers( const Kst2DPlot * plot );
    void fillWidget( const Kst2DPlot * plot );
    void applyContents( Kst2DPlotPtr plot );
    void applyAppearance( Kst2DPlotPtr plot );
    void applyXAxis( Kst2DPlotPtr plot );
    void applyYAxis( Kst2DPlotPtr plot );
    void applyRange( Kst2DPlotPtr plot );
    void addPlotMarker();
    void removePlotMarker();
    void removeAllPlotMarkers();
    void applyPlotMarkers( Kst2DPlotPtr plot );
    void fillPlot( Kst2DPlotPtr plot );
    void insertCurrentScalar();
    void setScalarDestXLabel();
    void setScalarDestYLabel();
    void setScalarDestTopLabel();
    void editLegend();

  private:
    void populateEditMultiple(QCheckBox *checkBoxWidget);

    Kst2DPlotPtr _plot;
    QLineEdit *_scalarDest;
    bool _editMultipleMode;

  protected:
};

#endif
