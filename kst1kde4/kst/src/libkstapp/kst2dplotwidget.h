/***************************************************************************
                       kst2dplotwidget.h  -  Part of KST
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

#include "ui_view2dplotwidget.h"

class Kst2dPlotWidget: public QWidget, public Ui::View2DPlotWidget {
  Q_OBJECT

  public:
    Kst2dPlotWidget(QWidget* parent = 0, const char *name = 0, Qt::WindowFlags fl = 0);
    virtual ~Kst2dPlotWidget();

  public:
    void populateEditMultiple(const Kst2DPlot *plot);

  signals:
    void changed();

  public slots:
    void generateDefaultLabels(bool xl = false, bool yl = false, bool zl = false);
    void updateButtons();
    void upDisplayedCurve();
    void downDisplayedCurve();
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
    void insertXExpressionMin(const QString&);
    void insertYExpressionMin(const QString&);
    void insertXExpressionMax(const QString&);
    void insertYExpressionMax(const QString&);
    void modifiedYAxisText();
    void modifiedXAxisText();
    void modifiedTopAxisText();
    void autoLabelY();
    void autoLabelX();
    void autoLabelTop();

  protected:
    virtual void resizeEvent(QResizeEvent*);

  private:
    void populateEditMultiple(QRadioButton *radioButtonWidget);
    void populateEditMultiple(QComboBox *comboWidget);
    void populateEditMultiple(KColorButton *colorButton);
    void populateEditMultiple(QLineEdit *lineEditWidget);
    void populateEditMultiple(QSpinBox *spinBoxWidget);
    void populateEditMultiple(QCheckBox *checkBoxWidget);

    Kst2DPlotPtr _plot;
    QLineEdit *_scalarDest;
    bool _editMultipleMode;
};

#endif
