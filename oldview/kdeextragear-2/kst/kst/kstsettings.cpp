/***************************************************************************
             kstsettings.cpp: a collection of settings for kst
                             -------------------
    begin                : Nov 23, 2003
    copyright            : (C) 2003 The University of Toronto
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

#include "kstsettings.h"

#include <kconfig.h>
#include <kstaticdeleter.h>

KstSettings::KstSettings() {
  plotUpdateTimer = 50;
  psdSampleRate = 1;
  backgroundColor = QColor(255, 255, 255); // white
  foregroundColor = QColor(0,0,0); // black
}


KstSettings::KstSettings(const KstSettings& x) {
  *this = x;
}


KstSettings& KstSettings::operator=(const KstSettings& x) {
  plotUpdateTimer = x.plotUpdateTimer;
  psdSampleRate = x.psdSampleRate;
  backgroundColor = x.backgroundColor;
  foregroundColor = x.foregroundColor;
  return *this;
}


KstSettings *KstSettings::_self = 0L;
static KStaticDeleter<KstSettings> kstsettingssd;

KstSettings *KstSettings::globalSettings() {
  if (!_self) {
    _self = kstsettingssd.setObject(_self, new KstSettings);
    _self->reload();
  }

  return _self;
}


void KstSettings::setGlobalSettings(const KstSettings *settings) {
  globalSettings(); // force instantiation

  *_self = *settings;
}


void KstSettings::save() {
  KConfig cfg("kstrc", false, false);

  cfg.setGroup("Kst");
  cfg.writeEntry("Plot Update Timer", plotUpdateTimer);
  cfg.writeEntry("PSD Sample Rate", psdSampleRate);
  cfg.writeEntry("Background Color", backgroundColor);
  cfg.writeEntry("Foreground Color", foregroundColor);
  cfg.sync();
}


void KstSettings::reload() {
  KConfig cfg("kstrc");

  cfg.setGroup("Kst");
  plotUpdateTimer = cfg.readNumEntry("Plot Update Timer", 50);
  psdSampleRate = cfg.readNumEntry("PSD Sample Rate", 1);
  backgroundColor = cfg.readColorEntry("Background Color", &backgroundColor);
  foregroundColor = cfg.readColorEntry("Foreground Color", &foregroundColor);
}


// vim: ts=2 sw=2 et
