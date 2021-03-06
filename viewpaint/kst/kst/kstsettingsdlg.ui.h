/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void KstSettingsDlg::init()
{
    fillAxesSettings();
    updateCurveColorSettings();
    setSettings(KstSettings::globalSettings());
    updateAxesButtons();
    updateAxesSettings();
    updateEMailSettings();
    _source->insertStringList(KstDataSource::pluginList());
    if (_source->count() > 0) {
	sourceChanged(_source->text(0));
    } else {
	_configureSource->setEnabled(false);
    }
    _apply->setEnabled(false);

    QString hours = i18n(" hours");
    QLineEdit* edit = dynamic_cast<QLineEdit*>(_valueOffset->child("qt_spinbox_edit"));
    if (edit) {
	edit->setMaxLength(5 + hours.length());
    }
    _valueOffset->setRange(-24.0, 24.0, 0.50, false);
    _valueOffset->setSuffix(i18n(" hours"));

    connect(_valueOffset->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(updateTimezone(const QString&)));
    connect(_timer->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));
    connect(_kIntSpinBoxEMailPort->child("qt_spinbox_edit"), SIGNAL(textChanged(const QString&)), this, SLOT(setDirty()));
}


void KstSettingsDlg::setSettings(const KstSettings *settings)
{
    _timer->setValue(settings->plotUpdateTimer);
    _fontSize->setValue(settings->plotFontSize);
    _colors->setBackground(settings->backgroundColor);
    _colors->setForeground(settings->foregroundColor);
    _promptWindowClose->setChecked(settings->promptWindowClose);
    _showQuickStart->setChecked(settings->showQuickStart);

    _xMajorGrid->setChecked(settings->xMajor);
    _yMajorGrid->setChecked(settings->yMajor);
    _xMinorGrid->setChecked(settings->xMinor);
    _yMinorGrid->setChecked(settings->yMinor);
    _majorGridColor->setColor(settings->majorColor);
    _minorGridColor->setColor(settings->minorColor);
    _checkBoxDefaultMajorGridColor->setChecked(settings->majorGridColorDefault);
    _checkBoxDefaultMinorGridColor->setChecked(settings->minorGridColorDefault);

    _checkBoxXInterpret->setChecked(settings->xAxisInterpret);
    _comboBoxXInterpret->setCurrentItem(settings->xAxisInterpretation);
    _comboBoxXDisplay->setCurrentItem(settings->xAxisDisplay);

    _lineEditSender->setText(settings->emailSender);
    _lineEditHost->setText(settings->emailSMTPServer);
    _lineEditLogin->setText(settings->emailUsername);
    _lineEditPassword->setText(settings->emailPassword);
    _kIntSpinBoxEMailPort->setValue(settings->emailSMTPPort);
    _checkBoxAuthentication->setChecked(settings->emailRequiresAuthentication);
    _buttonGroupEncryption->setButton((int)settings->emailEncryption);
    _buttonGroupAuthentication->setButton((int)settings->emailAuthentication);

    _tz->setTimezone(settings->timezone);
    setUTCOffset(settings->timezone);

    int index = -1;
    int i;

    for (i = 0; i < _comboBoxCurveColorSequence->count(); i++) {
	if (_comboBoxCurveColorSequence->text(i) == settings->curveColorSequencePalette) {
	    _comboBoxCurveColorSequence->setCurrentItem(i);
	    break;
	} else if (_comboBoxCurveColorSequence->text(i) == "Kst Colors") {
	    index = i;
	}
    }
    if (i == _comboBoxCurveColorSequence->count()) {
	if (index != -1) {
	    _comboBoxCurveColorSequence->setCurrentItem(index);
	} else {
	    _comboBoxCurveColorSequence->setCurrentItem(0);
	}
    }
}


void KstSettingsDlg::defaults()
{
    KstSettings s;
    setSettings(&s);
    setDirty();
}


void KstSettingsDlg::setDirty()
{
    _apply->setEnabled(true);
}


void KstSettingsDlg::setClean()
{
    _apply->setEnabled(false);
}


void KstSettingsDlg::save()
{
    KstSettings s;

    s.plotUpdateTimer   = _timer->value();
    s.plotFontSize      = _fontSize->value();
    s.backgroundColor   = _colors->background();
    s.foregroundColor   = _colors->foreground();
    s.promptWindowClose = _promptWindowClose->isChecked();
    s.showQuickStart    = _showQuickStart->isChecked();
    s.curveColorSequencePalette = _comboBoxCurveColorSequence->currentText();

    s.xMajor            = _xMajorGrid->isChecked();
    s.yMajor            = _yMajorGrid->isChecked();
    s.xMinor            = _xMinorGrid->isChecked();
    s.yMinor            = _yMinorGrid->isChecked();
    s.majorColor        = _majorGridColor->color();
    s.minorColor        = _minorGridColor->color();
    s.majorGridColorDefault = _checkBoxDefaultMajorGridColor->isChecked();
    s.minorGridColorDefault = _checkBoxDefaultMinorGridColor->isChecked();
    s.xAxisInterpret        = _checkBoxXInterpret->isChecked();
    s.xAxisInterpretation   = (KstAxisInterpretation)(_comboBoxXInterpret->currentItem());
    s.xAxisDisplay          = (KstAxisDisplay)(_comboBoxXDisplay->currentItem());

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

    KstSettings::setGlobalSettings(&s);
    KstSettings::globalSettings()->save();
    emit settingsChanged();
    if (emitTZChanged) {
	KstApp::inst()->emitTimezoneChanged(tzName, s.offsetSeconds);
    }
}


void KstSettingsDlg::updateAxesButtons()
{
    bool major = _xMajorGrid->isChecked() || _yMajorGrid->isChecked();
    bool minor = _xMinorGrid->isChecked() || _yMinorGrid->isChecked();

    _checkBoxDefaultMajorGridColor->setEnabled(major);
    _checkBoxDefaultMinorGridColor->setEnabled(minor);
    _majorGridColor->setEnabled(major && !_checkBoxDefaultMajorGridColor->isChecked());
    _minorGridColor->setEnabled(minor && !_checkBoxDefaultMinorGridColor->isChecked());
}


void KstSettingsDlg::updateAxesSettings()
{
    bool interpret = _checkBoxXInterpret->isChecked();

    _comboBoxXInterpret->setEnabled(interpret);
    _comboBoxXDisplay->setEnabled(interpret);
}


void KstSettingsDlg::updateEMailSettings()
{
    bool authenticate = _checkBoxAuthentication->isChecked();

    _lineEditLogin->setEnabled(authenticate);
    _textLabelEMailLogin->setEnabled(authenticate);
    _lineEditPassword->setEnabled(authenticate);
    _textLabelEMailPassword->setEnabled(authenticate);
    _buttonGroupAuthentication->setEnabled(authenticate);
}


void KstSettingsDlg::updateCurveColorSettings()
{
    _comboBoxCurveColorSequence->insertStringList(KPalette::getPaletteList());
}


void KstSettingsDlg::fillAxesSettings()
{
    unsigned int i;

    for (i = 0; i < numAxisInterpretations; i++) {
	_comboBoxXInterpret->insertItem(i18n(AxisInterpretations[i].label));
    }
    for (i = 0; i < numAxisDisplays; i++) {
	_comboBoxXDisplay->insertItem(i18n(AxisDisplays[i].label));
    }
}


void KstSettingsDlg::configureSource()
{
    KstDataSourceConfigWidget *cw = KstDataSource::configWidgetForPlugin(_source->currentText());
    if (!cw) {
	return;
    }
    KDialogBase *dlg = new KDialogBase(this, "Data Config Dialog", true, i18n("Configure Data Source"));
    connect(dlg, SIGNAL(okClicked()), cw, SLOT(save()));
    connect(dlg, SIGNAL(applyClicked()), cw, SLOT(save()));
    cw->reparent(dlg, QPoint(0, 0));
    dlg->setMainWidget(cw);
    cw->load();
    dlg->exec();
    delete dlg;
}


void KstSettingsDlg::updateUTCOffset()
{
    setUTCOffset(_tz->tzName());
}


int KstSettingsDlg::utcOffset(const QString& timezone)
{
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
	for (KstTimezones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it) {
	    if ((*it)->name() == timezone) {
		seconds = (*it)->offset();
	    }
	}
    }
    return seconds;
}


void KstSettingsDlg::setUTCOffset(const QString& timezone)
{
    double hours = double(utcOffset(timezone)) / 3600.0;
    _valueOffset->child("qt_spinbox_edit")->blockSignals(true);
    _valueOffset->blockSignals(true);
    _valueOffset->setValue(hours);
    _valueOffset->child("qt_spinbox_edit")->blockSignals(false);
    _valueOffset->blockSignals(false);
    setDirty();
}


void KstSettingsDlg::updateTimezone(const QString& strHours)
{
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


void KstSettingsDlg::updateTimezone(double hours)
{
    _tz->setCurrentItem(0);
    _tz->setCurrentText(timezoneFromUTCOffset(hours));
    setDirty();
}


void KstSettingsDlg::sourceChanged(const QString& name)
{
    _configureSource->setEnabled(KstDataSource::pluginHasConfigWidget(name));
}


QString KstSettingsDlg::timezoneFromUTCOffset(double hours)
{
    bool negative = false;
    int offnum = int( floor( ( hours * 100.0 ) + 0.5 ) );

    if (offnum < 0) {
	negative = true;
	offnum *= -1;
    }
    QString tzName = QString("UTC%1%2").arg(negative ? '-' : '+').arg(offnum, 4);
    tzName.replace(' ', "0");
    return tzName;
}


// vim: ts=8 sw=4 noet
