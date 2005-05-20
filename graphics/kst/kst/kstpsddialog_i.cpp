/***************************************************************************
                       kstpsddialog_i.cpp  -  Part of KST
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
#include <qspinbox.h>

// include files for KDE
#include <kcombobox.h>
#include <kdebug.h>
#include <kmessagebox.h>

// application specific inclues
#include "fftoptionswidget.h"
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "kstpsddialog_i.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "vectorselector.h"

#define DIALOGTYPE KstPsdDialogI
#define DTYPE "Power Spectrum"
#include "dataobjectdialog.h"

const QString& KstPsdDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstPsdDialogI> KstPsdDialogI::_inst = 0L;

KstPsdDialogI *KstPsdDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstPsdDialogI(KstApp::inst());
  }
  return _inst;
}


KstPsdDialogI::KstPsdDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstPsdDialog(parent, name, modal, fl) {
  Init();
  connect(_vector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
}

KstPsdDialogI::~KstPsdDialogI() {
  DP = 0L;
}

KstPSDPtr KstPsdDialogI::_getPtr(const QString& tagin) {
  KstPSDList c = kstObjectSubList<KstDataObject, KstPSD>(KST::dataObjectList);
  return *c.findTag(tagin);
}

void KstPsdDialogI::updateWindow() {
  _curvePlacement->update();
}

void KstPsdDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }

  DP->readLock();

  _tagName->setText(DP->tagName());

  _vector->setSelection(DP->vTag());

  // set sample rate, Units, FFT len, and vector units
  _kstFFTOptions->FFTLen->setValue(DP->len());
  _kstFFTOptions->SampRate->setText(QString::number(DP->freq()));

  _kstFFTOptions->VectorUnits->setText(DP->vUnits());
  _kstFFTOptions->RateUnits->setText(DP->rUnits());
  _kstFFTOptions->Apodize->setChecked(DP->apodize());
  _kstFFTOptions->RemoveMean->setChecked(DP->removeMean());
  _kstFFTOptions->Interleaved->setChecked(DP->average());

  DP->readUnlock();

  _curveAppearance->hide();
  _curvePlacement->hide();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstPsdDialogI::_fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _curvePlacement->update();
  _kstFFTOptions->update();

  // for some reason the lower widget needs to be shown first to prevent overlapping?
  _curveAppearance->hide();
  _curvePlacement->show();
  _curveAppearance->show();
  _curveAppearance->reset();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstPsdDialogI::update() {
  _curvePlacement->update();
  _vector->update();
}

/* returns true if succesful */
bool KstPsdDialogI::new_I() {
  QString tag_name = _tagName->text();
  if (tag_name == defaultTag) {
    tag_name = KST::suggestPSDName(_vector->selectedVector());
  }

  // verify that the curve name is unique
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (_vector->selectedVector().isEmpty()) {
    KMessageBox::sorry(this, i18n("New PSD not made: define vectors first."));
    return false;
  }

  KST::vectorList.lock().readLock();
  KstVectorPtr p = *KST::vectorList.findTag(_vector->selectedVector());
  KST::vectorList.lock().readUnlock();
  if (!p) {
    kdFatal() << "Bug in kst: the vector field in plotDialog (PSD) refers to "
              << "a non existant vector...." << endl;
  }

  // create the psd curve
  if (!_kstFFTOptions->checkValues()) {
    return false;
  } else {
    p->readLock();
    KstPSDPtr psd = new KstPSD(tag_name, p,
                            _kstFFTOptions->SampRate->text().toDouble(),
                            _kstFFTOptions->Interleaved->isChecked(),
                            _kstFFTOptions->FFTLen->text().toInt(),
                            _kstFFTOptions->Apodize->isChecked(),
                            _kstFFTOptions->RemoveMean->isChecked(),
                            _kstFFTOptions->VectorUnits->text(),
                            _kstFFTOptions->RateUnits->text());

    KstVCurvePtr vc = new KstVCurve(KST::suggestCurveName(tag_name,true), psd->vX(), psd->vY(), 0L, 0L, 0L, 0L, _curveAppearance->color());
    vc->setHasPoints(_curveAppearance->showPoints());
    vc->setHasLines(_curveAppearance->showLines());
    vc->setHasBars(_curveAppearance->showBars());
    vc->Point.setType(_curveAppearance->pointType());
    vc->setLineWidth(_curveAppearance->lineWidth());
    vc->setLineStyle(_curveAppearance->lineStyle());
    vc->setBarStyle(_curveAppearance->barStyle());
    vc->setPointDensity(_curveAppearance->pointDensity());

    Kst2DPlotPtr plot;
    KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_curvePlacement->_plotWindow->currentText()));
    if (!w) {
      QString n = KstApp::inst()->newWindow(KST::suggestWinName());
      w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
    }
    if (w) {
      if (_curvePlacement->existingPlot()) {
        // assign curve to plot
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(_curvePlacement->plotName()));
        if (plot) {
          plot->addCurve(vc.data());
        }
      }

      if (_curvePlacement->newPlot()) {
        // assign curve to plot
        QString name = w->createPlot<Kst2DPlot>(KST::suggestPlotName());
        if (_curvePlacement->reGrid()) {
          w->view()->cleanup(_curvePlacement->columns());
        }
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
        if (plot) {
          plot->setXAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay(), true, 0.0);
          plot->setYAxisInterpretation(false, KstAxisInterpretation(), KstAxisDisplay(), true, 0.0);
          _curvePlacement->update();
          _curvePlacement->setCurrentPlot(plot->tagName());
          plot->addCurve(vc.data());
          plot->GenerateDefaultLabels();
        }
      }
    }
    p->readUnlock();
    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.append(psd.data());
    KST::dataObjectList.append(vc.data());
    KST::dataObjectList.lock().writeUnlock();
    psd = 0L;
    vc = 0L;
    emit modified();
  }
  return true;
}

// returns true if succesful
bool KstPsdDialogI::edit_I() {
  // verify that the curve name is unique
  if (_tagName->text() != DP->tagName() && KST::dataTagNameNotUnique(_tagName->text())) {
    return false;
  }

  DP->writeLock();

  KST::vectorList.lock().readLock();
  DP->setVector(*KST::vectorList.findTag(_vector->selectedVector())); 
  KST::vectorList.lock().readUnlock();

  DP->setTagName(_tagName->text());

  if (!_kstFFTOptions->checkValues()) {
    DP->writeUnlock();
    return false;
  }

  DP->setFreq(_kstFFTOptions->SampRate->text().toDouble());
  DP->setLen(_kstFFTOptions->FFTLen->text().toInt());

  DP->setVUnits(_kstFFTOptions->VectorUnits->text());
  DP->setRUnits(_kstFFTOptions->RateUnits->text());

  DP->setApodize(_kstFFTOptions->Apodize->isChecked());
  DP->setRemoveMean(_kstFFTOptions->RemoveMean->isChecked());
  DP->setAverage(_kstFFTOptions->Interleaved->isChecked());

  DP->writeUnlock();

  emit modified();

  return true;
}

#include "kstpsddialog_i.moc"
// vim: ts=2 sw=2 et
