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

#ifndef COLORPALETTEWIDGET_H
#define COLORPALETTEWIDGET_H
 
#include <QWidget>
#include "ui_colorpalettewidget.h"
#include "kst_export.h"

class KST_EXPORT ColorPaletteWidget : public QWidget, public Ui::ColorPaletteWidget {
   Q_OBJECT
public:
  ColorPaletteWidget(QWidget *parent = 0);
  virtual ~ColorPaletteWidget(); 

  void    updatePalette( const QString &palette);
  QString selectedPalette();
  void    refresh();
  void    refresh( const QString & palette );
  int     currentPaletteIndex();
};
 
#endif