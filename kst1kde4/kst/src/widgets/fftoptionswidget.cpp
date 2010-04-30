/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>

#include "kstobjectdefaults.h"
#include "psdcalculator.h"

#include "fftoptionswidget.h"


KstFFTOptions::KstFFTOptions(QWidget *parent) : QWidget(parent) {
  setupUi(this);
  update();
}


KstFFTOptions::~KstFFTOptions( ) {
}


void KstFFTOptions::update() {
  KST::objectDefaults.sync();

  FFTLen->setValue(KST::objectDefaults.fftLen());
  SampRate->setText(QString::number(KST::objectDefaults.psdFreq()));
  VectorUnits->setText(KST::objectDefaults.vUnits());
  RateUnits->setText(KST::objectDefaults.rUnits());

  Apodize->setChecked(KST::objectDefaults.apodize());
  RemoveMean->setChecked(KST::objectDefaults.removeMean());
  Interleaved->setChecked(KST::objectDefaults.psdAverage());
  InterpolateHoles->setChecked(KST::objectDefaults.interpolateHoles());

  ApodizeFxn->setCurrentIndex(KST::objectDefaults.apodizeFxn());
  Output->setCurrentIndex(KST::objectDefaults.output());

  clickedInterleaved();
  clickedApodize();
  changedApodizeFxn();
}


void KstFFTOptions::changedApodizeFxn() {
  int gaussianIndex = WindowGaussian;

  //
  // if the first entry is empty then we are in edit multiple mode...
  //

  if (ApodizeFxn->itemText(0).isEmpty()) {
    ++gaussianIndex;
  }

  if (ApodizeFxn->currentIndex() == gaussianIndex && Apodize->isChecked()) {
    Sigma->setEnabled(true);
  } else {
    Sigma->setEnabled(false);
  }
}


void KstFFTOptions::clickedInterleaved() {
  FFTLen->setEnabled(Interleaved->isChecked());
}


void KstFFTOptions::clickedApodize() {
  ApodizeFxn->setEnabled(Apodize->isChecked());
}


void KstFFTOptions::synch() {
  clickedInterleaved();
  clickedApodize();
}


bool KstFFTOptions::checkValues() {
  double newFreq = SampRate->text().toDouble();
  int newLen = FFTLen->text().toInt();

  return checkGivenValues(newFreq, newLen);
}


bool KstFFTOptions::checkGivenValues(double sampRate, int FFTLen) {
  bool retVal = false;

  if (sampRate > 0) {
    if (FFTLen >= 2) {
      retVal = true;
    } else {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The FFT length must be greater than 2^2"));
    }
  } else {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("The sample rate must be greater than 0"));
  }

  return retVal;
}
