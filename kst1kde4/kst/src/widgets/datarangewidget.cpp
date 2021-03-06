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

#include "enodes.h"
#include "kstdateparser.h"
#include "kstmath.h"
#include "kstvectordefaults.h"
#include "kst_export.h"
#include "datarangewidget.h"
#include "timedefinitions.h"

KstDataRange::KstDataRange(QWidget *parent) : QWidget(parent) {
  setupUi(this);

  connect(F0, SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
  connect(N, SIGNAL(textChanged(const QString&)), this, SLOT(modified()));
  connect(_startUnits, SIGNAL(activated(int)), this, SLOT(modified()));
  connect(_rangeUnits, SIGNAL(activated(int)), this, SLOT(modified()));
  connect(CountFromEnd, SIGNAL(clicked()), this, SLOT(modified()));
  connect(CountFromEnd, SIGNAL(clicked()), this, SLOT(clickedCountFromEnd()));
  connect(ReadToEnd, SIGNAL(clicked()), this, SLOT(modified()));
  connect(ReadToEnd, SIGNAL(clicked()), this, SLOT(clickedReadToEnd()));
  connect(DoSkip, SIGNAL(clicked()), this, SLOT(modified()));
  connect(DoSkip, SIGNAL(clicked()), this, SLOT(clickedDoSkip()));
  connect(DoFilter, SIGNAL(clicked()), this, SLOT(modified()));
  connect(Skip, SIGNAL(valueChanged(int)), this, SLOT(modified()));

  _time = false;
  update();
}


KstDataRange::~KstDataRange() {
}


void KstDataRange::modified() {
  emit changed();
}


void KstDataRange::clickedCountFromEnd() {
  if (CountFromEnd->isChecked()) {
    N->setEnabled(true);
	  _rangeUnits->setEnabled(true);
	  F0->setEnabled(false);
	  _startUnits->setEnabled(false);
	  ReadToEnd->setChecked(false);
  } else {
	  F0->setEnabled(true);
	  _startUnits->setEnabled(true);
  }
}


void KstDataRange::clickedReadToEnd() {
  if (ReadToEnd->isChecked()) {
	  F0->setEnabled(true);
	  _startUnits->setEnabled(true);
	  N->setEnabled(false);
	  _rangeUnits->setEnabled(false);
	  CountFromEnd->setChecked(false);
  } else {
	  N->setEnabled(true);
	  if (N->text() == "") {
	    N->setText("100");
	  }
	  _rangeUnits->setEnabled(true);
  }
}


void KstDataRange::clickedDoSkip() {
  if (DoSkip->isChecked()) {
	  Skip->setEnabled(true);
	  DoFilter->setEnabled(true);
  } else {
	  Skip->setEnabled(false);
	  DoFilter->setEnabled(false);
  }
}


void KstDataRange::updateEnables() {
  if (DoSkip->isChecked()) {
	  Skip->setEnabled(true);
	  DoFilter->setEnabled(true);
  } else {
	  Skip->setEnabled(false);
	  DoFilter->setEnabled(false);
  }

  if (CountFromEnd->isChecked()) {
	  N->setEnabled(true);
	  _rangeUnits->setEnabled(true);
	  F0->setEnabled(false);
	  _startUnits->setEnabled(false);
	  ReadToEnd->setChecked(false);
  } else if (ReadToEnd->isChecked()) {
	  F0->setEnabled(true);
	  _startUnits->setEnabled(true);
	  N->setEnabled(false);
	  _rangeUnits->setEnabled(false);
  } else {
	  N->setEnabled(true);
	  _rangeUnits->setEnabled(true);
	  F0->setEnabled(true);
	  _startUnits->setEnabled(true);
  }
}


void KstDataRange::update() {
  CountFromEnd->setChecked(KST::vectorDefaults.countFromEOF());
  ReadToEnd->setChecked(KST::vectorDefaults.readToEOF());
  F0->setText(QString::number(KST::vectorDefaults.f0(), 'g', 15));
  if (KST::vectorDefaults.n() >= 0) {
    N->setText(QString::number(KST::vectorDefaults.n(), 'g', 15));
  } else {
    N->setText("");
  }
  Skip->setValue(KST::vectorDefaults.skip());
  DoSkip->setChecked(KST::vectorDefaults.doSkip());
  DoFilter->setChecked(KST::vectorDefaults.doAve());

  clickedCountFromEnd();
  clickedReadToEnd();
  clickedDoSkip();
}


void KstDataRange::setAllowTime(bool allow) {
  if (allow != _time) {
	  _time = allow;
	  _startUnits->clear();
	  _startUnits->addItem(i18n(KST::timeDefinitions[0].name, KST::timeDefinitions[0].description));
	  _rangeUnits->clear();
	  _rangeUnits->addItem(i18n(KST::timeDefinitions[0].name, KST::timeDefinitions[0].description));
	  if (_time) {
	    for (int i = 1; KST::timeDefinitions[i].name; ++i) {
		    _startUnits->addItem(i18n(KST::timeDefinitions[i].name, KST::timeDefinitions[i].description));
		    if (i != KST::dateTimeEntry) {
		      _rangeUnits->addItem(i18n(KST::timeDefinitions[i].name, KST::timeDefinitions[i].description));
		    }
	    }
	  }
  }
}


void KstDataRange::setF0Value(double v) {
  F0->setText(QString::number(v, 'g', 15));
}


void KstDataRange::setNValue(double v) {
  N->setText(QString::number(v, 'g', 15));
}


double KstDataRange::interpret(const char *txt) {
  return Equation::interpret(txt);
}


double KstDataRange::f0Value() {
  const int cur = _startUnits->currentIndex();
  
  if (cur == KST::dateTimeEntry) {
	  KST::ExtDateTime edt = KST::parsePlanckDate(F0->text(), true);
	  if (edt.isNull()) {
	    return KST::NOPOINT;
	  }
	
    return KST::extDateTimeToMilliseconds(edt);
  }
  
  return ::d2i(interpret(F0->text().toLatin1()) * KST::timeDefinitions[cur].factor);
}


double KstDataRange::nValue() {
  int cur = _rangeUnits->currentIndex();

  if (cur >= KST::dateTimeEntry) {
	  ++cur; // we don't allow "date"
  }
  
  return ::d2i(interpret(N->text().toLatin1()) * KST::timeDefinitions[cur].factor);
}


KST::ExtDateTime KstDataRange::f0DateTimeValue() {
  const int cur = _startUnits->currentIndex();
  
  KST::ExtDateTime edt;
  if (cur == KST::dateTimeEntry) {
	  edt = KST::parsePlanckDate(F0->text(), true);
  }
  
  return edt;
}


bool KstDataRange::isStartRelativeTime() {
  return _startUnits->currentIndex() > KST::dateTimeEntry;
}


bool KstDataRange::isStartAbsoluteTime() {
  return _startUnits->currentIndex() == KST::dateTimeEntry;
}


bool KstDataRange::isRangeRelativeTime() {
  return _rangeUnits->currentIndex() > KST::dateTimeEntry;
}

