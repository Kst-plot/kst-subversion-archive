/***************************************************************************
                     kstimage.h: image type for kst
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

#ifndef KSTIMAGE_H
#define KSTIMAGE_H

#include <kpalette.h>

#include "kstmatrix.h"

/**A class for handling images for Kst
 *@author University of British Columbia
 */
class KstImage: public KstDataObject {
  public:
    //constructor for colormap only
    KstImage(const QString &in_tag, KstMatrixPtr in_matrix, double lowerZ, double upperZ, bool autoThreshold, KPalette* pal);
    //constructor for contour map only
    KstImage(const QString &in_tag, KstMatrixPtr in_matrix, int numContours, const QColor& contourColor, int contourWeight);
    //constructor for both colormap and contour map
    KstImage(const QString &in_tag,
        KstMatrixPtr in_matrix,
        double lowerZ,
        double upperZ,
        bool autoThreshold,
        KPalette* pal,
        int numContours,
        const QColor& contourColor,
        int contourWeight);

    KstImage(const QDomElement& e);
    virtual ~KstImage();

    virtual void _showDialog();
    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual UpdateType update(int update_counter = -1);
    virtual QString propertyString() const;
    virtual bool loadInputs();

    virtual int sampleCount() const { return NS; }
    virtual int samplesPerFrame() const { return 1; }

    virtual bool getNearestZ(double x, double y, double& z);
    virtual QRgb getMappedColor(double x, double y);
    virtual void setPalette(KPalette* pal);
    virtual void setUpperThreshold(double z);
    virtual void setLowerThreshold(double z);
    virtual void setAutoThreshold(bool yes);


    virtual double upperThreshold() const { return _zUpper; }
    virtual double lowerThreshold() const { return _zLower; }
    virtual bool autoThreshold() const { return _autoThreshold; }

    virtual QString matrixTag() const { return _matrix->tagName(); }
    virtual QString paletteName() const;

    virtual void matrixDimensions(double &x, double &y, double &width, double &height);

    virtual void changeToColorOnly(const QString &in_tag, KstMatrixPtr in_matrix,
        double lowerZ, double upperZ, bool autoThreshold, KPalette* pal);
    virtual void changeToContourOnly(const QString &in_tag, KstMatrixPtr in_matrix,
        int numContours, const QColor& contourColor, int contourWeight);
    virtual void changeToColorAndContour(const QString &in_tag, KstMatrixPtr in_matrix,
        double lowerZ, double upperZ, bool autoThreshold, KPalette* pal,
        int numContours, const QColor& contourColor, int contourWeight);

    //contour lines
    virtual int numContourLines() const { return _numContourLines; }
    virtual QValueList<double> contourLines() const { return _contourLines; }
    virtual bool addContourLine(double line);
    virtual bool removeContourLine(double line);
    virtual void clearContourLines();
    virtual const QColor& contourColor() const { return _contourColor; }
    virtual int contourWeight() const { return _contourWeight; } // a contour weight of -1 means variable weight

    //other properties
    virtual bool hasContourMap() const { return _hasContourMap; }
    virtual bool hasColorMap() const { return _hasColorMap; }

  protected:

    int NS;
    QValueList<QPair<QString,QString> > _inputMatrixLoadQueue;

  private:
    //use these to set defaults when either is not used.
    void setColorDefaults();
    void setContourDefaults();
    KstMatrixPtr _matrix;
    KPalette* _pal;
    //upper and lower thresholds
    double _zUpper;
    double _zLower;
    bool _autoThreshold;

    bool _hasColorMap;
    bool _hasContourMap;

    int _numContourLines;
    QValueList<double> _contourLines;
    QColor _contourColor;
    int _contourWeight; //_contourWeight = -1 means variable weight
    QString _lastPaletteName;
};


typedef KstSharedPtr<KstImage> KstImagePtr;
typedef KstObjectList<KstImagePtr> KstImageList;

#endif
// vim: ts=2 sw=2 et
