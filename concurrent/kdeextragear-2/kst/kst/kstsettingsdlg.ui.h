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
    setSettings(KstSettings::globalSettings());
}


void KstSettingsDlg::setSettings(const KstSettings *settings)
{
    _psdSampleRate->setValue(settings->psdSampleRate);
    _timer->setValue(settings->plotUpdateTimer);
    _colors->setBackground(settings->backgroundColor);
    _colors->setForeground(settings->foregroundColor);
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
    s.psdSampleRate = _psdSampleRate->value();
    s.plotUpdateTimer = _timer->value();
    s.backgroundColor = _colors->background();
    s.foregroundColor = _colors->foreground();
    KstSettings::setGlobalSettings(&s);
    KstSettings::globalSettings()->save();
    emit settingsChanged();
}
