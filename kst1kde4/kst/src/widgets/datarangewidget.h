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

#ifndef DATARANGEWIDGET_H
#define DATARANGEWIDGET_H
 
#include <QWidget>
#include "ui_datarangewidget.h"
#include "kst_export.h"

class KST_EXPORT KstDataRange : public QWidget, public Ui::DataRange
 {
   Q_OBJECT
public:
  KstDataRange(QWidget *parent = 0);
  virtual ~KstDataRange(); 

  void updateEnables();
  void update();
  void setAllowTime(bool allow);
  void setF0Value(double v);
  void setNValue(double v);
  double interpret(const char *txt);
  double f0Value();
  double nValue();
  KST::ExtDateTime f0DateTimeValue();
  bool isStartRelativeTime();
  bool isStartAbsoluteTime();
  bool isRangeRelativeTime();

signals:
  void changed();

public slots:
  void modified();

private slots:
  void clickedCountFromEnd();
  void clickedReadToEnd();
  void clickedDoSkip();

private:
  bool  _time;
};

#endif