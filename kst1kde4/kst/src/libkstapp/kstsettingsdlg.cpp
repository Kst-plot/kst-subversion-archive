/***************************************************************************
                       kstsettingsdlg.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2007 The University of British Columbia
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

#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

#include "colorpalettewidget.h"
#include "kst.h"
#include "kstdatasource.h"
#include "kstplotdefines.h"
#include "kstsettingsdlg.h"
#include "ksttimezones.h"
#include "ktimezonecombo.h"

KstSettingsDlg::KstSettingsDlg(QWidget* parent, const char *name, Qt::WindowFlags fl) : QDialog(parent, fl) {
  QString hours = QObject::tr(" hours");
  QLineEdit* edit;

  setupUi(this);
printf("-1\n");
  fillAxesSettings();
  updateCurveColorSettings();
  setSettings(KstSettings::globalSettings());
  setClean();
  updateAxesButtons();
  updateAxesSettings();
  updateEMailSettings();
  updateUTCOffset();
printf("0\n");
  _source->insertItems(0, KstDataSource::pluginList());
  if (_source->count() > 0) {
    sourceChanged(_source->itemText(0));
  } else {
    _configureSource->setEnabled(false);
  }

  edit = _valueOffset->findChild<QLineEdit*>();
  if (edit) {
    edit->setMaxLength(5 + hours.length());
  }
printf("2\n");
  _valueOffset->setRange(-24.0, 24.0);
  _valueOffset->setSingleStep(0.50);
  _valueOffset->setSuffix(QObject::tr(" hours"));
// xxx  _colorPalette->_label->setText(QObject::tr("Curve color sequence: "));

  connect(_spinBoxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(setDirty()));
  connect(_spinBoxLineWidth->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));
  connect(_valueOffset->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(updateTimezone(const QString&)));
  connect(_timer->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));
  connect(_kIntSpinBoxEMailPort->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));
  connect(_colorPalette->_palette, SIGNAL(activated(int)), this, SLOT(setDirty()));
  connect(_fontSize->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));
  connect(_fontMinSize->findChild<QLineEdit*>(), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));

  connect(_cancel, SIGNAL(clicked()), this, SLOT(close()));
  connect(_timer, SIGNAL(valueChanged(int)), this, SLOT(setDirty()));
  connect(_defaults, SIGNAL(clicked()), this, SLOT(defaults()));
  connect(_ok, SIGNAL(clicked()), this, SLOT(save()));
  connect(_ok, SIGNAL(clicked()), this, SLOT(close()));
  connect(_colors, SIGNAL(bgChanged(QColor)), this, SLOT(setDirty()));
  connect(_colors, SIGNAL(bgChanged(QColor)), this, SLOT(setDirty()));
  connect(_xMajorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_xMinorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_yMajorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_yMinorGrid, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_checkBoxDefaultMajorGridColor, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_checkBoxDefaultMinorGridColor, SIGNAL(clicked()), this, SLOT(updateAxesButtons()));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), _comboBoxXInterpret, SLOT(setEnabled(bool)));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), _comboBoxXDisplay, SLOT(setEnabled(bool)));
  connect(_configureSource, SIGNAL(clicked()), this, SLOT(configureSource()));
  connect(_source, SIGNAL(clicked(QString)), this, SLOT(sourceChanged(QString)));
  connect(_xMajorGrid, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_xMinorGrid, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_yMajorGrid, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_yMinorGrid, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_majorGridColor, SIGNAL(changed(QColor)), this, SLOT(setDirty()));
  connect(_minorGridColor, SIGNAL(changed(QColor)), this, SLOT(setDirty()));
  connect(_checkBoxDefaultMajorGridColor, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_checkBoxDefaultMinorGridColor, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_comboBoxXDisplay, SIGNAL(activated(int)), this, SLOT(setDirty()));
  connect(_promptWindowClose, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_showQuickStart, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_tiedZoomGlobal, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_lineEditSender, SIGNAL(textChanged(QString)), this, SLOT(setDirty()));
  connect(_lineEditHost, SIGNAL(textChanged(QString)), this, SLOT(setDirty()));
  connect(_lineEditLogin, SIGNAL(textChanged(QString)), this, SLOT(setDirty()));
  connect(_lineEditPassword, SIGNAL(textChanged(QString)), this, SLOT(setDirty()));
  connect(_checkBoxAuthentication, SIGNAL(clicked()), this, SLOT(setDirty()));
  connect(_buttonGroupEncryption, SIGNAL(clicked(int)), this, SLOT(setDirty()));
  connect(_buttonGroupAuthentication, SIGNAL(clicked(int)), this, SLOT(setDirty()));
  connect(_checkBoxXInterpret, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
  connect(_checkBoxAuthentication, SIGNAL(clicked()), this, SLOT(updateEMailSettings()));
  connect(_comboBoxXInterpret, SIGNAL(activated(QString)), this, SLOT(setDirty()));
  connect(_fontSize, SIGNAL(valueChanged(int)), this, SLOT(setDirty()));
  connect(_valueOffset, SIGNAL(valueChanged(double)), this, SLOT(updateTimezone(double)));
  connect(_fontMinSize, SIGNAL(valueChanged(int)), this, SLOT(setDirty()));
printf("3\n");
}


KstSettingsDlg::~KstSettingsDlg() {
}


void KstSettingsDlg::setSettings(const KstSettings *settings) {
  _timer->setValue(settings->plotUpdateTimer);
  _fontSize->setValue(settings->plotFontSize);
  _fontMinSize->setValue(settings->plotFontMinSize);
  _colors->setBackground(settings->backgroundColor);
  _colors->setForeground(settings->foregroundColor);
  _promptPlotDelete->setChecked(settings->promptPlotDelete);
  _promptWindowClose->setChecked(settings->promptWindowClose);
  _showQuickStart->setChecked(settings->showQuickStart);
  _tiedZoomGlobal->setChecked(settings->tiedZoomGlobal);

  _xMajorGrid->setChecked(settings->xMajor);
  _yMajorGrid->setChecked(settings->yMajor);
  _xMinorGrid->setChecked(settings->xMinor);
  _yMinorGrid->setChecked(settings->yMinor);
  _majorGridColor->setColor(settings->majorColor);
  _minorGridColor->setColor(settings->minorColor);
  _checkBoxDefaultMajorGridColor->setChecked(settings->majorGridColorDefault);
  _checkBoxDefaultMinorGridColor->setChecked(settings->minorGridColorDefault);

  _checkBoxXInterpret->setChecked(settings->xAxisInterpret);
  _comboBoxXInterpret->setCurrentIndex(settings->xAxisInterpretation);
  _comboBoxXDisplay->setCurrentIndex(settings->xAxisDisplay);

  _spinBoxLineWidth->setValue(settings->defaultLineWeight);

  _lineEditSender->setText(settings->emailSender);
  _lineEditHost->setText(settings->emailSMTPServer);
  _lineEditLogin->setText(settings->emailUsername);
  _lineEditPassword->setText(settings->emailPassword);
  _kIntSpinBoxEMailPort->setValue(settings->emailSMTPPort);
  _checkBoxAuthentication->setChecked(settings->emailRequiresAuthentication);
// xxx  _buttonGroupEncryption->setButton((int)settings->emailEncryption);
// xxx  _buttonGroupAuthentication->setButton((int)settings->emailAuthentication);

  _tz->setTimezone(settings->timezone);
  setUTCOffset(settings->timezone);

// xxx  _colorPalette->refresh(settings->curveColorSequencePalette);
}


void KstSettingsDlg::defaults() {
  KstSettings s;

  setSettings(&s);
  setDirty();
}


void KstSettingsDlg::setDirty() {
  _dirty = true;
}


void KstSettingsDlg::setClean() {
  _dirty = false;
}


void KstSettingsDlg::save() {
  if (!_dirty) {
    return;
  }

  KstSettings s;

  s.plotUpdateTimer   = _timer->value();
  s.plotFontSize      = _fontSize->value();
  s.plotFontMinSize   = _fontMinSize->value();
  s.backgroundColor   = _colors->background();
  s.foregroundColor   = _colors->foreground();
  s.promptPlotDelete  = _promptPlotDelete->isChecked();
  s.promptWindowClose = _promptWindowClose->isChecked();
  s.showQuickStart    = _showQuickStart->isChecked();
  s.tiedZoomGlobal    = _tiedZoomGlobal->isChecked();

  s.curveColorSequencePalette = _colorPalette->selectedPalette();

  s.xMajor            = _xMajorGrid->isChecked();
  s.yMajor            = _yMajorGrid->isChecked();
  s.xMinor            = _xMinorGrid->isChecked();
  s.yMinor            = _yMinorGrid->isChecked();
  s.majorColor        = _majorGridColor->color();
  s.minorColor        = _minorGridColor->color();
  s.majorGridColorDefault = _checkBoxDefaultMajorGridColor->isChecked();
  s.minorGridColorDefault = _checkBoxDefaultMinorGridColor->isChecked();
  s.xAxisInterpret        = _checkBoxXInterpret->isChecked();
  s.xAxisInterpretation   = (KstAxisInterpretation)(_comboBoxXInterpret->currentIndex());
  s.xAxisDisplay          = (KstAxisDisplay)(_comboBoxXDisplay->currentIndex());

  s.defaultLineWeight = _spinBoxLineWidth->value();

  s.emailSender = _lineEditSender->text();
  s.emailSMTPServer = _lineEditHost->text();
  s.emailUsername = _lineEditLogin->text();
  s.emailPassword = _lineEditPassword->text();
  s.emailSMTPPort = _kIntSpinBoxEMailPort->value();
  s.emailRequiresAuthentication = _checkBoxAuthentication->isChecked();

  QString tzName = _tz->tzName();
  bool emitTZChanged = tzName != KstSettings::globalSettings()->timezone;
  s.timezone = tzName;
  s.offsetSeconds = utcOffset(tzName);
/* xxx
  int value = _buttonGroupEncryption->id(_buttonGroupEncryption->selected());
  if (value >= 0 && value < EMailEncryptionMAXIMUM) {
    s.emailEncryption = (EMailEncryption)value;
  } else {
    s.emailEncryption = EMailEncryptionNone;
  }

  value = _buttonGroupAuthentication->id(_buttonGroupAuthentication->selected());
  if (value >= 0 && value < EMailAuthenticationMAXIMUM) {
    s.emailAuthentication = (EMailAuthentication)value;
  } else {
    s.emailAuthentication = EMailAuthenticationPLAIN;
  }
*/
  KstSettings::setGlobalSettings(&s);
  KstSettings::globalSettings()->save();
  emit settingsChanged();
  if (emitTZChanged) {
    KstApp::inst()->emitTimezoneChanged(tzName, s.offsetSeconds);
  }
}


void KstSettingsDlg::updateAxesButtons() {
  bool major = _xMajorGrid->isChecked() || _yMajorGrid->isChecked();
  bool minor = _xMinorGrid->isChecked() || _yMinorGrid->isChecked();

  _checkBoxDefaultMajorGridColor->setEnabled(major);
  _checkBoxDefaultMinorGridColor->setEnabled(minor);
  _majorGridColor->setEnabled(major && !_checkBoxDefaultMajorGridColor->isChecked());
  _minorGridColor->setEnabled(minor && !_checkBoxDefaultMinorGridColor->isChecked());
}


void KstSettingsDlg::updateAxesSettings() {
  bool interpret = true;

  _comboBoxXInterpret->setEnabled(interpret);
  _comboBoxXDisplay->setEnabled(interpret);
}


void KstSettingsDlg::updateEMailSettings() {
  bool authenticate = _checkBoxAuthentication->isChecked();

  _lineEditLogin->setEnabled(authenticate);
  _textLabelEMailLogin->setEnabled(authenticate);
  _lineEditPassword->setEnabled(authenticate);
  _textLabelEMailPassword->setEnabled(authenticate);
  _buttonGroupAuthentication->setEnabled(authenticate);
}


void KstSettingsDlg::updateCurveColorSettings() {
    _colorPalette->refresh();
}


void KstSettingsDlg::fillAxesSettings() {
  unsigned int i;

  for (i = 0; i < numAxisInterpretations; i++) {
    _comboBoxXInterpret->insertItem(0, QObject::tr(AxisInterpretations[i].label));
  }
  for (i = 0; i < numAxisDisplays; i++) {
    _comboBoxXDisplay->insertItem(0, QObject::tr(AxisDisplays[i].label));
  }
}


void KstSettingsDlg::configureSource() {
  KstDataSourceConfigWidget *cw;
  cw = KstDataSource::configWidgetForPlugin(_source->currentText());

  if (cw) {
/* xxx
    KDialogBase *dlg = new KDialogBase(this, "Data Config Dialog", true, QObject::tr("Configure Data Source"));

    connect(dlg, SIGNAL(okClicked()), cw, SLOT(save()));
    cw->reparent(dlg, QPoint(0, 0));
    dlg->setMainWidget(cw);
    cw->load();
    dlg->exec();

    delete dlg;
*/
  }
}


void KstSettingsDlg::updateUTCOffset() {
  setUTCOffset(_tz->tzName());
}


int KstSettingsDlg::utcOffset(const QString& timezone) {
  int seconds = 0;

  if (timezone.startsWith("UTC")) {
    bool ok;
    int hours = timezone.mid(3).toInt(&ok);

    if (ok) {
      seconds = int( ( double(hours) / 100.0 ) * 3600.0);
    } else {
      seconds = 0;
    }
  } else {
    KstTimezones db;
    const KstTimezones::ZoneMap zones = db.allZones();
    KstTimezones::ZoneMap::const_iterator it;

    for (it = zones.begin(); it != zones.end(); ++it) {
      if ((*it)->name() == timezone) {
        seconds = -(*it)->offset();
      }
    }
  }

  return seconds;
}


void KstSettingsDlg::setUTCOffset(const QString& timezone) {
  double hours = double(utcOffset(timezone)) / 3600.0;

  _valueOffset->findChild<QLineEdit*>()->blockSignals(true);
  _valueOffset->blockSignals(true);

  _valueOffset->setValue(hours);

  _valueOffset->findChild<QLineEdit*>()->blockSignals(false);
  _valueOffset->blockSignals(false);

  setDirty();
}


void KstSettingsDlg::updateTimezone(const QString& strHours) {
  double hours = 0.0;
  QString strHrs = strHours;

  strHrs.replace(_valueOffset->suffix(), "");
  hours = strHrs.toDouble();
  if (hours > 24.0) {
    hours = 24.0;
    _valueOffset->setValue(hours);
  } else if (hours < -24.0) {
    hours = -24.0;
    _valueOffset->setValue(hours);
  } else {
    updateTimezone(hours);
  }
  setDirty();
}


void KstSettingsDlg::updateTimezone(double hours) {
  _tz->setCurrentIndex(0);
// xxx  _tz->setCurrentText(timezoneFromUTCOffset(hours));
  setDirty();
}


void KstSettingsDlg::sourceChanged(const QString& name) {
  _configureSource->setEnabled(KstDataSource::pluginHasConfigWidget(name));
}


QString KstSettingsDlg::timezoneFromUTCOffset(double hours) {
  QString tzName;
  bool negative = false;
  int offnum = int( floor( ( hours * 100.0 ) + 0.5 ) );

  if (offnum < 0) {
    negative = true;
    offnum *= -1;
  }
  tzName = QString("UTC%1%2").arg(negative ? '-' : '+').arg(offnum, 4);
  tzName.replace(' ', "0");

  return tzName;
}

#include "kstsettingsdlg.moc"
