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

#ifndef FFTOPTIONSWIDGET_H
#define FFTOPTIONSWIDGET_H
 
#include <QWidget>
#include "ui_fftoptionswidget.h"
#include "kst_export.h"

class KST_EXPORT KstFFTOptions : public QWidget, public Ui::KstFFTOptions {
   Q_OBJECT
public:
  KstFFTOptions(QWidget *parent = 0);
  virtual ~KstFFTOptions(); 

  void update();
  void synch();
  bool checkValues();
  bool checkGivenValues(double sampRate, int FFTLen);

private Q_SLOTS:
  void changedApodizeFxn();
  void clickedInterleaved();
  void clickedApodize();
};

#endif
