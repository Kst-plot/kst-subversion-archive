/***************************************************************************
                          histogram.h: Histogram for KST
                             -------------------
    begin                : Wed July 11 2002
    copyright            : (C) 2002 by C. Barth Netterfield
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

/** A class for handling power spectra for kst

 *@author C. Barth Netterfield
 */
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "dataobject.h"
#include "kst_export.h"

namespace Kst {

enum HsNormType { KST_HS_NUMBER, KST_HS_PERCENT, KST_HS_FRACTION, KST_HS_MAX_ONE};

class ObjectStore;

class KST_EXPORT Histogram : public DataObject {
  Q_OBJECT

  public:
    static const QString staticTypeString;
    const QString& typeString() const { return staticTypeString; }

    virtual UpdateType update(int update_counter = -1);
    virtual void save(QTextStream &ts, const QString& indent = QString::null);
    virtual QString propertyString() const;

    int nBins() const;
    void setNBins(int in_n_bins);

    void setXRange(double xmin_in, double xmax_in);

    QString vTag() const;

    void setVector(VectorPtr);

    virtual QString yLabel() const;
    virtual QString xLabel() const;

    bool isNormNum()        const { return _NormMode == KST_HS_NUMBER; }
    void setIsNormNum()           { _NormMode = KST_HS_NUMBER; }
    bool isNormPercent()    const { return _NormMode == KST_HS_PERCENT; }
    void setIsNormPercent()       { _NormMode = KST_HS_PERCENT; }
    bool isNormFraction()   const { return _NormMode == KST_HS_FRACTION; }
    void setIsNormFraction()      { _NormMode = KST_HS_FRACTION; }
    bool isNormOne()        const { return _NormMode == KST_HS_MAX_ONE; }
    void setIsNormOne()           { _NormMode = KST_HS_MAX_ONE; }

    static void AutoBin(const VectorPtr, int *n, double *max, double *min);

    virtual void showNewDialog();
    virtual void showEditDialog();

    virtual bool slaveVectorsUsed() const;

    virtual ObjectTag xVTag() const {return _bVector->tag();}
    virtual ObjectTag yVTag() const {return _hVector->tag();}

    void setRealTimeAutoBin(bool autobin);
    bool realTimeAutoBin() const;

    VectorPtr vX() const { return _bVector; }
    VectorPtr vY() const { return _hVector; }

    double xMin() const { return _MinX; }
    double xMax() const { return _MaxX; }
    double width() const { return _W; }

    double vMax() const;
    double vMin() const;
    int vNumSamples() const;

    virtual DataObjectPtr makeDuplicate(DataObjectDataObjectMap& duplicatedMap);

  protected:
    Histogram(ObjectStore *store, const ObjectTag &in_tag, VectorPtr in_V,
        double xmin_in, double xmax_in,
        int in_n_bins,
        HsNormType new_norm_in);
    Histogram(ObjectStore *store, const QDomElement &e);
    virtual ~Histogram();

  private:
    HsNormType _NormMode;
    VectorPtr _bVector, _hVector;
    double _MaxX;
    double _MinX;
    int _NS;
    int _NBins;
    unsigned long *_Bins;
    double _Normalization;
    double _W;
    bool _realTimeAutoBin;

    void commonConstructor(ObjectStore *store,
        VectorPtr in_V,
        double xmin_in, double xmax_in,
        int in_n_bins,
        HsNormType in_norm);
    void internalSetNBins(int in_n_bins);
};

typedef SharedPtr<Histogram> HistogramPtr;
typedef ObjectList<Histogram> HistogramList;

}

#endif
// vim: ts=2 sw=2 et
