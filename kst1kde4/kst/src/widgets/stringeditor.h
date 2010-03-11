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

#ifndef STRINGEDITOR_H
#define STRINGEDITOR_H
 
#include <QWidget>
#include <kstscalar.h>
#include "ui_stringeditor.h"
#include "kst_export.h"

class KST_EXPORT StringEditor : public QDialog, public Ui::StringEditor {
  Q_OBJECT
public:
  StringEditor(QWidget *parent = 0);
  virtual ~StringEditor();
};
 
#endif





