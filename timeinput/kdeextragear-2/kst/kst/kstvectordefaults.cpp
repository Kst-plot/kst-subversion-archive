/***************************************************************************
                            kstvectordefaults.cpp
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "kstvectordefaults.h"
#include "kstrvector.h"
#include "kstpsd.h"
#include "kstdatacollection.h"
#include "stdinsource.h"

#include <kconfig.h>

KstVectorDefaults KST::vectorDefaults;

KstVectorDefaults::KstVectorDefaults() {
  _dataSource = ".";
  _f0 = 0;
  _n = -1;
  _doSkip = false;
  _doAve = false;
  _skip = 0;
  _psd_freq = 1.0;
  _fft_len = 10;

  _vUnits = "V";
  _rUnits = "Hz";
  _apodize = true;
  _removeMean = true;
  _psd_average = true;
}

double KstVectorDefaults::f0() const {
  return _f0;
}

double KstVectorDefaults::n() const {
  return _n;
}

bool KstVectorDefaults::readToEOF() const {
  return _n <= 0;
}

bool KstVectorDefaults::countFromEOF() const {
  return _f0 < 0;
}

const QString& KstVectorDefaults::wizardXVector() const {
  return _wizardX;
}

const QString& KstVectorDefaults::dataSource() const {
  return _dataSource;
}

bool KstVectorDefaults::doSkip() const {
  return _doSkip;
}

bool KstVectorDefaults::doAve() const {
  return _doAve;
}

int KstVectorDefaults::skip() const {
  return _skip;
}

int KstVectorDefaults::fftLen() const {
  return _fft_len;
}

double KstVectorDefaults::psdFreq() const {
  return _psd_freq;
}

void KstVectorDefaults::sync() {
  KST::vectorList.lock().readLock();
  KstRVectorList vl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KST::vectorList.lock().readUnlock();
  int j = vl.count() - 1;

  // Find a non-stdin source
  while (j >= 0) {
    vl[j]->readLock();
    KstDataSourcePtr dsp = vl[j]->dataSource();
    vl[j]->readUnlock();
    if (dsp && !kst_cast<KstStdinSource>(dsp)) {
      break;
    }
    --j;
  }

  if (j >= 0) {
    vl[j]->readLock();
    _f0 = vl[j]->reqStartFrame();
    _n = vl[j]->reqNumFrames();
    _dataSource = vl[j]->filename();
    _skip = vl[j]->skip();
    _doAve = vl[j]->doAve();
    _doSkip = vl[j]->doSkip();
    vl[j]->readUnlock();
  }

  KstPSDList pl = kstObjectSubList<KstDataObject, KstPSD>(KST::dataObjectList);

  j = pl.count() - 1;
  if (j >= 0) {
    pl[j]->readLock();
    _psd_freq = pl[j]->freq();
    _fft_len = pl[j]->len();

    _vUnits = pl[j]->VUnits;
    _rUnits = pl[j]->RUnits;
    _apodize = pl[j]->apodize();
    _removeMean = pl[j]->removeMean();
    _psd_average = pl[j]->average();

    pl[j]->readUnlock();
  }
}

void KstVectorDefaults::writeConfig(KConfig *config) {
  config->writeEntry("defaultDataSource", KST::vectorDefaults.dataSource());
  config->writeEntry("defaultWizardXVector", KST::vectorDefaults.wizardXVector());
  config->writeEntry("defaultStartFrame", KST::vectorDefaults.f0());
  config->writeEntry("defaultNumFrames", KST::vectorDefaults.n());
  config->writeEntry("defaultDoSkip", KST::vectorDefaults.doSkip());
  config->writeEntry("defaultDoAve", KST::vectorDefaults.doAve());
  config->writeEntry("defaultSkip", KST::vectorDefaults.skip());
  config->writeEntry("defaultFFTLen", KST::vectorDefaults.fftLen());
  config->writeEntry("defaultPSDFreq", KST::vectorDefaults.psdFreq());

  config->writeEntry("defaultVUnits", KST::vectorDefaults.vUnits());
  config->writeEntry("defaultRUnits", KST::vectorDefaults.rUnits());
  config->writeEntry("defaultApodize", KST::vectorDefaults.apodize());
  config->writeEntry("defaultRemoveMean", KST::vectorDefaults.removeMean());
  config->writeEntry("defaultPSDAverage", KST::vectorDefaults.psdAverage());
}

void KstVectorDefaults::readConfig(KConfig *config) {
  _f0 = config->readDoubleNumEntry("defaultStartFrame", 0);
  _n = config->readDoubleNumEntry("defaultNumFrames", -1);
  _dataSource = config->readEntry("defaultDataSource", ".");
  _wizardX = config->readEntry("defaultWizardXVector", "INDEX");
  _doSkip = config->readNumEntry("defaultDoSkip", 0);
  _doAve = config->readNumEntry("defaultDoAve", 0);
  _skip = config->readNumEntry("defaultSkip", 0);
  _fft_len = config->readNumEntry("defaultFFTLen", 10);
  _psd_freq = config->readDoubleNumEntry("defaultPSDFreq", 100.0);

  _vUnits = config->readEntry("defaultVUnits",  "V" );
  _rUnits = config->readEntry("defaultRUnits",  "Hz" );
  _apodize = config->readNumEntry("defaultApodize",  1 );
  _removeMean = config->readNumEntry("defaultRemoveMean",  1 );
  _psd_average = config->readNumEntry("defaultPSDAverage",  1 );
}


void KstVectorDefaults::setWizardXVector(const QString& vector) {
  _wizardX = vector;
}

// vim: ts=2 sw=2 et
