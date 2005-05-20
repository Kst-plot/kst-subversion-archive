/***************************************************************************
                     cdf.cpp  -  CDF file data source
                             -------------------
    begin                : 17/06/2004
    copyright            : (C) 2004 Nicolas Brisset <nicodev@users.sourceforge.net>
    email                : kst@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cdf_source.h" // Local header for the kst CDF datasource
#include <cdf.h>
#include "cdfdefs.h" // Macros to handle rVars and zVars transparently

#include <kdebug.h>

#include <qfile.h>
#include <qfileinfo.h>

#include <ctype.h>
#include <stdlib.h>

CdfSource::CdfSource(KConfig *cfg, const QString& filename, const QString& type)
: KstDataSource(cfg, filename, type) {
  _maxFrameCount = 0;

  if (!type.isEmpty() && type != "CDF") {
    return;
  }

  _valid = initFile();
}


CdfSource::~CdfSource() {
  _maxFrameCount = 0;
}


bool CdfSource::reset() {
  _maxFrameCount = 0;
  _fieldList.clear();
  return _valid = initFile();
}


bool CdfSource::initFile() {
  kdDebug() << _filename << ": entering initFile()" << endl;
  CDFid id;
  CDFstatus status = CDFopen(_filename.latin1(), &id);
  if (status < CDF_OK) {
    kdDebug() << _filename << ": failed to open in initFile()" << endl;
    return false;
  }

  // Query field list and store it in _fieldList (plus "INDEX")
  kdDebug() << _filename << ": building field list" << endl;
  _fieldList.clear();
  //_fieldList += "INDEX"; commented out as it leads to problems in countFrames():
  // which value should the method return when not all variables have the same length ?
  long numRvars, numZvars, varN, numDims;
  char varName[CDF_VAR_NAME_LEN + 1];
  status = CDFlib(SELECT_, CDF_READONLY_MODE_, READONLYon,
      GET_, CDF_NUMrVARS_, &numRvars,
      CDF_NUMzVARS_, &numZvars,
      NULL_);
  // Add 0-dimensional rVariables 
  for (varN = 0; varN < numRvars; varN++) {
    status = CDFlib(SELECT_, rVAR_, varN, GET_, rVAR_NAME_, varName,
        rVARs_NUMDIMS_, &numDims, NULL_);
    if (status == CDF_OK && numDims == 0) {
      _fieldList += varName;
    }
  }
  // Add 0-dimensional zVariables 
  for (varN = 0; varN < numZvars; varN++) {
    status = CDFlib(SELECT_, zVAR_, varN, GET_, zVAR_NAME_, varName,
        zVAR_NUMDIMS_, &numDims, NULL_);
    if (status == CDF_OK && numDims == 0) {
      _fieldList += varName;
    }
  }
  kdDebug() << _filename << ": variables found: " << _fieldList.count() << endl;

  // Count frames for all fields and store the result in _frameCounts
  kdDebug() << _filename << ": counting frames " << endl;
  long maxRec;
  bool isZvar = true;
  for (QStringList::Iterator it = _fieldList.begin(); it != _fieldList.end(); ++it ) {
    QString ftmp = *it;
    ftmp.truncate(CDF_VAR_NAME_LEN);
    strcpy(varName, ftmp.latin1());
    status = CDFlib(SELECT_, zVAR_NAME_, varName, NULL_);
    if (status < CDF_OK) { // if not zVar, try rVar
      isZvar = false;
      status = CDFlib(SELECT_, rVAR_NAME_, varName, NULL_);
    }
    if (status < CDF_OK) { // No such var in the file, I know this is not supposed to happen but... :-)
      CDFclose(id);
      kdDebug() << _filename << ": critical problem with" << ftmp << endl;
      return false; 
    }
    // If we've come so far, it means the var exists and we know whether it is a Zvar
    // so we can query its record count easily
    status = CDFlib(GET_, BOO(isZvar,zVAR_MAXREC_,rVAR_MAXREC_), &maxRec, NULL_);
    // Now, store the value in the _frameCount QMap
    _frameCounts[ftmp] = (int) maxRec;
    // Update _maxFrameCount if relevant
    if ((int) maxRec > _maxFrameCount) _maxFrameCount = maxRec;
  }

  // Close the file :-)
  status = CDFclose(id);

  return status >= CDF_OK;
}


KstObject::UpdateType CdfSource::update(int u) {
  if (KstObject::checkUpdateCounter(u)) {
    return lastUpdateResult();
  }
  return setLastUpdateResult(KstObject::NO_CHANGE);
}


int CdfSource::readField(double *v, const QString& field, int s, int n) {
  int i;
  CDFstatus status;
  CDFid id;
  long dataType;          /* CDF data type */
  long maxRec;            /* max number of values for this var */
  char varName[CDF_VAR_NAME_LEN+1];
  bool isZvar = true;     /* Should be the case for recent cdf files */
  // Allocate an arbitrary (large) size
  kdDebug() << "Entering CdfSource::readField with params: " << field << ", from " << s << " for " << n << " values" << endl;

  // Handle the special case where we query INDEX
  if (field.lower() == "index") {
    if (n < 0) {
      v[0] = double(s);
      return 1;
    }
    for (int i = 0; i < n; ++i) {
      v[i] = double(s + i);
    }
    return n;
  }

  // If not INDEX, look into the CDF file...
  status = CDFopen(_filename.latin1(), &id);
  if (status < CDF_OK) {
    kdDebug() << _filename << ": failed to open to read from field " << field << endl;
    return -1;
  }

  kdDebug() << _filename << " opened." << endl;
  QString ftmp = field;
  ftmp.truncate(CDF_VAR_NAME_LEN);
  // Variable selection
  strcpy(varName, ftmp.latin1());
  status = CDFlib(SELECT_, zVAR_NAME_, varName, GET_, zVAR_DATATYPE_, &dataType, NULL_);
  if (status < CDF_OK) { // if not zVar, try rVar
    kdDebug() << ftmp << ": " << " not a zVAR (" << status <<")" << endl;
    isZvar = false;
    status = CDFlib(SELECT_, rVAR_NAME_, varName, GET_, rVAR_DATATYPE_, &dataType, NULL_);
  }
  // I suppose the returned int is the number of values read, <0 when there is a problem
  if (status < CDF_OK) {
    kdDebug() << ftmp << ": " << " not a rVAR either -> exiting" << endl;
    CDFclose(id);
    return -1; 
  }
  
  // If n<0 set it to 1 as suggested by George Staikos
  // (needs to be documented better I guess !)
  // First check for the existence of more values for this field
  status = CDFlib (GET_, BOO(isZvar, zVAR_MAXREC_, rVAR_MAXREC_), &maxRec, NULL_);

  if (n < 0) {
    n = 1;
  }
  
  void *binary = malloc(sizeof(long double)); // FIXME: put this on the stack

  kdDebug() << "Starting to read " << n << " value(s)... from " << s << " to " << s+n << " and less than " << maxRec << endl;
  for (i = s; i < s+n && i < maxRec; i++) {		 
    status = CDFlib (SELECT_,
        BOO(isZvar, zVAR_RECNUMBER_, rVAR_SEQPOS_), (long) i,
        GET_, BOO(isZvar, zVAR_DATA_, rVAR_DATA_), binary,
        NULL_);
    switch (dataType) {
      case CDF_INT2:
        v[i-s] = (double) *((Int16 *) binary);
        break;
      case CDF_INT4:
        v[i-s] = (double) *((Int32 *) binary);
        break;
      case CDF_UINT1:
        v[i-s] = (double) *((uChar *) binary);
        break;
      case CDF_UINT2:
        v[i-s] = (double) *((uInt16 *) binary);
        break;
      case CDF_UINT4:
        v[i-s] = (double) *((uInt32 *) binary);
        break;
      case CDF_REAL4:
      case CDF_FLOAT:
        v[i-s] = (double) *((float *) binary);
        break;
      case CDF_REAL8:
      case CDF_DOUBLE: 
        v[i-s] = (double) *((double *) binary);
        break;
    }
    // Uncomment following to see that it DOES read the values successfully
    //kdDebug() << field << "[" << i << "]=" << v[i] << endl;

  }
  free(binary);
  kdDebug() << "Finished reading " << field << endl;
  
  status = CDFclose(id);

  return i-s;
}


bool CdfSource::isValidField(const QString& field) const {  
  return _fieldList.contains(field);
}


int CdfSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 1; // For 0-dimensional vars always the case I guess, but what are frames exactly ?
}


int CdfSource::frameCount(const QString& field) const {
  // Handle the case where the argument is null (return max number of records for all vars)
  if (field.isEmpty() || field == "INDEX") {
    return _maxFrameCount;
  }
  // Other case : count queried specifically for one field
  else { 
    return _frameCounts[field];
  }
}


QString CdfSource::fileType() const {
  return "CDF";
}


void CdfSource::save(QTextStream &ts, const QString& indent) {
  // FIXME (copied from ascii.cpp !)
  KstDataSource::save(ts, indent);
}


bool CdfSource::isEmpty() const {
  return frameCount() < 1;
}


extern "C" {
KstDataSource *create_cdf(KConfig *cfg, const QString& filename, const QString& type) {
  return new CdfSource(cfg, filename, type);
}

QStringList provides_cdf() {
  QStringList rc;
  rc += "CDF";
  return rc;
}

/** understands_cdf: returns true if:
    - the file is readable (!)
    - the file has a .cdf extension (required by the cdf lib)
    - CDFopen does not complain (currently disabled) **/
int understands_cdf(KConfig*, const QString& filename) {
  QFile f(filename);
  QFileInfo fInfo(f);

  if (!f.open(IO_ReadOnly)) {
    kdDebug() << "Unable to read file !" << endl;
    return 0;
  }
  
  if (fInfo.extension(false) != "cdf") {
    kdDebug() << "Wrong extension for CDF reader !" << endl;
    return 0;
  }

  return 76;
  // Commented out the following (though nice and robust) because it takes too long
  // Extension check should be enough :-) (?)
  /* CDFid id;
  CDFstatus status;
  status = CDFopen (fInfo.baseName(true).latin1(), &id);
  if (status < CDF_OK) {
    kdDebug() << "CDFlib unable to read the file !" << endl;
    return false;
  }
  else {
    status = CDFclose (id);
    return true;
  } */

}

// FIXME: implement fieldlist accelerator static here for better performance?

}

// vim: ts=2 sw=2 et
