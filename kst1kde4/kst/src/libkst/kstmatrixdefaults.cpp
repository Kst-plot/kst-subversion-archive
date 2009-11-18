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

#include <kconfig.h>
#include <kconfiggroup.h>

#include "kstdatacollection.h" 
#include "kstmatrixdefaults.h"
#include "kstrmatrix.h"
#include "stdinsource.h"

KstMatrixDefaults KST::matrixDefaults;

KstMatrixDefaults::KstMatrixDefaults() {
  //
  // some arbitrary defaults for the defaults...
  //
  
  _dataSource = ".";
  _xStart = 0;
  _yStart = 0;
  _xNumSteps = -1;
  _yNumSteps = -1;
  _doSkip = false;
  _doAve = false;
  _skip = 0;
}

const QString& KstMatrixDefaults::dataSource() const {
  return _dataSource;
}


int KstMatrixDefaults::xStart() const {
  return _xStart;
}


bool KstMatrixDefaults::xCountFromEnd() const {
  return _xStart < 0;
}


int KstMatrixDefaults::yStart() const {
  return _yStart;
}


bool KstMatrixDefaults::yCountFromEnd() const {
  return _yStart < 0;
}


int KstMatrixDefaults::xNumSteps() const {
  return _xNumSteps;
}


bool KstMatrixDefaults::xReadToEnd() const {
  return _xNumSteps < 1;
}


int KstMatrixDefaults::yNumSteps() const {
  return _yNumSteps;
}


bool KstMatrixDefaults::yReadToEnd() const {
  return _yNumSteps < 1;
}


bool KstMatrixDefaults::doSkip() const {
  return _doSkip;
}


bool KstMatrixDefaults::doAverage() const {
  return _doAve;
}


int KstMatrixDefaults::skip() const {
  return _skip;
}

void KstMatrixDefaults::sync() {
  KstRMatrixList::iterator it;
  KstRMatrixList rmatrixList;
  KstRMatrixPtr rmatrix;

  KST::matrixList.lock().readLock();
  rmatrixList = kstObjectSubList<KstMatrix,KstRMatrix>(KST::matrixList);
  KST::matrixList.lock().unlock();
  
  //
  // find a non-stdin source...
  //
  
  for (it=rmatrixList.begin(); it!=rmatrixList.end(); ++it) {
    KstDataSourcePtr dsp;
    
    rmatrix = *it;
    
    rmatrix->readLock();
    dsp = rmatrix->dataSource();
    rmatrix->unlock();
    if (dsp && !kst_cast<KstStdinSource>(dsp)) {
      break;
    }
  }
  
  if (it != rmatrixList.end()) {
    rmatrix->readLock();

    _dataSource = rmatrix->filename();
    _xStart = rmatrix->reqXStart();
    _yStart = rmatrix->reqYStart();
    _xNumSteps = rmatrix->reqXNumSteps();
    _yNumSteps = rmatrix->reqYNumSteps();
    _skip = rmatrix->skip();
    _doAve = rmatrix->doAverage();
    _doSkip = rmatrix->doSkip();

    rmatrix->unlock();
  }
}

void KstMatrixDefaults::writeConfig(KConfig *config) {
  KConfigGroup	group(config, "matrixdefaults");
  
  group.writeEntry("defaultMatrixDataSource", KST::matrixDefaults.dataSource());
  group.writeEntry("defaultXStart", KST::matrixDefaults.xStart());
  group.writeEntry("defaultYStart", KST::matrixDefaults.yStart());
  group.writeEntry("defaultXNumSteps", KST::matrixDefaults.xNumSteps());
  group.writeEntry("defaultYNumSteps", KST::matrixDefaults.yNumSteps());
  group.writeEntry("defaultMatrixDoSkip", KST::matrixDefaults.doSkip());
  group.writeEntry("defaultMatrixDoAverage", KST::matrixDefaults.doAverage());
  group.writeEntry("defaultMatrixSkip", KST::matrixDefaults.skip());
}


void KstMatrixDefaults::readConfig(KConfig *config) {
  KConfigGroup	group(config, "matrixdefaults");

  _dataSource = group.readEntry("defaultMatrixDataSource", ".");
  _xStart = group.readEntry("defaultXStart", 0);
  _yStart = group.readEntry("defaultYStart", 0);
  _xNumSteps = group.readEntry("defaultXNumSteps", -1);
  _yNumSteps = group.readEntry("defaultYNumSteps", -1);
  _doSkip = group.readEntry("defaultMatrixDoSkip", 0);
  _doAve = group.readEntry("defaultMatrixDoAverage", 0);
  _skip = group.readEntry("defaultMatrixSkip", 0);
}
