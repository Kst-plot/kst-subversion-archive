/***************************************************************************
                          kstpsd.h: Power Spectra for KST
                             -------------------
    begin                : Fri Feb 10 2002
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

#ifndef KSTPSD_H
#define KSTPSD_H

#include "kstdataobject.h"
#include "kst_export.h"

class KST_EXPORT KstPSD : public KstDataObject {
  public:
    KstPSD(const QString& in_tag, KstVectorPtr in_V, double freq,
        bool average, int len,
        bool in_apodize, bool in_removeMean,
        const QString& VUnits, const QString& RUnits, int in_apodizeFxn = 0, double in_gaussianSigma = 3.0);
    KstPSD(const QDomElement& e);
    virtual ~KstPSD();

    virtual UpdateType update(int update_counter = -1);

    virtual void save(QTextStream& ts, const QString& indent = QString::null);
    virtual QString propertyString() const;

    static void createWindow(int apodizeFxn, double* w, int len, double gaussianSigma);
    
    bool apodize() const;
    void setApodize(bool in_apodize);
    
    int apodizeFxn() const;
    void setApodizeFxn(int in_apodizeFxn);
    
    double gaussianSigma() const;
    void setGaussianSigma(double in_gaussianSigma);

    bool removeMean() const;
    void setRemoveMean(bool in_removeMean);

    bool average() const;
    void setAverage(bool in_average);

    double freq() const;
    void setFreq(double in_freq);

    int len() const;
    void setLen(int in_len);

    QString vTag() const;
    void setVector(KstVectorPtr);

    const QString& vUnits() const;
    void setVUnits(const QString& units);
    const QString& rUnits() const;
    void setRUnits(const QString& units);

    virtual bool slaveVectorsUsed() const;

    virtual void _showDialog();

    virtual QString xVTag() const { return (*_fVector)->tagName(); }
    virtual QString yVTag() const { return (*_sVector)->tagName(); }

    KstVectorPtr vX() const { return *_fVector; }
    KstVectorPtr vY() const { return *_sVector; }

    const KstCurveHintList *curveHints() const;
    
    virtual KstDataObjectPtr makeDuplicate(KstDataObjectDataObjectMap& duplicatedMap);

  private:
    void generateWindow(int len);
    void commonConstructor(const QString& in_tag, KstVectorPtr in_V,
        double freq, bool average, int len,
        bool apodize, bool removeMean,
        const QString& VUnits, const QString& RUnits, int in_apodizeFxn, double in_gaussianSigma);

    void _adjustLengths();
    bool _Apodize;
    int _apodizeFxn;
    double _gaussianSigma;
    int _prevApodizeFxn;
    bool _RemoveMean;
    bool _Average;
    int _last_f0;
    int _last_n_subsets;
    int _last_n_new;
    double _Freq;
    int _Len;
    double *_w;
    int _PSDLen;
    double *_a;
    int _ALen;
    int _WLen;
    QString _vUnits;
    QString _rUnits;

    KstVectorMap::Iterator _sVector, _fVector;
    static const QString& INVECTOR;
    static const QString& SVECTOR;
    static const QString& FVECTOR;
};

typedef KstSharedPtr<KstPSD> KstPSDPtr;
typedef KstObjectList<KstPSDPtr> KstPSDList;

#endif
// vim: ts=2 sw=2 et
