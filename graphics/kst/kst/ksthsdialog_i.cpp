/***************************************************************************
                      ksthsdialog_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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

// include files for Qt
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>

// include files for KDE
#include <kcombobox.h>
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "ksthsdialog_i.h"
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "kstsettings.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "vectorselector.h"

#define DIALOGTYPE KstHsDialogI
#define DTYPE "Histogram"
#include "dataobjectdialog.h"

const QString& KstHsDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstHsDialogI> KstHsDialogI::_inst;


KstHsDialogI *KstHsDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstHsDialogI(KstApp::inst());
  }
  return _inst;
}


KstHsDialogI::KstHsDialogI(QWidget* parent,
                           const char* name, bool modal, WFlags fl)
: KstHsDialog(parent, name, modal, fl) {
  Init();

  connect(AutoBin, SIGNAL(clicked()), this, SLOT(autoBin()));
  connect(_vector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_realTimeAutoBin, SIGNAL(clicked()), this, SLOT(updateButtons()));

  QColor qc = _curveAppearance->color();
  _curveAppearance->setValue(false, false, true, qc, 0, 0, 0, 1, 0);
}

KstHsDialogI::~KstHsDialogI() {
  DP = 0L;
}

KstHistogramPtr KstHsDialogI::_getPtr(const QString &tagin) {
  KstHistogramList c =
    kstObjectSubList<KstDataObject, KstHistogram>(KST::dataObjectList);

  return *c.findTag(tagin);
}

void KstHsDialogI::updateWindow() {
  _curvePlacement->update();
}

void KstHsDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }

  DP->readLock();

  _tagName->setText(DP->tagName());

  _vector->setSelection(DP->vTag());

  N->setValue(DP->nBins());
  Min->setText(QString::number(DP->vX()->min() - (DP->width()/2.0)));
  Max->setText(QString::number(DP->vX()->max() + (DP->width()/2.0)));
  _realTimeAutoBin->setChecked(DP->realTimeAutoBin());

  if (DP->isNormPercent()) {
    NormIsPercent->setChecked(true);
  } else if (DP->isNormFraction()) {
    NormIsFraction->setChecked(true);
  } else if (DP->isNormOne()) {
    PeakIs1->setChecked(true);
  } else {
    NormIsNumber->setChecked(true);
  }


  DP->readUnlock();
  updateButtons();

  // can't edit curve props from here....
  _curveAppearance->hide();
  _curvePlacement->hide();


  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstHsDialogI::_fillFieldsForNew() {
  // set tag name
  _tagName->setText(defaultTag);

  // set the curve placement window
  _curvePlacement->update();

  // for some reason the lower widget needs to be shown first to prevent overlapping?
  _curveAppearance->hide();
  _curvePlacement->show();
  _curveAppearance->show();
  _curveAppearance->reset();

  QColor qc = _curveAppearance->color();
  _curveAppearance->setValue(false, false, true, qc, 0, 0, 0, 1, 0);

  _realTimeAutoBin->setChecked(false);
  updateButtons();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstHsDialogI::update() {
  _curvePlacement->update();
  _vector->update();
}

bool KstHsDialogI::new_I() {
  QString tag_name = _tagName->text();
  if (tag_name == defaultTag) {
    tag_name = KST::suggestHistogramName(_vector->selectedVector());
  }

  // verify that the curve name is unique
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (_vector->selectedVector().isEmpty()) {
    KMessageBox::sorry(this, i18n("New Histogram not made: define "
                                "vectors first."));
    return false;
  }

  // find max and min
  double new_min = Min->text().toDouble();
  double new_max = Max->text().toDouble();
  if (new_max < new_min) {
    double m = new_max;
    new_max = new_min;
    new_min = m;
  }

  if (new_max == new_min) {
    KMessageBox::sorry(this, i18n("Max and Min can not be equal."));
    return false;
  }

  int new_n_bins = N->text().toInt();
  if (new_n_bins < 1) {
    KMessageBox::sorry(this, i18n("You must have one or more bins in "
                                "a histogram."));
    return false;
  }

  KstHsNormType new_norm_mode;
  if (NormIsPercent->isChecked()) {
    new_norm_mode = KST_HS_PERCENT;
  } else if (NormIsFraction->isChecked()) {
    new_norm_mode = KST_HS_FRACTION;
  } else if (PeakIs1->isChecked()) {
    new_norm_mode = KST_HS_MAX_ONE;
  } else {
    new_norm_mode = KST_HS_NUMBER;
  }

  KstHistogramPtr hs;

  KST::vectorList.lock().readLock();
  KstVectorList::Iterator i = KST::vectorList.findTag(_vector->selectedVector());
  KST::vectorList.lock().readUnlock();
  if (i == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the Vector field in plotDialog (Hs) refers to "
              << " a non existant vector..." << endl;
  }

  (*i)->readLock();
  hs = new KstHistogram(tag_name, *i, new_min, new_max,
                        new_n_bins, new_norm_mode);
  hs->setRealTimeAutoBin(_realTimeAutoBin->isChecked());

  KstVCurvePtr vc = new KstVCurve(KST::suggestCurveName(tag_name, true), hs->vX(), hs->vY(), 0L, 0L, 0L, 0L, _curveAppearance->color());

  vc->setHasPoints(_curveAppearance->showPoints());
  vc->setHasLines(_curveAppearance->showLines());
  vc->setHasBars(_curveAppearance->showBars());
  vc->Point.setType(_curveAppearance->pointType());
  vc->setLineWidth(_curveAppearance->lineWidth());
  vc->setLineStyle(_curveAppearance->lineStyle());
  vc->setBarStyle(_curveAppearance->barStyle());
  vc->setPointDensity(_curveAppearance->pointDensity());

  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_curvePlacement->_plotWindow->currentText()));
  if (!w) {
    QString n = KstApp::inst()->newWindow(KST::suggestWinName());
    w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
  }
  if (w) {
    Kst2DPlotPtr plot;
    if (_curvePlacement->existingPlot()) {
      /* assign curve to plot */
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(_curvePlacement->plotName()));
      if (plot) {
        plot->addCurve(vc.data());
      }
    }

    if (_curvePlacement->newPlot()) {
      /* assign curve to plot */
      QString name = w->createPlot<Kst2DPlot>(KST::suggestPlotName());
      if (_curvePlacement->reGrid()) {
        w->view()->cleanup(_curvePlacement->columns());
      }
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
      if (plot) {
        _curvePlacement->update();
        _curvePlacement->setCurrentPlot(plot->tagName());
        plot->addCurve(vc.data());
        plot->GenerateDefaultLabels();
      }
    }
  }
  (*i)->readUnlock();

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(hs.data());
  KST::dataObjectList.append(vc.data());
  KST::dataObjectList.lock().writeUnlock();

  hs = 0L;
  vc = 0L;
  emit modified();

  return true;
}


bool KstHsDialogI::edit_I() {
  // verify that the curve name is unique
  QString tag_name = _tagName->text();
  if (tag_name != DP->tagName() && KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  // find max and min
  double new_min = Min->text().toDouble();
  double new_max = Max->text().toDouble();
  if (new_max < new_min) {
    double m = new_max;
    new_max = new_min;
    new_min = m;
  }

  if (new_max == new_min) {
    KMessageBox::sorry(this, i18n("Max and Min can not be equal."));
    Min->setFocus();
    return false;
  }

  int new_n_bins = N->text().toInt();
  if (new_n_bins < 1) {
    KMessageBox::sorry(this, i18n("You must have one or more bins in a histogram."));
    N->setFocus();
    return false;
  }

  DP->writeLock();

  KST::vectorList.lock().readLock();
  DP->setVector(*KST::vectorList.findTag(_vector->selectedVector()));
  KST::vectorList.lock().readUnlock();

  DP->setTagName(tag_name);

  DP->setNBins(new_n_bins);
  DP->setXRange(new_min, new_max);
  DP->setRealTimeAutoBin(_realTimeAutoBin->isChecked());

  if (NormIsPercent->isChecked()) {
    DP->setIsNormPercent();
  } else if (NormIsFraction->isChecked()) {
    DP->setIsNormFraction();
  } else if (PeakIs1->isChecked()) {
    DP->setIsNormOne();
  } else {
    DP->setIsNormNum();
  }

  DP->setDirty();

  DP->writeUnlock();

  emit modified();

  return true;
}

void KstHsDialogI::autoBin() {
  KstReadLocker ml(&KST::vectorList.lock());

  if (!KST::vectorList.isEmpty()) {
    KstVectorList::Iterator i = KST::vectorList.findTag(_vector->selectedVector());
    double max, min;
    int n;
    
    if (i == KST::vectorList.end()) {
      kdFatal() << "Bug in kst: the Vector field in hsdialog refers to "
                   "a non existant vector..." << endl;
    }
    (*i)->readLock(); // Hmm should we really lock here?  AutoBin should I think
    KstHistogram::AutoBin(KstVectorPtr(*i), &n, &max, &min);
    (*i)->readUnlock();

    N->setValue(n);
    Min->setText(QString::number(min));
    Max->setText(QString::number(max));
  }
}

void KstHsDialogI::updateButtons() {
  if (_realTimeAutoBin->isChecked()) {
    autoBin();
  }

  Min->setEnabled(!_realTimeAutoBin->isChecked());
  Max->setEnabled(!_realTimeAutoBin->isChecked());
  N->setEnabled(!_realTimeAutoBin->isChecked());
  AutoBin->setEnabled(!_realTimeAutoBin->isChecked());
}

#include "ksthsdialog_i.moc"
// vim: ts=2 sw=2 et
