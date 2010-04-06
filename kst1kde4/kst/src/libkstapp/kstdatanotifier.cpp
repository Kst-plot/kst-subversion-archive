/***************************************************************************
                             kstdatanotifier.cpp
                             -------------------
    begin                : Sep 13 2005
    copyright            : (C) 2005 The University of Toronto
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

#include "kstdatanotifier.h"

#include "kst.h"

#include <QColor>
#include <QTimer>
#include <QToolTip>

static const int delays[] = { 900, 675, 425, 300, 200, 100,  80,  40,  20 };
static const int dark[] =   { 100, 150, 200, 250, 300, 375, 450, 525, 600 };
static const unsigned int numIterations = sizeof(delays) / sizeof(int);


KstDataNotifier::KstDataNotifier(QWidget *parent)
: KLed(QColor(0, 255, 0), parent) {
  off();
  setShape(Rectangular);
  setLook(Sunken);
  _animationStage = 0;
  QToolTip::add(this, tr("Indicates that new data has arrived."));
  show();

  _colors = new QColor[numIterations];
  for(unsigned int i=0; i<numIterations; ++i) {
    _colors[i] = QColor(0, 255, 0).dark(dark[i]);
  }
}


KstDataNotifier::~KstDataNotifier() {
  delete[] _colors;
}


void KstDataNotifier::arrived() {
  if (state() == KLed::On) {
    _animationStage = 0;
    setColor(_colors[0]);
  } else {
    QTimer::singleShot(0, this, SLOT(animate()));
  }
}


void KstDataNotifier::animate() {
  if (state() == KLed::Off) {
    on();
    setColor(_colors[0]);
    _animationStage = 0;
    QTimer::singleShot(delays[_animationStage], this, SLOT(animate()));
    return;
  }

  if (++_animationStage < numIterations) {
    setColor(_colors[_animationStage]);
    QTimer::singleShot(delays[_animationStage], this, SLOT(animate()));
  } else {
    off();
  }
}

#include "kstdatanotifier.moc"
