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

#include <kconfig.h>
#include <kconfiggroup.h>

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


void KstVectorDefaults::writeConfig(KConfig *config) {
  KConfigGroup	group(config, "vectordefaults");
  
  group.writeEntry("defaultDataSource", KST::vectorDefaults.dataSource());
  group.writeEntry("defaultWizardXVector", KST::vectorDefaults.wizardXVector());
  group.writeEntry("defaultStartFrame", KST::vectorDefaults.f0());
  group.writeEntry("defaultNumFrames", KST::vectorDefaults.n());
  group.writeEntry("defaultDoSkip", KST::vectorDefaults.doSkip());
  group.writeEntry("defaultDoAve", KST::vectorDefaults.doAve());
  group.writeEntry("defaultSkip", KST::vectorDefaults.skip());
}


void KstVectorDefaults::readConfig(KConfig *config) {
  KConfigGroup  group(config, "vectordefaults");

  _f0 = group.readEntry("defaultStartFrame", 0.0);
  _n = group.readEntry("defaultNumFrames", -1.0);
  _dataSource = group.readEntry("defaultDataSource", ".");
  _wizardX = group.readEntry("defaultWizardXVector", "INDEX");
  _doSkip = group.readEntry("defaultDoSkip", 0);
  _doAve = group.readEntry("defaultDoAve", 0);
  _skip = group.readEntry("defaultSkip", 0);
}


void KstVectorDefaults::setWizardXVector(const QString& vector) {
  _wizardX = vector;
}
