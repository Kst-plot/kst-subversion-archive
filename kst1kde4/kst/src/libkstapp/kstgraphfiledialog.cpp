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

#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QSettings>

#include <kimageio.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include "kstgraphfiledialog.h"


KstGraphFileDialog::KstGraphFileDialog(QWidget* parent, const char* name,
                                         bool modal, Qt::WindowFlags fl)
: QDialog(parent, fl) {
  setupUi(this);
  _autoSaveTimer = new QTimer(this);

  connect(_autoSaveTimer, SIGNAL(timeout()),      this, SLOT(reqGraphFile()));
  connect(_ok,            SIGNAL(clicked()),      this, SLOT(ok_I()));
  connect(_Apply,         SIGNAL(clicked()),      this, SLOT(apply_I()));
  connect(_comboBoxSizeOption, SIGNAL(activated(int)), this, SLOT(enableWidthHeight()));
  connect(_comboBoxFormats, SIGNAL(activated(const QString&)), this, SLOT(enableEPSVector(const QString&)));
/* xxx
  _saveLocation->setFilter(KImageIO::mimeTypes().join(" "));
  _saveLocation->setMode(KFile::File);
*/
// xxx  _comboBoxFormats->insertStrList(QImageIO::outputFormats());
  _comboBoxFormats->setCurrentIndex(0);

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
// xxx  _url = _saveLocation->url();
  _format = _comboBoxFormats->currentText();
  _w = _xSize->value();
  _h = _ySize->value();
  _displayOption = _comboBoxSizeOption->currentIndex();
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
// xxx    _autoSaveTimer->start(_savePeriod*1000, false);
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
  QSettings cfg("kstrc", QSettings::NativeFormat, this);

  cfg.beginGroup("AutoSaveImages");

  //cfg.setValue("AutoSave", _autosave); // not read
  cfg.setValue("Seconds", _savePeriod);
  cfg.setValue("Location", _url);

  cfg.setValue("XSize", _w);
  cfg.setValue("YSize", _h);
  cfg.setValue("Display", _displayOption);
  cfg.setValue("Square", _displayOption == 1);
  cfg.setValue("All", _allWindows);
  cfg.setValue("Format", _format);
  cfg.setValue("EPSVector", _saveEPSAsVector);

  cfg.endGroup();
  cfg.sync();

}


void KstGraphFileDialog::loadProperties() {
  QSettings cfg("kstrc");
  bool isSquare;

  cfg.beginGroup("AutoSaveImages");

  if (_url.isEmpty()) {
    _url = cfg.value("Location", "").toString();
  }
  if (_url.isEmpty()) {
    _url = QDir::currentPath();
    if (_url.length() > 0) {
      if (_url.endsWith("/", Qt::CaseInsensitive)) {
        _url += QString("export");
      } else {
        _url += QString("/export");
      }
    }
  }

  _format = cfg.value("Format", "PNG").toString();
  _w = cfg.value("XSize", 640).toInt();
  _h = cfg.value("YSize", 480).toInt();
  isSquare = cfg.value("Square", false).toBool();
  if (isSquare) {
    _displayOption = 1;
  } else {
    _displayOption = cfg.value("Display", 0).toInt();
  }
  _allWindows = cfg.value("All", false).toBool();
  _autoSave = false; // do not read from config file...
  _savePeriod = cfg.value("Seconds", 15).toInt();
  _saveEPSAsVector = cfg.value("EPSVector", true).toBool();
}


void KstGraphFileDialog::enableEPSVector(const QString &format) {
  _saveEPSVector->setEnabled(format == "EPS");
}


void KstGraphFileDialog::enableWidthHeight() {
  int displayOption = _comboBoxSizeOption->currentIndex();

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
/* xxx
  _saveLocation->setURL(_url);
  _saveLocation->completionObject()->setDir(_url);
*/
  QString upfmt = _format.toUpper();

  for (int i = 0; i < _comboBoxFormats->count(); i++) {
    if (_comboBoxFormats->itemText(i).toUpper() == upfmt) {
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
