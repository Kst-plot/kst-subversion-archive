/***************************************************************************
                     kstgraphdialog_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <kurlrequester.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>

#include "kstgraphfiledialog_i.h"

KstGraphFileDialogI::KstGraphFileDialogI(QWidget* parent, const char* name,
                                         bool modal, WFlags fl)
: KstGraphFileDialog(parent, name, modal, fl) {
  autoSaveTimer = new QTimer(this);

  connect(_square,       SIGNAL(clicked()),         this, SLOT(square()));
  connect(_save,         SIGNAL(clicked()),         this, SLOT(reqGraphFile()));
  connect(_autosave,     SIGNAL(toggled(bool)),     this, SLOT(setAutoSave()));
  connect(_period,       SIGNAL(valueChanged(int)), this, SLOT(setAutoSave()));
  connect(autoSaveTimer, SIGNAL(timeout()),         this, SLOT(reqGraphFile()));
  connect(_xSize,     SIGNAL(valueChanged(int)), this, SLOT(xsizeChanged(int)));

  _url->setFilter(KImageIO::mimeTypes().join(" "));
  _url->setMode(KFile::File);
}

KstGraphFileDialogI::~KstGraphFileDialogI() {
}

void  KstGraphFileDialogI::showGraphFileDialog() {
  show();
  raise();
}

void KstGraphFileDialogI::square() {
  xsizeChanged(_xSize->value());
}

void KstGraphFileDialogI::xsizeChanged(int x) {
  if (_square->isChecked()) {
    _ySize->setValue(x);
  }
}

void KstGraphFileDialogI::reqGraphFile() {
  emit graphFileReq(_url->url(), _xSize->value(), _ySize->value());
}

void KstGraphFileDialogI::setAutoSave() {
  if (_autosave->isChecked()) {
    autoSaveTimer->start(_period->value()*1000, false);
  } else {
    autoSaveTimer->stop();
  }
}

#include "kstgraphfiledialog_i.moc"
