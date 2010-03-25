/***************************************************************************
                     kstgraphfiledialog.cpp  -  Part of KST
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kimageio.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include "kstgraphfiledialog.h"


KstGraphFileDialog::KstGraphFileDialog(QWidget* parent, const char* name,
                                         bool modal, Qt::WindowFlags fl)
: QDialog(parent, name, modal, fl) {
  setupUi(this);
  _autoSaveTimer = new QTimer(this);

  connect(_autoSaveTimer, SIGNAL(timeout()),      this, SLOT(reqGraphFile()));
  connect(_ok,            SIGNAL(clicked()),      this, SLOT(ok_I()));
  connect(_Apply,         SIGNAL(clicked()),      this, SLOT(apply_I()));
  connect(_comboBoxSizeOption, SIGNAL(activated(int)), this, SLOT(enableWidthHeight()));
  connect(_comboBoxFormats, SIGNAL(activated(const QString&)), this, SLOT(enableEPSVector(const QString&)));

  _saveLocation->setFilter(KImageIO::mimeTypes().join(" "));
  _saveLocation->setMode(KFile::File);

  _comboBoxFormats->insertStrList(QImageIO::outputFormats());
  _comboBoxFormats->setCurrentItem(0);

  loadProperties();

  applyAutosave();
}


KstGraphFileDialog::~KstGraphFileDialog() {
}


void  KstGraphFileDialog::show_I() {
  loadProperties();
  updateDialog();
  show();
  raise();
}


void KstGraphFileDialog::ok_I() {
  apply_I();
  hide();
}


void KstGraphFileDialog::apply_I() {
  _url = _saveLocation->url();
  _format = _comboBoxFormats->currentText();
  _w = _xSize->value();
  _h = _ySize->value();
  _displayOption = _comboBoxSizeOption->currentItem();
  _allWindows = _radioButtonAll->isChecked();
  _autoSave = _autosave->isChecked();
  _savePeriod = _period->value();
  _saveEPSAsVector = _saveEPSVector->isChecked();
  applyAutosave();

  if (!_autoSave) {
    if (_format == "EPS" && _saveEPSAsVector) {
      reqEpsGraphFile();
    } else {
      reqGraphFile();
    }
  }
  saveProperties();
}


void KstGraphFileDialog::reqGraphFile() {
  emit graphFileReq(_url, _format, _w, _h, _allWindows, _displayOption);
}


void KstGraphFileDialog::reqEpsGraphFile() {
  emit graphFileEpsReq(_url, _w, _h, _allWindows, _displayOption);
}


void KstGraphFileDialog::applyAutosave() {
  if (_autoSave) {
    _autoSaveTimer->start(_savePeriod*1000, false);
  } else {
    _autoSaveTimer->stop();
  }
}


void KstGraphFileDialog::setURL(const QString& url) {
  QString path;

  if (url.isEmpty()) {
    path = QDir::currentPath();
  } else {
    path = url;
  }

  _url = path;
}


void KstGraphFileDialog::saveProperties() {
  KConfig cfg("kstrc", false, false);

  cfg.setGroup("AutoSaveImages");

  //cfg.writeEntry("AutoSave", _autosave); // not read
  cfg.writeEntry("Seconds", _savePeriod);
  cfg.writeEntry("Location", _url);

  cfg.writeEntry("XSize", _w);
  cfg.writeEntry("YSize", _h);
  cfg.writeEntry("Display", _displayOption);
  cfg.writeEntry("Square", _displayOption == 1);
  cfg.writeEntry("All", _allWindows);
  cfg.writeEntry("Format", _format);
  cfg.writeEntry("EPSVector", _saveEPSAsVector);

  cfg.sync();
}


void KstGraphFileDialog::loadProperties() {
  KConfig cfg("kstrc");
  bool isSquare;

  cfg.setGroup("AutoSaveImages");

  if (_url.isEmpty()) {
    _url = cfg.readEntry("Location", "");
  }
  if (_url.isEmpty()) {
    _url = QDir::currentPath();
    if (_url.length() > 0) {
      if (_url.endsWith("/", FALSE)) {
        _url += QString("export");
      } else {
        _url += QString("/export");
      }
    }
  }

  _format = cfg.readEntry("Format", "PNG");
  _w = cfg.readNumEntry("XSize", 640);
  _h = cfg.readNumEntry("YSize", 480);
  isSquare = cfg.readBoolEntry("Square", false);
  if (isSquare) {
    _displayOption = 1;
  } else {
    _displayOption = cfg.readNumEntry("Display", 0);
  }
  _allWindows = cfg.readBoolEntry("All", false);
  _autoSave = false; // do not read from config file...
  _savePeriod = cfg.readNumEntry("Seconds", 15);
  _saveEPSAsVector = cfg.readBoolEntry("EPSVector", true);
}


void KstGraphFileDialog::enableEPSVector(const QString &format) {
  _saveEPSVector->setEnabled(format == "EPS");
}


void KstGraphFileDialog::enableWidthHeight() {
  int displayOption = _comboBoxSizeOption->currentItem();

  switch (displayOption) {
    case 0:
      _xSize->setEnabled(true);
      _ySize->setEnabled(true);
      break;
    case 1:
      _xSize->setEnabled(true);
      _ySize->setEnabled(false);
      break;
    case 2:
      _xSize->setEnabled(true);
      _ySize->setEnabled(false);
      break;
    case 3:
      _xSize->setEnabled(false);
      _ySize->setEnabled(true);
      break;
  }
}


void KstGraphFileDialog::updateDialog() {
  if (_url.isEmpty()) {
    _url = QDir::currentPath();
  }
  _saveLocation->setURL(_url);
  _saveLocation->completionObject()->setDir(_url);

  QString upfmt = _format.upper();

  for (int i = 0; i < _comboBoxFormats->count(); i++) {
    if (_comboBoxFormats->text(i).toUpper() == upfmt) {
      _comboBoxFormats->setCurrentIndex(i);
      break;
    }
  }

  _xSize->setValue(_w);
  _ySize->setValue(_h);
  _comboBoxSizeOption->setCurrentIndex(_displayOption);
  _radioButtonAll->setChecked(_allWindows);
  _radioButtonActive->setChecked(!_allWindows);
  _autosave->setChecked(_autoSave);
  _saveOnce->setChecked(!_autoSave);
  _period->setValue(_savePeriod);
  _period->setEnabled(_autoSave);
  _saveEPSVector->setChecked(_saveEPSAsVector);

  enableEPSVector(_comboBoxFormats->currentText());
  enableWidthHeight();
}

#include "kstgraphfiledialog.moc"
