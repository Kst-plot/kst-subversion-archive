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

  void    modified();
  bool    showLines();
  bool    showPoints();
  bool    showBars();
  void    fillCombo();
  void    setColor( QColor c );
  QColor  color();
  void    drawLine();
  int     pointType();
  void    reset(QColor newColor);
  void    comboChanged();
  void    reset();
  void    setUsePoints( bool usePoints );
  void    setMustUseLines( bool bMustUseLines );
  void    redrawCombo();
  void    fillLineStyleCombo();
  int     lineStyle();
  int     lineWidth();
  void    setValue( bool hasLines, bool hasPoints, bool hasBars, const QColor & c, int pointType, int lineWidth, int lineStyle, int barStyle, int pointDensity );
  void    resizeEvent( QResizeEvent * pEvent );
  int     barStyle();
  int     pointDensity();
  void    enableSettings();
};

#endif