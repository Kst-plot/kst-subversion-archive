/***************************************************************************
                       ksteqdialog.cpp  -  Part of KST
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

#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QRegExp>
#include <QSpinBox>

#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "eparse-eh.h"
#include "kst2dplot.h"
// xxx #include "kstchoosecolordialog.h"
#include "kstdataobjectcollection.h"
#include "kstdefaultnames.h"
#include "ksteqdialog.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "scalarselector.h"
#include "vectorselector.h"

const QString& KstEqDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

QPointer<KstEqDialog> KstEqDialog::_inst;

KstEqDialog *KstEqDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstEqDialog(KstApp::inst());
  }

  return _inst;
}


KstEqDialog::KstEqDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstDataDialog(parent) {
  _w = new Ui::EqDialogWidget();
  _w->setupUi(_contents);

  setMultiple(true);

  connect(_w->_vectors, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_xVectors, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_scalars, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));

  //
  // connections for edit multiple mode...
  //

  connect(_w->_doInterpolation, SIGNAL(clicked()), this, SLOT(setDoInterpolationDirty()));
  populateFunctionList();

  // 
  // connections for apply button...
  //

  connect(_w->_equation, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVectors, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_xVectors, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_doInterpolation, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_curveAppearance, SIGNAL(changed()), this, SLOT(wasModifiedApply()));
}


KstEqDialog::~KstEqDialog() {
}


void KstEqDialog::updateWindow() {
  _w->_curvePlacement->update();
}


void KstEqDialog::fillFieldsForEdit() {
  KstEquationPtr ep;

  ep = kst_cast<KstEquation>(_dp);
  if (!ep) {
    return; // shouldn't be needed
  }

  // re-parse to get the latest equation
  ep->writeLock();
  ep->reparse();
  ep->unlock();

  ep->readLock();
  _tagName->setText(ep->tagName());
  _w->_equation->setText(ep->equation());

  _w->_doInterpolation->setChecked(ep->doInterp());
  if (ep->vXIn()) {
    _w->_xVectors->setSelection(ep->vXIn()->tag().displayString());
  }

  ep->unlock();

  _w->_curveAppearance->hide();
  _w->_curvePlacement->hide();
  _legendText->hide();
  _legendLabel->hide();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstEqDialog::fillFieldsForNew() {
  KstEquationList eqs;
  KstVCurveList curves;

  eqs = kstObjectSubList<KstDataObject, KstEquation>(KST::dataObjectList);
  curves = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);
  _legendText->show();
  _legendLabel->show();

  _w->_curvePlacement->update();

  //
  // set the X Axis Vector to the X axis vector of 
  // the last curve on the global curve list...
  //

  if (curves.count() > 0) {
    _w->_xVectors->setSelection(curves.last()->xVTag().displayString());
  }

  _w->_equation->clear();

  //
  // for some reason the lower widget needs to be shown first to prevent overlapping?
  //

  _w->_curveAppearance->hide();
  _w->_curvePlacement->show();
  _w->_curveAppearance->show();
  _w->_curveAppearance->reset();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstEqDialog::update() {
  _w->_curvePlacement->update();
  _w->_vectors->update();
  _w->_xVectors->update();
  _w->_scalars->update();
}


bool KstEqDialog::newObject() {
  QString tag_name = _tagName->text();
  QString etext = _w->_equation->text();

  etext.remove(QRegExp("[^a-zA-Z0-9\\(\\)\\+\\-\\*/\\%\\^\\|\\&\\!<>=_.]"));
  etext.replace(KstObjectTag::tagSeparator, KstObjectTag::tagSeparatorReplacement);
  if (etext.length() > 12) {
    etext.truncate(12);
    etext += "...";
  }

  if (tag_name == defaultTag) {
    tag_name = KST::suggestEQName(etext);
  }

  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (!checkEntries()) {
    return false;
  }

  KST::vectorList.lock().readLock();
  KstVectorPtr vp = *KST::vectorList.findTag(_w->_xVectors->selectedVector());
  if (vp) {
    KST::vectorList.lock().unlock();
  
    KstEquationPtr eq;

    eq = new KstEquation(tag_name, _w->_equation->text(), vp, _w->_doInterpolation->isChecked());
  
    if (!eq->isValid()) {
      QStringList::const_iterator i;
      QString strWarning = i18n("There is an error in the equation you entered.\n");
  
      eq = 0L;
  
      for (i = Equation::errorStack.begin(); i != Equation::errorStack.end(); ++i) {
        strWarning += *i;
        strWarning += "\n";
      }
  
      QMessageBox::warning(this, i18n("Kst"), strWarning);

      return false;
    }
  
    KstVCurvePtr vc;
    QColor color;

// xxx    color = KstApp::inst()->chooseColorDlg()->getColorForCurve(eq->vX(), eq->vY());
    if (!color.isValid()) {
      color = _w->_curveAppearance->color();
    }

    vc = new KstVCurve(KST::suggestCurveName(eq->tag(), true), eq->vX(), eq->vY(), KstVectorPtr(), KstVectorPtr(), KstVectorPtr(), KstVectorPtr(), color);
    vc->setHasPoints(_w->_curveAppearance->showPoints());
    vc->setHasLines(_w->_curveAppearance->showLines());
    vc->setHasBars(_w->_curveAppearance->showBars());
    vc->setLineWidth(_w->_curveAppearance->lineWidth());
    vc->setLineStyle(_w->_curveAppearance->lineStyle());
    vc->setPointStyle(_w->_curveAppearance->pointType());
    vc->setPointDensity(_w->_curveAppearance->pointDensity());
    vc->setBarStyle(_w->_curveAppearance->barStyle());
  
    QString legend_text = _legendText->text();
    if (legend_text == defaultTag) {
      vc->setLegendText(QString(""));
    } else {
      vc->setLegendText(legend_text);
    }
  
    KstViewWindow *w;
/* xxx
    w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_w->_curvePlacement->_plotWindow->currentText()));
    if (!w) {
      QString n = KstApp::inst()->newWindow(KST::suggestWinName());

      w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
    }
*/  
    if (w) {
      Kst2DPlotPtr plot;

      if (_w->_curvePlacement->existingPlot()) {
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(_w->_curvePlacement->plotName()));
        if (plot) {
          plot->addCurve(vc);
        }
      }
  
      if (_w->_curvePlacement->newPlot()) {
        QString name = w->createPlot(KST::suggestPlotName());

        if (_w->_curvePlacement->reGrid()) {
          w->view()->cleanup(_w->_curvePlacement->columns());
        }
        plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
        if (plot) {
          _w->_curvePlacement->update();
          _w->_curvePlacement->setCurrentPlot(plot->tagName());
          plot->addCurve(vc);
          plot->generateDefaultLabels();
        }
      }
    }
  
    KST::dataObjectList.lock().writeLock();
    KST::dataObjectList.append(eq);
    KST::dataObjectList.append(vc);
    KST::dataObjectList.lock().unlock();
  
    //
    // drop the reference before we update...
    //

    eq = 0L; 
    vc = 0L;

    emit modified();
  }

  return true;
}


bool KstEqDialog::checkEntries() {
  if (_w->_xVectors->selectedVector().isEmpty() && !_editMultipleMode) {
    QMessageBox::warning(this, i18n("Kst"), i18n("An X vector must be defined first."));

    return false;
  }

  return true;
}


bool KstEqDialog::editSingleObject(KstEquationPtr eqPtr) {
  KstVectorPtr vp;

  eqPtr->writeLock();
  if (!checkEntries()) {
    eqPtr->unlock();

    return false;
  }

  if (_xVectorsDirty) {
    KstVectorList::iterator i;
    
    KST::vectorList.lock().readLock();

    i = KST::vectorList.findTag(_w->_xVectors->selectedVector());
    if (i != KST::vectorList.end()) {
      vp = *i;
    }

    KST::vectorList.lock().unlock();
  } else {
    vp = eqPtr->vX();
  }

  //
  // update the _doInterpolation only if it is dirty...
  //

  if (_doInterpolationDirty) {
    eqPtr->setExistingXVector(vp, _w->_doInterpolation->isChecked());
  } else {
    eqPtr->setExistingXVector(vp, eqPtr->doInterp());
  }

  if (_equationDirty) {
    eqPtr->setEquation(_w->_equation->text());
    if (!eqPtr->isValid()) {
      QStringList::const_iterator i;
      QString strWarning = i18n("There is an error in the equation you entered.\n");

      for (i = Equation::errorStack.begin(); i != Equation::errorStack.end(); ++i) {
        strWarning += *i;
        strWarning += "\n";
      }

      QMessageBox::warning(this, i18n("Kst"), strWarning);
      eqPtr->unlock();

      return true;
    }

    eqPtr->setRecursed(false);
    if (eqPtr->recursion()) {
      eqPtr->setRecursed(true);
      eqPtr->unlock();
      QMessageBox::critical(this, i18n("Kst"), i18n("There is a recursion resulting from the equation you entered."));

      return false;
    }
  }

  eqPtr->unlock();

  return true;
}


bool KstEqDialog::editObject() {
  KstEquationList eqList;

  eqList = kstObjectSubList<KstDataObject,KstEquation>(KST::dataObjectList);

  if (_editMultipleMode) { 
    //
    // if the user selected no vector, treat it as non-dirty...
    //

    _xVectorsDirty = _w->_xVectors->_vector->currentIndex() != 0;
    _equationDirty = !_w->_equation->text().isEmpty();

    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstEquationList::iterator eqIter;
        KstEquationPtr eqPtr;

        eqIter = eqList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        if (eqIter == eqList.end()) {
          return false;
        }

        eqPtr = *eqIter;
        if (!editSingleObject(eqPtr)) {
          return false;
        }
        didEdit = true;
      }
    } 
    if (!didEdit) {
      QMessageBox::warning(this, i18n("Kst"), i18n("Select one or more objects to edit."));
      return false;  
    }
  } else {
    QString tagName = _tagName->text();
    KstEquationPtr ep;

    //
    // verify that the curve name is unique...
    //

    ep = kst_cast<KstEquation>(_dp);
    if (!ep || (tagName != ep->tagName() && KstData::self()->dataTagNameNotUnique(tagName))) {
      _tagName->setFocus();
      return false;
    }

    ep->writeLock();
    ep->setTagName(tagName);
    ep->unlock();

    //
    // then edit the object...
    //

    _equationDirty = true;
    _xVectorsDirty = true;
    _doInterpolationDirty = true;
    if (!editSingleObject(ep)) {
      return false;
    }
  }

  emit modified();

  return true;
}


void KstEqDialog::populateFunctionList() {
  _w->Operators->clear();
  _w->Operators->addItem("+");
  _w->Operators->addItem("-");
  _w->Operators->addItem("*");
  _w->Operators->addItem("/");
  _w->Operators->addItem("%");
  _w->Operators->addItem("^");
  _w->Operators->addItem("&");
  _w->Operators->addItem("|");
  _w->Operators->addItem("&&");
  _w->Operators->addItem("||");
  _w->Operators->addItem("!");
  _w->Operators->addItem("<");
  _w->Operators->addItem("<=");
  _w->Operators->addItem("==");
  _w->Operators->addItem(">=");
  _w->Operators->addItem(">");
  _w->Operators->addItem("!=");
  _w->Operators->addItem("PI");
  _w->Operators->addItem("e");
  _w->Operators->addItem("STEP()");
  _w->Operators->addItem("ABS()");
  _w->Operators->addItem("SQRT()");
  _w->Operators->addItem("CBRT()");
  _w->Operators->addItem("SIN()");
  _w->Operators->addItem("COS()");
  _w->Operators->addItem("TAN()");
  _w->Operators->addItem("ASIN()");
  _w->Operators->addItem("ACOS()");
  _w->Operators->addItem("ATAN()");
  _w->Operators->addItem("SEC()");
  _w->Operators->addItem("CSC()");
  _w->Operators->addItem("COT()");
  _w->Operators->addItem("SINH()");
  _w->Operators->addItem("COSH()");
  _w->Operators->addItem("TANH()");
  _w->Operators->addItem("EXP()");
  _w->Operators->addItem("LN()");
  _w->Operators->addItem("LOG()");
  _w->Operators->addItem("PLUGIN()");
}


void KstEqDialog::populateEditMultiple() {
  KstEquationList eqList;

  eqList = kstObjectSubList<KstDataObject,KstEquation>(KST::dataObjectList);
  _editMultipleWidget->_objectList->insertItems(0, eqList.tagNames());

  //
  // also intermediate state for multiple edit...
  //

  _w->_xVectors->_vector->insertItem(0, "");
  _w->_xVectors->_vector->setCurrentIndex(0);
  _w->_doInterpolation->setTristate(true);
  _w->_doInterpolation->setChecked(Qt::PartiallyChecked);
  _w->_equation->setText("");

  //
  // and clean all the fields...
  //

  _equationDirty = false;
  _xVectorsDirty = false;
  _doInterpolationDirty = false;
}


void KstEqDialog::setDoInterpolationDirty() {
  _w->_doInterpolation->setTristate(false); 
  _doInterpolationDirty = true;
}

#include "ksteqdialog.moc"

