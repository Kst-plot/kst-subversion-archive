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

// xxx #include <kconfig.h>
// xxx #include <kconfiggroup.h>
#include <QSettings>

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

void KstMatrixDefaults::writeConfig(QSettings *config) {
  config->beginGroup("matrixdefaults");
  
  config->setValue("defaultMatrixDataSource", KST::matrixDefaults.dataSource());
  config->setValue("defaultXStart", KST::matrixDefaults.xStart());
  config->setValue("defaultYStart", KST::matrixDefaults.yStart());
  config->setValue("defaultXNumSteps", KST::matrixDefaults.xNumSteps());
  config->setValue("defaultYNumSteps", KST::matrixDefaults.yNumSteps());
  config->setValue("defaultMatrixDoSkip", KST::matrixDefaults.doSkip());
  config->setValue("defaultMatrixDoAverage", KST::matrixDefaults.doAverage());
  config->setValue("defaultMatrixSkip", KST::matrixDefaults.skip());
  
  config->endGroup();
}


void KstMatrixDefaults::readConfig(QSettings *config) {
  config->beginGroup("matrixdefaults");

  _dataSource = config->value("defaultMatrixDataSource", ".").toString();
  _xStart = config->value("defaultXStart", 0).toInt();
  _yStart = config->value("defaultYStart", 0).toInt();
  _xNumSteps = config->value("defaultXNumSteps", -1).toInt();
  _yNumSteps = config->value("defaultYNumSteps", -1).toInt();
  _doSkip = config->value("defaultMatrixDoSkip", 0).toBool();
  _doAve = config->value("defaultMatrixDoAverage", 0).toBool();
  _skip = config->value("defaultMatrixSkip", 0).toInt();

  config->endGroup();
}
