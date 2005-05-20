/***************************************************************************
                   kstmatrix.cpp: 2D matrix type for kst
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 University of British Columbia
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *   Permission is granted to link with any opensource library             *
 *                                                                         *
 ***************************************************************************/

#include "dialoglauncher.h"
#include "kstdatacollection.h"
#include "kstdebug.h"
#include "kstmatrix.h"

#include <qstylesheet.h>

#include <math.h>

#include <klocale.h>

KstMatrix::KstMatrix(const QString &in_tag, KstVectorPtr in_Z, uint nX, uint nY, double minX, double minY, double stepX, double stepY, bool useMaxX) : KstDataObject() {
  _zVector = in_Z;
  commonConstructor(in_tag, nX, nY, minX, minY, stepX, stepY, useMaxX);
  setDirty();
}


KstMatrix::KstMatrix(const QDomElement &e)
: KstDataObject(e) {
  QString in_tag, zVectorName, in_nX, in_nY, in_minX, in_minY, in_stepX, in_stepY;
  bool in_useMaxX = false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "zvectag") {
        zVectorName = e.text();
      } else if (e.tagName() == "nX") {
        in_nX = e.text();
      } else if (e.tagName() == "nY") {
        in_nY = e.text();
      } else if (e.tagName() == "useMaxX") {
        in_useMaxX = (e.text() != "0");
      } else if (e.tagName() == "minX") {
        in_minX = e.text();
      } else if (e.tagName() == "minY") {
        in_minY = e.text();
      } else if (e.tagName() == "stepX") {
        in_stepX = e.text();
      } else if (e.tagName() == "stepY") {
        in_stepY = e.text();
      }

    }
    n = n.nextSibling();
  }
  _inputVectorLoadQueue.append(qMakePair(QString("ZVECTOR"), zVectorName));

  commonConstructor(in_tag,
                    in_nX.toUInt(),
                    in_nY.toUInt(),
                    in_minX.toDouble(),
                    in_minY.toDouble(),
                    in_stepX.toDouble(),
                    in_stepY.toDouble(),
                    in_useMaxX);
}


bool KstMatrix::loadInputs() {
  QValueList<QPair<QString,QString> >::Iterator i;
  KST::vectorList.lock().readLock();
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    if ((*i).first == "ZVECTOR") {
      _zVector = *KST::vectorList.findTag((*i).second);
      if (!_zVector.data()) {
        KstDebug::self()->log(i18n("Unable to find Z vector for %2: [%1]").arg((*i).second).arg(tagName()), KstDebug::Warning);
        KST::vectorList.lock().readUnlock();
        return false;
      }
    }
  }
  KST::vectorList.lock().readUnlock();

  setDirty();
  return true;
}


void KstMatrix::commonConstructor(const QString &in_tag, uint nX, uint nY, double minX, double minY, double stepX, double stepY, bool useMaxX) {
  //assuming that nX*nY <= length of in_Z. Should have been checked by dialog.
  //If for some reason not, update() will reduce _nX
  _nX = nX;
  _nY = nY;
  _minX = minX;
  _minY = minY;
  _stepX = stepX;
  _stepY = stepY;
  _useMaxX = useMaxX;
  _typeString = i18n("Matrix");
  setTagName(in_tag);
}


KstMatrix::~KstMatrix() {
}


//return the value at the specified matrix position
//return false if the position is out of bounds,
//otherwise uses a close position
bool KstMatrix::value(double x, double y, double& z) {
  int x_index = (int)floor((x - _minX) / (double)_stepX);
  int y_index = (int)floor((y - _minY) / (double)_stepY);
  int index =  x_index * _nY + y_index;
  if (x_index >= _nX || x_index < 0 || y_index >= _nY || y_index < 0) {
    return false;
  }
  if (index >= NS || index < 0 ) {
    return false;
  }
  z = _zVector->value(index);
  return true;
}

//return the value at the specified matrix position
//return false if the position is out of bounds
bool KstMatrix::valueRaw(int x, int y, double& z) {
  int index =  x * _nY + y;
  if (x >= _nX || x < 0 || y >= _nY || y < 0) {
    return false;
  }
  if (index >= NS || index < 0 ) {
    return false;
  }
  z = _zVector->value(index);
  return true;
}

//change the matrix parameters
void KstMatrix::changeParameters(const QString &in_tag, KstVectorPtr in_Z, int nX, int nY, double minX, double minY, double stepX, double stepY, bool useMaxX) {

  //assuming that nX*nY <= length of in_Z. Should have been checked by dialog.
  //If for some reason not, update() will reduce _nX
  _zVector = in_Z;
  _nX = nX;
  _nY = nY;
  _minX = minX;
  _minY = minY;
  _stepX = stepX;
  _stepY = stepY;
  _useMaxX = useMaxX;
  setTagName(in_tag);
  setDirty();
}


void KstMatrix::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<matrix>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<zvectag>" << QStyleSheet::escape(_zVector->tagName()) << "</zvectag>" << endl;
  ts << l2 << "<nX>" << _nX << "</nX>" << endl;
  ts << l2 << "<nY>" << _nY << "</nY>" << endl;
  ts << l2 << "<useMaxX>" << _useMaxX << "</useMaxX>" << endl;
  ts << l2 << "<minX>" << _minX << "</minX>" << endl;
  ts << l2 << "<minY>" << _minY << "</minY>" << endl;
  ts << l2 << "<stepX>" << _stepX << "</stepX>" << endl;
  ts << l2 << "<stepY>" << _stepY << "</stepY>" << endl;
  ts << indent << "</matrix>" << endl;
}


KstObject::UpdateType KstMatrix::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  bool updated = UPDATE == _zVector->update(update_counter);
  if (updated || force) {
    if (_useMaxX || _nX*_nY > _zVector->length()) {
      _nX = (uint)floor(_zVector->length() / _nY);
    }

    NS = _nX * _nY;

    //find max and min within the grid
    if (NS > 0) {
      _minZ = _zVector->value(0);
      _maxZ = _zVector->value(0);
    } else {
      _minZ = 0;  //no grid, use some arbitrary value
      _maxZ = 0;
    }
    for (int i = 0; i < NS; i++) {
      if (_minZ > _zVector->value(i)) {
        _minZ = _zVector->value(i);
      }
      if (_maxZ < _zVector->value(i)) {
        _maxZ = _zVector->value(i);
      }
    }
    return setLastUpdateResult(UPDATE);
  }
  return setLastUpdateResult(NO_CHANGE);
}


KstVectorPtr KstMatrix::vector() const {
  return _zVector;
}


QString KstMatrix::propertyString() const {
  return i18n("Using vector %1").arg(_zVector->tagName());
}


void KstMatrix::_showDialog() {
  KstDialogs::showMatrixDialog(tagName());
}

// vim: ts=2 sw=2 et
