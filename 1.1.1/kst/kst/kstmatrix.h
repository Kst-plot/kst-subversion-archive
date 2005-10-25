/***************************************************************************
                     kstmatrix.h: 2D matrix type for kst
                             -------------------
    begin                : Mon July 19 2004
    copyright            : (C) 2004 by University of British Columbia
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

#ifndef KSTMATRIX_H
#define KSTMATRIX_H

#include "kstdataobject.h"

class KstMatrix: public KstDataObject {
  public:
    KstMatrix(const QString &in_tag, KstVectorPtr in_Z, uint nX, uint nY, double minX, double minY, double stepX, double stepY, bool useMaxX);
    KstMatrix(const QDomElement &e);
    virtual ~KstMatrix();

    virtual void _showDialog();
    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual UpdateType update(int update_counter = -1);
    virtual QString propertyString() const;

    virtual bool loadInputs();

    //return the value at the specified matrix position
    //return false if the position does not exist
    virtual bool value(double x, double y, double& z);
    virtual bool valueRaw(int x, int y, double& z);
    
    //Returns the total number of samples (x times y) in the matrix
    virtual int sampleCount() const { return NS; }

    //change parameters
    virtual void changeParameters(const QString &in_tag, KstVectorPtr in_Z, int nX, int nY, double minX, double minY, double stepX, double stepY, bool useMaxX);

    //return some stats on the z values
    virtual double minZ() const { return _minZ; }
    virtual double maxZ() const { return _maxZ; }

    //some functions to get the matrix parameters; mostly used for dialogs
    virtual int xNumSteps() const { return _nX; }
    virtual int yNumSteps() const { return _nY; }
    virtual double xStepSize() const { return _stepX; }
    virtual double yStepSize() const { return _stepY; }
    virtual double minX() const { return _minX; }
    virtual double minY() const { return _minY; }
    virtual bool useMaxX() const { return _useMaxX; }
    virtual QString zVectorTag() const { return _zVector->tagName(); }

    KstVectorPtr vector() const;

  protected:
    int NS;

  private:
    KstVectorPtr _zVector;
    int _nX;  //this can be 0
    int _nY;  //this should never be 0
    double _minX;
    double _minY;
    double _stepX;
    double _stepY;
    double _minZ;
    double _maxZ;
    bool _useMaxX;
    void commonConstructor(const QString &in_tag, uint nX, uint nY, double minX, double minY, double stepX, double stepY, bool useMaxX);
};

typedef KstSharedPtr<KstMatrix> KstMatrixPtr;
typedef KstObjectList<KstMatrixPtr> KstMatrixList;
typedef KstObjectMap<KstMatrixPtr> KstMatrixMap;

#endif
// vim: ts=2 sw=2 et
