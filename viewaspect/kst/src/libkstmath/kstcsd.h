/***************************************************************************
                          kstcsd.h: Cumulative Spectral Decay for KST
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by University of British Columbia
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

/** A class for handling cumulative spectral decay for kst
 */

#ifndef KSTCSD_H
#define KSTCSD_H

#include "kstdataobject.h"
#include "kst_export.h"


class KST_EXPORT KstCSD : public KstDataObject {
  public:
    KstCSD(const QString &in_tag, KstVectorPtr in_V,
           double in_freq, bool in_average, bool in_removeMean,
           bool in_apodize, int in_apodizeFxn, int in_windowSize, int in_length,
           double in_gaussianSigma, const QString& in_vectorUnits = QString::null, 
           const QString& in_rateUnits = QString::null);
    KstCSD(const QDomElement& e);
    virtual ~KstCSD();

    virtual UpdateType update(int update_counter = -1);

    virtual void save(QTextStream& ts, const QString& indent = QString::null);
    virtual QString propertyString() const;
    
    QString vTag() const;
    void setVector(KstVectorPtr);

    virtual bool slaveVectorsUsed() const;

    virtual void _showDialog();

    bool apodize() const;
    void setApodize(bool in_apodize);

    bool removeMean() const;
    void setRemoveMean(bool in_removeMean);

    bool average() const;
    void setAverage(bool in_average);

    double freq() const;
    void setFreq(double in_freq);
    
    int apodizeFxn() const;
    void setApodizeFxn(int in_fxn);
    
    double gaussianSigma() const;
    void setGaussianSigma(double in_sigma);
    
    int windowSize() const;
    void setWindowSize(int in_size); 
    
    int length() const;
    void setLength(int in_length);
    
    const QString& vectorUnits() const;
    void setVectorUnits(const QString& units);
    
    const QString& rateUnits() const;
    void setRateUnits(const QString& units);
    
    KstMatrixPtr outputMatrix() const;
    
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap);

  private:
    void commonConstructor(const QString &in_tag, KstVectorPtr in_V,
                           double in_freq, bool in_average, bool in_removeMean,
                           bool in_apodize, int in_apodizeFxn, int in_windowSize, int in_length,
                           double in_gaussianSigma, const QString& in_vectorUnits, const QString& in_rateUnits,
                           const QString& vecName);
    
    double _frequency;
    bool _average;
    bool _removeMean;
    bool _apodize;
    int _apodizeFxn;
    double _gaussianSigma;
    int _windowSize;
    int _length;
    QString _vectorUnits;
    QString _rateUnits;
    
    // output matrix
    KstMatrixMap::Iterator _outMatrix;
};

typedef KstSharedPtr<KstCSD> KstCSDPtr;
typedef KstObjectList<KstCSDPtr> KstCSDList;

#endif
// vim: ts=2 sw=2 et
