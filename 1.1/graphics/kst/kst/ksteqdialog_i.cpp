/***************************************************************************
                       ksteqdialog_i.cpp  -  Part of KST
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
#include <qregexp.h>
#include <qspinbox.h>

// include files for KDE
#include <kcombobox.h>
#include <kdebug.h>
#include <kmessagebox.h>

// application specific includes
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "eparse-eh.h"
#include "kstdefaultnames.h"
#include "ksteqdialog_i.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "scalarselector.h"
#include "vectorselector.h"

#define DIALOGTYPE KstEqDialogI
#define DTYPE "Equation"
#include "dataobjectdialog.h"

const QString& KstEqDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstEqDialogI> KstEqDialogI::_inst;

KstEqDialogI *KstEqDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstEqDialogI(KstApp::inst());
  }
  return _inst;
}


KstEqDialogI::KstEqDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstEqDialog(parent, name, modal, fl) {
  Init();

  connect(_vectors, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_xVectors, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_scalars, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  populateFunctionList();
}


KstEqDialogI::~KstEqDialogI() {
  DP = 0L;
}


KstEquationPtr KstEqDialogI::_getPtr(const QString& tagin) {
  KstEquationList c = kstObjectSubList<KstDataObject, KstEquation>(KST::dataObjectList);
  return *c.findTag(tagin);
}


void KstEqDialogI::updateWindow() {
  _curvePlacement->update();
}


void KstEqDialogI::_fillFieldsForEdit() {
  if (DP == 0L) {
    return; // shouldn't be needed
  }

  DP->readLock();
  _tagName->setText(DP->tagName());

  Equation->setText(DP->equation());

  DoInterpolation->setChecked(DP->doInterp());
  if (DP->vX()) {
    _xVectors->setSelection(DP->vX()->tagName());
  }

  DP->readUnlock();

  _curveAppearance->hide();
  _curvePlacement->hide();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstEqDialogI::_fillFieldsForNew() {
  KstEquationList eqs = kstObjectSubList<KstDataObject, KstEquation>(KST::dataObjectList);

  /* set tag name */
  _tagName->setText(defaultTag);

  /* set the curve placement window  */
  _curvePlacement->update();

  Equation->clear();

  //for some reason the lower widget needs to be shown first to prevent overlapping?
  _curveAppearance->hide();
  _curvePlacement->show();
  _curveAppearance->show();
  _curveAppearance->reset();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstEqDialogI::update() {
  _curvePlacement->update();
  _vectors->update();
  _xVectors->update();
  _scalars->update();
}


bool KstEqDialogI::new_I() {
  KstEquationPtr eq;

  QString tag_name = _tagName->text();
  QString etext = Equation->text();
  etext.remove(QRegExp("[^a-zA-Z0-9\\(\\)\\+\\-\\*/\\%\\^\\|\\&\\!<>=_]"));
  if (etext.length() > 12) {
    etext.truncate(12);
    etext += "...";
  }

  if (tag_name==defaultTag) {
    tag_name = KST::suggestEQName(etext);
  }

  /* verify that the curve name is unique */
  if (KST::dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (!_checkEntries()) {
    return false;
  }

  KST::vectorList.lock().readLock();
  /* find *V */
  KstVectorList::Iterator i = KST::vectorList.findTag(_xVectors->selectedVector());
  if (i == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the Vector field in plotDialog (Eq) "
              << "refers to a non-existent vector..." << endl;
  }
  KST::vectorList.lock().readUnlock();

  /** Create the equation here */
  eq = new KstEquation(tag_name, Equation->text(), *i, DoInterpolation->isChecked());

  if (!eq->isValid()) {
    eq = 0L;
    QString parseErrors;
    for (QStringList::ConstIterator i = Equation::errorStack.begin(); i != Equation::errorStack.end(); ++i) {
      parseErrors += *i;
      parseErrors += "\n";
    }

    KMessageBox::detailedSorry(this, i18n("There is an error in the equation you entered.  Please fix it."), parseErrors);
    return false;
  }

  KstVCurvePtr vc = new KstVCurve(KST::suggestCurveName(tag_name, true), eq->vX(), eq->vY(), 0L, 0L, 0L, 0L, _curveAppearance->color());
  vc->setHasPoints(_curveAppearance->showPoints());
  vc->setHasLines(_curveAppearance->showLines());
  vc->setHasBars(_curveAppearance->showBars());
  vc->setLineWidth(_curveAppearance->lineWidth());
  vc->setLineStyle(_curveAppearance->lineStyle());
  vc->Point.setType(_curveAppearance->pointType());
  vc->setBarStyle(_curveAppearance->barStyle());

  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_curvePlacement->_plotWindow->currentText()));
  if (!w) {
    QString n = KstApp::inst()->newWindow(KST::suggestWinName());
    w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
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

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(eq.data());
  KST::dataObjectList.append(vc.data());
  KST::dataObjectList.lock().writeUnlock();

  eq = 0L; // drop the reference before we update
  vc = 0L;
  emit modified();
  return true;
}


bool KstEqDialogI::_checkEntries() {
  if (_xVectors->selectedVector().isEmpty()) {
    KMessageBox::sorry(this, i18n("An X vector must be defined first."));
    return false;
  }
  return true;
}


bool KstEqDialogI::edit_I() {
  /* verify that the curve name is unique */
  DP->writeLock();
  if (_tagName->text() != DP->tagName()) {
    if (KST::dataTagNameNotUnique(_tagName->text())) {
      DP->writeUnlock();
      return false;
    }
  }

  DP->setTagName(_tagName->text());
  if (!_checkEntries()) {
    DP->writeUnlock();
    return false;
  }

  KST::vectorList.lock().readLock();
  /* find *V */
  KstVectorList::Iterator i =
    KST::vectorList.findTag(_xVectors->selectedVector());
  if (i == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the Vector field in plotDialog (Eq) "
              << "refers to a non existant vector..." << endl;
  }

  KstVectorPtr vp = *i;
  KST::vectorList.lock().readUnlock();

  DP->setExistingXVector(vp, DoInterpolation->isChecked());

  DP->setEquation(Equation->text());
  if (!DP->isValid()) {
    QString parseErrors;
    for (QStringList::ConstIterator i = Equation::errorStack.begin(); i != Equation::errorStack.end(); ++i) {
      parseErrors += *i;
      parseErrors += "\n";
    }

    KMessageBox::detailedSorry(this, i18n("There is an error in the equation you entered.  Please fix it."), parseErrors);
    DP->writeUnlock();
    return false;
  }
  DP->writeUnlock();

  emit modified();

  return true;
}


void KstEqDialogI::populateFunctionList() {
  Operators->clear();
  Operators->insertItem("+");
  Operators->insertItem("-");
  Operators->insertItem("*");
  Operators->insertItem("/");
  Operators->insertItem("%");
  Operators->insertItem("^");
  Operators->insertItem("&");
  Operators->insertItem("|");
  Operators->insertItem("&&");
  Operators->insertItem("||");
  Operators->insertItem("!");
  Operators->insertItem("<");
  Operators->insertItem("<=");
  Operators->insertItem("==");
  Operators->insertItem(">=");
  Operators->insertItem(">");
  Operators->insertItem("!=");
  Operators->insertItem("PI");
  Operators->insertItem("e");
  Operators->insertItem("STEP()");
  Operators->insertItem("ABS()");
  Operators->insertItem("SQRT()");
  Operators->insertItem("CBRT()");
  Operators->insertItem("SIN()");
  Operators->insertItem("COS()");
  Operators->insertItem("TAN()");
  Operators->insertItem("ASIN()");
  Operators->insertItem("ACOS()");
  Operators->insertItem("ATAN()");
  Operators->insertItem("SEC()");
  Operators->insertItem("CSC()");
  Operators->insertItem("COT()");
  Operators->insertItem("SINH()");
  Operators->insertItem("COSH()");
  Operators->insertItem("TANH()");
  Operators->insertItem("EXP()");
  Operators->insertItem("LN()");
  Operators->insertItem("LOG()");
  Operators->insertItem("PLUGIN()");
}

#include "ksteqdialog_i.moc"
// vim: ts=2 sw=2 et
