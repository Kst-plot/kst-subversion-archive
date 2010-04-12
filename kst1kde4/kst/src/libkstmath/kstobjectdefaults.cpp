/***************************************************************************
                            kstobjectdefaults.cpp
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

#include "kstobjectdefaults.h"
#include "kstdataobjectcollection.h"
#include "kstpsd.h"

#include <QSettings>

KstObjectDefaults KST::objectDefaults;

KstObjectDefaults::KstObjectDefaults() {
  _psd_freq = 1.0;
  _fft_len = 10;
  _vUnits = "V";
  _rUnits = "Hz";
  _apodize = true;
  _removeMean = true;
  _psd_average = true;
  _apodizeFxn = 0;
  _output = 0;
  _interpolateHoles = false;
}


void KstObjectDefaults::sync() {
  KstPSDList pl;
  KstPSDList::iterator it;
  
  // xxx pl = kstObjectSubList<KstDataObject, KstPSD>(KST::dataObjectList);
  
  for (it=pl.begin(); it!=pl.end(); ++it) {
    (*it)->readLock();

    _psd_freq = (*it)->freq();
    _fft_len = (*it)->len();

    _vUnits = (*it)->vUnits();
    _rUnits = (*it)->rUnits();
    _apodize = (*it)->apodize();
    _removeMean = (*it)->removeMean();
    _psd_average = (*it)->average();
    _apodizeFxn = (*it)->apodizeFxn();
    _output = (*it)->output();
    _interpolateHoles = (*it)->interpolateHoles();

    (*it)->unlock();
  }
}


double KstObjectDefaults::psdFreq() const { 
  return _psd_freq; 
}


int KstObjectDefaults::fftLen() const { 
  return _fft_len; 
}


void KstObjectDefaults::writeConfig(QSettings *config) {

  config->setValue("defaultFFTLen", KST::objectDefaults.fftLen());
  config->setValue("defaultPSDFreq", KST::objectDefaults.psdFreq());
  config->setValue("defaultVUnits", KST::objectDefaults.vUnits());
  config->setValue("defaultRUnits", KST::objectDefaults.rUnits());
  config->setValue("defaultApodize", KST::objectDefaults.apodize());
  config->setValue("defaultRemoveMean", KST::objectDefaults.removeMean());
  config->setValue("defaultPSDAverage", KST::objectDefaults.psdAverage());
  config->setValue("defaultApodizeFxn", KST::objectDefaults.apodizeFxn());
  config->setValue("defaultOutput", KST::objectDefaults.output());
  config->setValue("defaultInterpolateHoles", KST::objectDefaults.interpolateHoles());

}


void KstObjectDefaults::readConfig(QSettings *config) {

  _fft_len = config->value("defaultFFTLen", 10).toInt();
  _psd_freq = config->value("defaultPSDFreq", 100.0).toDouble();
  _vUnits = config->value("defaultVUnits", "V").toString();
  _rUnits = config->value("defaultRUnits", "Hz").toString();
  _apodize = config->value("defaultApodize", 1).toBool();
  _removeMean = config->value("defaultRemoveMean", 1).toBool();
  _psd_average = config->value("defaultPSDAverage", 1).toBool();
  _apodizeFxn = config->value("defaultApodizeFxn", 0).toInt();
  _output = config->value("defaultOutput", 0).toInt();
  _interpolateHoles = config->value("defaultInterpolateHoles", false).toBool();

}
