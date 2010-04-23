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
#include "kstdatacollection.h"
#include "stdinsource.h"

KstVectorDefaults KST::vectorDefaults;

KstVectorDefaults::KstVectorDefaults() {
  //
  // some arbitrary defaults for the defaults...
  //
  
  _dataSource = ".";
  _f0 = 0;
  _n = -1;
  _doSkip = false;
  _doAve = false;
  _skip = 0;
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


void KstVectorDefaults::sync() { 
  KstRVectorList::iterator it;
  KstRVectorList rvectorList;
  KstRVectorPtr rvector;

  KST::vectorList.lock().readLock();
  rvectorList = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  KST::vectorList.lock().unlock();
  
  //
  // find a non-stdin source...
  //
  
  for (it=rvectorList.begin(); it!=rvectorList.end(); ++it) {
    KstDataSourcePtr dsp;
    
    rvector = *it;
    
    rvector->readLock();
    dsp = rvector->dataSource();
    rvector->unlock();
    
    if (dsp && !kst_cast<KstStdinSource>(dsp)) {
      break;
    }
  }

  if (it != rvectorList.end()) {
    rvector->readLock();

    _f0 = rvector->reqStartFrame();
    _n = rvector->reqNumFrames();
    _dataSource = rvector->filename();
    _skip = rvector->skip();
    _doAve = rvector->doAve();
    _doSkip = rvector->doSkip();
    
    rvector->unlock();
  }
}


void KstVectorDefaults::writeConfig(QSettings *config) {
  config->beginGroup("vectordefaults");
  
  config->setValue("defaultDataSource", KST::vectorDefaults.dataSource());
  config->setValue("defaultWizardXVector", KST::vectorDefaults.wizardXVector());
  config->setValue("defaultStartFrame", KST::vectorDefaults.f0());
  config->setValue("defaultNumFrames", KST::vectorDefaults.n());
  config->setValue("defaultDoSkip", KST::vectorDefaults.doSkip());
  config->setValue("defaultDoAve", KST::vectorDefaults.doAve());
  config->setValue("defaultSkip", KST::vectorDefaults.skip());

	config->endGroup();
}


void KstVectorDefaults::readConfig(QSettings *config) {
  config->beginGroup("vectordefaults");

  _f0 = config->value("defaultStartFrame", 0.0).toDouble();
  _n = config->value("defaultNumFrames", -1.0).toDouble();
  _dataSource = config->value("defaultDataSource", ".").toString();
  _wizardX = config->value("defaultWizardXVector", "INDEX").toString();
  _doSkip = config->value("defaultDoSkip", 0).toBool();
  _doAve = config->value("defaultDoAve", 0).toBool();
  _skip = config->value("defaultSkip", 0).toInt();

	config->endGroup();
}


void KstVectorDefaults::setWizardXVector(const QString& vector) {
  _wizardX = vector;
}
