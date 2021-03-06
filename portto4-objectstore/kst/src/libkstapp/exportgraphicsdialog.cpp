/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "exportgraphicsdialog.h"

#include "mainwindow.h"

namespace Kst {

ExportGraphicsDialog::ExportGraphicsDialog(MainWindow *parent)
  : QDialog(parent) {
  setupUi(this);
}


ExportGraphicsDialog::~ExportGraphicsDialog() {
}

}

// vim: ts=2 sw=2 et
