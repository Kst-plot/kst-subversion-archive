/***************************************************************************
                      kstcurvedialog_i.cpp  -  Part of KST
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

// include files for KDE
#include <kcombobox.h>
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "kstcurvedialog_i.h"
#include "kstviewwindow.h"
#include "kstuinames.h"
#include "vectorselector.h"

#define DIALOGTYPE KstCurveDialogI
#define DTYPE "Curve"
#include "dataobjectdialog.h"

const QString& KstCurveDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstCurveDialogI> KstCurveDialogI::_inst;

KstCurveDialogI *KstCurveDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstCurveDialogI(KstApp::inst());
  }
  return _inst;
}


KstCurveDialogI::KstCurveDialogI(QWidget* parent,
                                 const char* name, bool modal, WFlags fl)
  : KstCurveDialog(parent, name, modal, fl) {
  Init();

  connect(_xVector, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_yVector, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_xError, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_yError, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_xMinusError, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_yMinusError, SIGNAL(newVectorCreated(const QString&)),
          this, SIGNAL(modified()));
  connect(_checkBoxXMinusSameAsPlus, SIGNAL(toggled(bool)), this, SLOT(toggledXErrorSame(bool)));
  connect(_checkBoxYMinusSameAsPlus, SIGNAL(toggled(bool)), this, SLOT(toggledYErrorSame(bool)));

  _xError->provideNoneVector(true);
  _yError->provideNoneVector(true);
  _xMinusError->provideNoneVector(true);
  _yMinusError->provideNoneVector(true);

  _checkBoxXMinusSameAsPlus->setChecked(true);
  _checkBoxYMinusSameAsPlus->setChecked(true);
}

KstCurveDialogI::~KstCurveDialogI() {
  DP = 0L;
}

KstVCurvePtr KstCurveDialogI::_getPtr(const QString &tagin) {
  KstVCurveList c = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  // Will be null if not found
  return *c.findTag(tagin);
}

void KstCurveDialogI::updateWindow() {
  _curvePlacement->update();
}

void KstCurveDialogI::toggledXErrorSame(bool on) {
  textLabelXMinus->setEnabled(!on);
  _xMinusError->setEnabled(!on);
}

void KstCurveDialogI::toggledYErrorSame(bool on) {
  textLabelYMinus->setEnabled(!on);
  _yMinusError->setEnabled(!on);
}

void KstCurveDialogI::_fillFieldsForEdit() {
  if (DP != 0L) {
    DP->readLock();

    _tagName->setText(DP->tagName());

    _xVector->setSelection(DP->xVTag());
    _yVector->setSelection(DP->yVTag());
    _xError->setSelection(DP->xETag());
    _yError->setSelection(DP->yETag());
    _xMinusError->setSelection(DP->xEMinusTag());
    _yMinusError->setSelection(DP->yEMinusTag());

    if (DP->xEMinusTag() == DP->xETag()) {
      _checkBoxXMinusSameAsPlus->setChecked(true);
    } else {
      _checkBoxXMinusSameAsPlus->setChecked(false);
    }
    if (DP->yEMinusTag() == DP->yETag()) {
      _checkBoxYMinusSameAsPlus->setChecked(true);
    }  else {
      _checkBoxYMinusSameAsPlus->setChecked(false);
    }

    _curveAppearance->setValue(DP->hasLines(), DP->hasPoints(),
        DP->hasBars(), DP->color(),
        DP->Point.type(), DP->lineWidth(),
        DP->lineStyle(), DP->barStyle(), DP->pointDensity());

    _checkBoxIgnoreAutoscale->setChecked(DP->ignoreAutoScale());

    DP->readUnlock();

    _curvePlacement->hide();

    adjustSize();
    resize(minimumSizeHint());
    setFixedHeight(height());
  }
}

void KstCurveDialogI::_fillFieldsForNew() {
  KstVCurveList curves =
    kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  _tagName->setText(defaultTag);
  _curvePlacement->update();

  // for some reason the lower widget needs to be shown first to prevent overlapping?
  _curveAppearance->hide();
  _curvePlacement->show();
  _curveAppearance->show();
  _curveAppearance->reset();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void KstCurveDialogI::update() {
  _curvePlacement->update();
  _xVector->update();
  _yVector->update();
  _xError->update();
  _yError->update();
  _xMinusError->update();
  _yMinusError->update();
}

bool KstCurveDialogI::new_I() {
  //KstWriteLocker ml(&KST::vectorList.lock());
  if (_xVector->selectedVector().isEmpty() ||
      _yVector->selectedVector().isEmpty()) {
    KMessageBox::sorry(this, i18n("New curve not made: define vectors first."));
    return false;
  }

  KstVectorList::Iterator VX, VY;
  KstVectorList::Iterator EX, EY, EXMinus, EYMinus;

  // find VX and VY
  KST::vectorList.lock().readLock();
  VX = KST::vectorList.findTag(_xVector->selectedVector());
  VY = KST::vectorList.findTag(_yVector->selectedVector());
  EX = KST::vectorList.findTag(_xError->selectedVector());
  EY = KST::vectorList.findTag(_yError->selectedVector());
  if(_checkBoxXMinusSameAsPlus->isChecked()) {
    EXMinus = EX;
  } else {
    EXMinus = KST::vectorList.findTag(_xMinusError->selectedVector());
  }
  if(_checkBoxYMinusSameAsPlus->isChecked()) {
    EYMinus = EY;
  } else {
    EYMinus = KST::vectorList.findTag(_yMinusError->selectedVector());
  }
  if (VX == KST::vectorList.end() || VY == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the XVector field in plotDialog refers to "
              << "a non existent vector...." << endl;
  }

  // readlock the vectors, bercause when we update the curve, they get read
  KstReadLocker rl1(*VX), rl2(*VY);
  bool haveEx = (EX != KST::vectorList.end());
  bool haveEy = (EY != KST::vectorList.end());
  bool haveExMinus = (EXMinus != KST::vectorList.end());
  bool haveEyMinus = (EYMinus != KST::vectorList.end());
  if (haveEx) {
    (*EX)->readLock();
  }
  if (haveEy) {
    (*EY)->readLock();
  }
  if (haveExMinus) {
    (*EXMinus)->readLock();
  }
  if (haveEyMinus) {
    (*EYMinus)->readLock();
  }

  QString tag_name = _tagName->text();
  if (tag_name == defaultTag) {
    tag_name = KST::suggestCurveName((*VY)->tagName());
  }

  // verify that the curve name is unique
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    KST::vectorList.lock().readUnlock();
    if (haveEx) {
      (*EX)->readUnlock();
    }
    if (haveEy) {
      (*EY)->readUnlock();
    }
    if (haveExMinus) {
      (*EXMinus)->readUnlock();
    }
    if (haveEyMinus) {
      (*EYMinus)->readUnlock();
    }
    return false;
  }

  // create the curve
  KstVCurvePtr curve = new KstVCurve(tag_name, *VX, *VY,
                        haveEx ? *EX : KstVectorPtr(0L),
                        haveEy ? *EY : KstVectorPtr(0L),
                        haveExMinus ? *EXMinus : KstVectorPtr(0L),
                        haveEyMinus ? *EYMinus : KstVectorPtr(0L),
                        _curveAppearance->color());
  KST::vectorList.lock().readUnlock();

  curve->setHasPoints(_curveAppearance->showPoints());
  curve->setHasLines(_curveAppearance->showLines());
  curve->setHasBars(_curveAppearance->showBars());
  curve->setLineWidth(_curveAppearance->lineWidth());
  curve->setLineStyle(_curveAppearance->lineStyle());
  curve->Point.setType(_curveAppearance->pointType());
  curve->setBarStyle(_curveAppearance->barStyle());
  curve->setPointDensity(_curveAppearance->pointDensity());
  curve->setIgnoreAutoScale(_checkBoxIgnoreAutoscale->isChecked());

  if (_curvePlacement->existingPlot() || _curvePlacement->newPlot()) {
    KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_curvePlacement->_plotWindow->currentText()));
    if (!w) {
      QString n = KstApp::inst()->newWindow(KST::suggestWinName());
      w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
    }
    if (w) {
      Kst2DPlotPtr plot;
      if (_curvePlacement->existingPlot()) {
        // assign curve to plot
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(_curvePlacement->plotName()));
        if (plot) {
          plot->addCurve(curve.data());
        }
      }

      if (_curvePlacement->newPlot()) {
        // assign curve to plot
        QString name = w->createPlot<Kst2DPlot>(KST::suggestPlotName());
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
        if (_curvePlacement->reGrid()) {
          w->view()->cleanup(_curvePlacement->columns());
        }
        if (plot) {
          _curvePlacement->update();
          _curvePlacement->setCurrentPlot(plot->tagName());
          plot->addCurve(curve.data());
          plot->GenerateDefaultLabels();
        }
      }
    }
  }

  if (haveEx) {
    (*EX)->readUnlock();
  }
  if (haveEy) {
    (*EY)->readUnlock();
  }
  if (haveExMinus) {
    (*EXMinus)->readUnlock();
  }
  if (haveEyMinus) {
    (*EYMinus)->readUnlock();
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(curve.data());
  KST::dataObjectList.lock().writeUnlock();

  curve = 0L; // drop the reference
  emit modified();
  return true;
}

bool KstCurveDialogI::edit_I() {
  bool rc = false;

  DP->writeLock();

  // verify that the curve name is unique
  if (_tagName->text() != DP->tagName() && KST::dataTagNameNotUnique(_tagName->text())) {
    _tagName->setFocus();
  } else {
    DP->setTagName(_tagName->text());

    { // leave this scope here to destroy the iterator
      KstReadLocker ml(&KST::vectorList.lock());
      KstVectorList::Iterator it = KST::vectorList.findTag(_xVector->selectedVector());
      if (it != KST::vectorList.end()) {
        DP->setXVector(*it);
      }

      it = KST::vectorList.findTag(_yVector->selectedVector());
      if (it != KST::vectorList.end()) {
        DP->setYVector(*it);
      }

      it = KST::vectorList.findTag(_xError->selectedVector());
      DP->setXError(*it);
      if (_checkBoxXMinusSameAsPlus->isChecked()) {
        DP->setXMinusError(*it);
      } else {
        it = KST::vectorList.findTag(_xMinusError->selectedVector());
        DP->setXMinusError(*it);    
      }
      it = KST::vectorList.findTag(_yError->selectedVector());
      DP->setYError(*it);
      if (_checkBoxYMinusSameAsPlus->isChecked()) {
        DP->setYMinusError(*it);
      } else {
        it = KST::vectorList.findTag(_yMinusError->selectedVector());
        DP->setYMinusError(*it);
      }
    }

    DP->setColor(_curveAppearance->color());
    DP->setHasPoints(_curveAppearance->showPoints());
    DP->setHasLines(_curveAppearance->showLines());
    DP->setHasBars(_curveAppearance->showBars());
    DP->setLineWidth(_curveAppearance->lineWidth());
    DP->setLineStyle(_curveAppearance->lineStyle());
    DP->Point.setType(_curveAppearance->pointType());
    DP->setBarStyle(_curveAppearance->barStyle());
    DP->setPointDensity(_curveAppearance->pointDensity());
    DP->setIgnoreAutoScale(_checkBoxIgnoreAutoscale->isChecked());

    rc = true;
  }

  DP->writeUnlock();

  if (rc) {
    emit modified();
  }

  return rc;
}


#include "kstcurvedialog_i.moc"
// vim: ts=2 sw=2 et
