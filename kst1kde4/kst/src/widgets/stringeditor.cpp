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
 
#include "stringeditor.h"

StringEditor::StringEditor(QWidget *parent) : QDialog(parent) {
  setupUi(this);

  connect(_pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
  connect(_pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

StringEditor::~StringEditor() {
}



