/***************************************************************************
                          normalization.h
                          -------------------
    begin                : 08/15/08
    copyright            : (C) 2008 The University of British Columbia
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

#ifndef NORMALIZATION_H
#define NORMALIZATION_H

#include <kstbasicplugin.h>

class SpectralNormalization : public KstBasicPlugin {
Q_OBJECT
public:

  SpectralNormalization(QObject *parent, const char *name, const QStringList &args);
  SpectralNormalization(QObject *parent, const QStringList &args);
  virtual ~SpectralNormalization();

  virtual bool algorithm();

  virtual QStringList inputVectorList() const;
  virtual QStringList inputScalarList() const;
  virtual QStringList inputStringList() const;
  virtual QStringList outputVectorList() const;
  virtual QStringList outputScalarList() const;
  virtual QStringList outputStringList() const;

private:
  void fit(int k, int p, int iLength, double arr[], double cof[], KstVectorPtr vector_out);
  bool isMax(double arr[], int pos, int iLength);
  bool isMin(double arr[], int pos, int iLength);
  void excludePts(double arr[], int i, int w, int iLength);
  void searchHighPts(double arr[], int iLength);
  void interpolate(double Yi[], double arr[], int iLength);
};

#endif
