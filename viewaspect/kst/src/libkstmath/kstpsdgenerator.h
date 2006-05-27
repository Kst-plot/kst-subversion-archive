/***************************************************************************
                  kstpsdgenerator.h: power spectra generator for kst
                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by C. Barth Netterfield
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

/** A class to generate power spectra - mainly for use by kst data objects
 *  Based on KstPSD
 */

#ifndef KSTPSDGENERATOR_H
#define KSTPSDGENERATOR_H

#include <qvaluevector.h>

class KstPSDGenerator {
  public:
    KstPSDGenerator(QValueVector<double>* inVector, double in_freq,
        bool in_average, int in_len,
        bool in_apodize, bool in_removeMean, int in_apodizeFxn, double in_gaussianSigma);

    ~KstPSDGenerator();

    void updateNow();

    bool apodize() const;
    void setApodize(bool in_apodize);
    
    int apodizeFxn() const;
    void setApodizeFxn(bool in_apodizeFxn);
    
    double gaussianSigma() const;
    void setGaussianSigma(double in_gaussianSigma);

    bool removeMean() const;
    void setRemoveMean(bool in_removeMean);

    bool average() const;
    void setAverage(bool in_average);

    double freq() const;
    void setFreq(double in_freq);

    int length() const;
    void setLength(int in_len);

    void setInputVector(QValueVector<double>* inVector);

    QValueVector<double>* frequencyVector() const { return _frequencyVector; }
    QValueVector<double>* powerVector() const { return _powerVector; }
    
    double frequencyVectorStep() const;

  private:
    void generateWindow(int len);
    void adjustLengths();
    double cabs2(double r, double i);
    bool _Apodize;
    int _apodizeFxn;
    // keep track of prev apodizeFxn to avoid redundant regenerations
    int _prevApodizeFxn;
    double _gaussianSigma; 
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
    double _sW;
    
    // pointer to input vector
    QValueVector<double>* _inputVector;
    
    // output vectors
    QValueVector<double>* _frequencyVector;
    QValueVector<double>* _powerVector;
    
    // info about _frequencyVector
    double _frequencyVectorStep;
};

#endif
// vim: ts=2 sw=2 et
