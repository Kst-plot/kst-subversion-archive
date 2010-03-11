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

#ifndef SCALAREDITOR_H
#define SCALAREDITOR_H
 
#include <QWidget>
#include <kstscalar.h>
#include "ui_scalareditor.h"
#include "kst_export.h"

class KST_EXPORT ScalarEditor : public QDialog, public Ui::ScalarEditor {
  Q_OBJECT
public:
  ScalarEditor(QWidget *parent = 0);
  virtual ~ScalarEditor();
};
 
#endif





