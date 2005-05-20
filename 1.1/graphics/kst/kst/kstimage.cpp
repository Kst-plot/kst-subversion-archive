/***************************************************************************
                   KstImage.cpp: Image class for Kst
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
#include "kstimage.h"

#include <kdebug.h>
#include <klocale.h>

#include <qstylesheet.h>

#include <math.h>

KstImage::KstImage(const QDomElement& e) : KstDataObject(e){
  QString in_tag, in_matrixName, in_paletteName;
  bool in_hasColorMap = false, in_hasContourMap = false;
  double in_zLower = 0, in_zUpper = 0;
  _autoThreshold = false;

  /* parse the DOM tree */
  QDomNode n = e.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if( !e.isNull() ) { // the node was really an element.
      if (e.tagName() == "tag") {
        in_tag = e.text();
      } else if (e.tagName() == "matrixtag") {
        in_matrixName = e.text();
      } else if (e.tagName() == "palettename") {
        in_paletteName = e.text();
      } else if (e.tagName() == "lowerthreshold") {
        in_zLower = e.text().toDouble();
      } else if (e.tagName() == "upperthreshold") {
        in_zUpper = e.text().toDouble();
      } else if (e.tagName() == "hascolormap") {
        in_hasColorMap = (e.text() != "0");
      } else if (e.tagName() == "hascontourmap") {
        in_hasContourMap = (e.text() != "0");
      } else if (e.tagName() == "numcontourlines") {
        _numContourLines = e.text().toInt();
      } else if (e.tagName() == "contourcolor") {
        _contourColor.setNamedColor(e.text());
      } else if (e.tagName() == "contourweight") {
        _contourWeight = e.text().toInt();
      } else if (e.tagName() == "autothreshold") {
        _autoThreshold = (e.text() != "0");
      }

    }
    n = n.nextSibling();
  }
  _matrix = 0L;
  _inputMatrixLoadQueue.append(qMakePair(QString("THEMATRIX"), in_matrixName));

  setTagName(in_tag);
  _typeString = i18n("Image");
  _hasColorMap = in_hasColorMap;
  _hasContourMap = in_hasContourMap;
  _zLower = in_zLower;
  _zUpper = in_zUpper;

  if (_hasColorMap) {
    KPalette *in_pal = new KPalette(in_paletteName);
    //maybe the palette doesn't exist anymore.  Generate a grayscale palette then.
    if (in_pal->nrColors() <= 0) {
      for (int i = 0; i < 256; i++) {
        in_pal->addColor(QColor(i,i,i));
      }
      KstDebug::self()->log(i18n("Unable to find palette %1.  Using a 256 color grayscale palette instead.").arg(in_paletteName),
                                 KstDebug::Warning);
    }
    _pal = in_pal;
  }

  if (!_hasColorMap) {
    setColorDefaults();
  }
  if (!_hasContourMap) {
    setContourDefaults();
  }
}


bool KstImage::loadInputs() {
  QValueList<QPair<QString,QString> >::Iterator i;
  KST::dataObjectList.lock().readLock();
  for (i = _inputMatrixLoadQueue.begin(); i != _inputMatrixLoadQueue.end(); ++i) {
    if ((*i).first == "THEMATRIX") {
      KstMatrixList matrices = kstObjectSubList<KstDataObject, KstMatrix>(KST::dataObjectList);
      _matrix = *(matrices.findTag((*i).second));
      if (!_matrix) {
        KstDebug::self()->log(i18n("Unable to find matrix for %2: [%1]").arg((*i).second).arg(tagName()), KstDebug::Warning);
        KST::dataObjectList.lock().readUnlock();
        return false;
      }
    }
  }
  KST::dataObjectList.lock().readUnlock();

  _matrix->readLock();
  setDirty();
  _matrix->readUnlock();
  return true;
}


//constructor for colormap only
KstImage::KstImage(const QString &in_tag, KstMatrixPtr in_matrix, double lowerZ, double upperZ, bool autoThreshold, KPalette* pal) : KstDataObject(){
  _matrix = in_matrix;
  setTagName(in_tag);
  _typeString = i18n("Image");
  _zLower = lowerZ;
  _zUpper = upperZ;
  _autoThreshold = autoThreshold;
  _pal = pal;
  _hasContourMap = false;
  _hasColorMap = true;

  setContourDefaults();
  setDirty();
}


//constructor for contour map only
KstImage::KstImage(const QString &in_tag, KstMatrixPtr in_matrix, int numContours, const QColor& contourColor, int contourWeight) : KstDataObject(){
  _matrix = in_matrix;
  setTagName(in_tag);
  _typeString = i18n("Image");
  _contourColor = contourColor;
  _numContourLines = numContours;
  _contourWeight = contourWeight;
  _hasContourMap = true;
  _hasColorMap = false;

  setColorDefaults();
  setDirty();

}


//constructor for both colormap and contour map
KstImage::KstImage(const QString &in_tag,
                   KstMatrixPtr in_matrix,
                   double lowerZ,
                   double upperZ,
                   bool autoThreshold,
                   KPalette* pal,
                   int numContours,
                   const QColor& contourColor,
                   int contourWeight) {
  _matrix = in_matrix;
  setTagName(in_tag);
  _typeString = i18n("Image");
  _contourColor = contourColor;
  _numContourLines = numContours;
  _contourWeight = contourWeight;
  _hasContourMap = true;
  _hasColorMap = true;
  _zLower = lowerZ;
  _zUpper = upperZ;
  _autoThreshold = autoThreshold;
  _pal = pal;
  setDirty();
}


KstImage::~KstImage() {
  delete _pal;
  _pal = 0L;
}


void KstImage::save(QTextStream &ts, const QString& indent) {
  QString l2 = indent + "  ";
  ts << indent << "<image>" << endl;
  ts << l2 << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << l2 << "<matrixtag>" << QStyleSheet::escape(_matrix->tagName()) << "</matrixtag>" << endl;
  ts << l2 << "<hascolormap>" << _hasColorMap << "</hascolormap>" <<endl;
  if (_pal) {
    ts << l2 << "<palettename>" << QStyleSheet::escape(_pal->name()) << "</palettename>" << endl;
  }
  ts << l2 << "<lowerthreshold>" << _zLower << "</lowerthreshold>" << endl;
  ts << l2 << "<upperthreshold>" << _zUpper << "</upperthreshold>" << endl;
  ts << l2 << "<hascontourmap>" << _hasContourMap << "</hascontourmap>" << endl;
  ts << l2 << "<numcontourlines>" << _numContourLines << "</numcontourlines>" << endl;
  ts << l2 << "<contourweight>" << _contourWeight << "</contourweight>" << endl;
  ts << l2 << "<contourcolor>" << QStyleSheet::escape(_contourColor.name()) << "</contourcolor>" << endl;
  ts << l2 << "<autothreshold>" << _autoThreshold << "</autothreshold>" << endl;
  ts << indent << "</image>" << endl;
}


KstObject::UpdateType KstImage::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  bool updated = UPDATE == _matrix->update(update_counter);
  if (updated || force) {
    NS = _matrix->sampleCount();

    //recalculate the thresholds if necessary
    if (_autoThreshold) {
      _zLower = _matrix->minZ();
      _zUpper = _matrix->maxZ();
    }

    //update the contour lines
    if (hasContourMap()) {
      double min = _matrix->minZ(), max = _matrix->maxZ();
      double contourStep  = (max - min) / (double)(_numContourLines + 1);
      if (contourStep > 0) {
        _contourLines.clear();
        for (int i = 0; i < _numContourLines; i++) {
          _contourLines.append(min + (i+1) * contourStep);
        }
      }
    }
    return setLastUpdateResult(UPDATE);
  }

  return setLastUpdateResult(NO_CHANGE);
}

QString KstImage::propertyString() const {
  return i18n("Using matrix %1" ).arg(_matrix->tagName());
}

QRgb KstImage::getMappedColor(double x, double y) {
  double z;
  if (_matrix->value(x,y,z)) {
    int index;
    if (_zUpper - _zLower != 0) {
      if (z > _zUpper) {
        index = _pal->nrColors() - 1;
      } else if (z < _zLower) {
        index = 0;
      } else {
        index = (int)floor(((z - _zLower) * (_pal->nrColors() - 1)) / (_zUpper - _zLower));
      }
    } else {
      index = 0;
    }
    return _pal->color(index).rgb();
  }
  return _pal->color(0).rgb();
}

void KstImage::setPalette(KPalette* pal) {
  delete _pal;
  _pal = pal;
}

void KstImage::_showDialog() {
  KstDialogs::showImageDialog(tagName());
}

void KstImage::setUpperThreshold(double z) {
  setDirty();
  _zLower = z;
}

void KstImage::setLowerThreshold(double z) {
  setDirty();
  _zUpper = z;
}

void KstImage::changeToColorOnly(const QString &in_tag, KstMatrixPtr in_matrix,
                                     double lowerZ, double upperZ, bool autoThreshold, KPalette* pal) {
  setTagName(in_tag);
  _matrix = in_matrix;
  _zLower = lowerZ;
  _zUpper = upperZ;
  _autoThreshold = autoThreshold;
  delete _pal;
  _pal = pal;
  _hasColorMap = true;
  _hasContourMap = false;
  setDirty();
}

void KstImage::changeToContourOnly(const QString &in_tag, KstMatrixPtr in_matrix,
                                       int numContours, const QColor& contourColor, int contourWeight) {
  setTagName(in_tag);
  _matrix = in_matrix;
  _numContourLines = numContours;
  _contourWeight = contourWeight;
  _contourColor = contourColor;
  _hasColorMap = false;
  _hasContourMap = true;
  if (_pal) {
    _lastPaletteName = _pal->name();
  }
  delete _pal;
  _pal = 0L;
  setDirty();
}

void KstImage::changeToColorAndContour(const QString &in_tag, KstMatrixPtr in_matrix,
                                               double lowerZ, double upperZ, bool autoThreshold, KPalette* pal,
                                               int numContours, const QColor& contourColor, int contourWeight) {
  setTagName(in_tag);
  _matrix = in_matrix;
  _zLower = lowerZ;
  _zUpper = upperZ;
  _autoThreshold = autoThreshold;
  delete _pal;
  _pal = pal;
  _numContourLines = numContours;
  _contourWeight = contourWeight;
  _contourColor = contourColor;
  _hasColorMap = true;
  _hasContourMap = true;
  setDirty();
}

void KstImage::matrixDimensions(double &x, double &y, double &width, double &height) {
  if (_matrix) {
    x = _matrix->minX();
    y = _matrix->minY();
    width = _matrix->xNumSteps() * _matrix->xStepSize();
    height = _matrix->yNumSteps() * _matrix->yStepSize();
  } else {
    x = y = width = height = 0;
  }
}

//this should check for duplicates
bool KstImage::addContourLine(double line) {
  _contourLines.append(line);
  setDirty();
  return true;
}

bool KstImage::removeContourLine(double line) {
  setDirty();
  return _contourLines.remove(line);
}

void KstImage::clearContourLines() {
  setDirty();
  _contourLines.clear();
}

bool KstImage::getNearestZ(double x, double y, double& z) {
  return _matrix->value(x,y,z);
}

QString KstImage::paletteName() const {
  if (_pal) {
    return _pal->name();
  }
  return _lastPaletteName;
}

void KstImage::setColorDefaults() {
  _zLower = 0;
  _zUpper = 100;
  _pal = 0L;
  setDirty();
}

void KstImage::setContourDefaults() {
  _contourColor = QColor("red");
  _numContourLines = 1;
  _contourWeight = 0;
  setDirty();
}

void KstImage::setAutoThreshold(bool yes) {
  _autoThreshold = yes;
  setDirty();
}

// vim: ts=2 sw=2 et
