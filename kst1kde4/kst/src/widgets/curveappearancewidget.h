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

#ifndef CURVEAPPEARANCEWIDGET_H
#define CURVEAPPEARANCEWIDGET_H
 
#include <QWidget>
#include "ui_curveappearancewidget.h"
#include "kst_export.h"

class KST_EXPORT CurveAppearanceWidget : public QWidget, public Ui::CurveAppearanceWidget {
   Q_OBJECT
public:
  CurveAppearanceWidget(QWidget *parent = 0);
  virtual ~CurveAppearanceWidget();

  bool    showLines();
  bool    showPoints();
  bool    showBars();
  QColor  color();
  int     pointType();
  int     lineStyle();
  int     lineWidth();
  void    resizeEvent( QResizeEvent * pEvent );
  int     barStyle();
  int     pointDensity();

public slots:
  void    setColor( QColor c );
  void    drawLine();
  void    reset(QColor newColor);
  void    reset();
  void    setUsePoints( bool usePoints );
  void    setMustUseLines( bool bMustUseLines );
  void    redrawCombo();
  void    setValue( bool hasLines, bool hasPoints, bool hasBars, const QColor & c, int pointType, int lineWidth, int lineStyle, int barStyle, int pointDensity );
  void    enableSettings();

private slots:
  void    modified();
  void    fillCombo();
  void    comboChanged();
  void    fillLineStyleCombo();

signals:
  void    changed();  
};

#endif
