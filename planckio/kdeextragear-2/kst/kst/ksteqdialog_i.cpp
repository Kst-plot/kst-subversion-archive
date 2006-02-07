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
#include "ksteqdialog_i.h"

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kmessagebox.h>

#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "vectorselector.h"
#include "scalarselector.h"
#include "kstdoc.h"
#include "kstdatacollection.h"


KstEqDialogI *KstEqDialogI::_inst = 0L;
static KStaticDeleter<KstEqDialogI> _eqInst;

KstEqDialogI *KstEqDialogI::globalInstance() {
  if (!_inst) {
    _inst = _eqInst.setObject(new KstEqDialogI);
  }
return _inst;
}


KstEqDialogI::KstEqDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstEqDialog(parent, name, modal, fl) {
  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(_vectors, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
  connect(_xVectors, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
  connect(_scalars, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
}

KstEqDialogI::~KstEqDialogI() {
}

void KstEqDialogI::show_I() {
  update();
  show();
  raise();
}

void KstEqDialogI::show_New() {
  update(-2);
  show();
  raise();
}

void KstEqDialogI::show_I(const QString &field) {
  int i = kstObjectSubList<KstDataObject, KstEquationCurve>(KST::dataObjectList).findIndexTag(field);
  update(i);
  show();
  raise();
}

void KstEqDialogI::update(int new_index) {
  int index;
  int i_eq;
  KstEquationCurvePtr eq;
  bool isNew = false;
  int n_v, n_c;

  KstEquationCurveList curves = kstObjectSubList<KstDataObject, KstEquationCurve>(KST::dataObjectList);

  if (new_index == -1) {
    if (curves.findTag(Select->currentText()) != curves.end()) {
      QString save = Select->currentText();
      Select->blockSignals(true);
      Select->clear();
      for (KstEquationCurveList::iterator i = curves.begin(); i != curves.end(); ++i) {
        Select->insertItem((*i)->tagName());
      }
      Select->setCurrentText(save);
      Select->blockSignals(false);
      return;
    }
  }

  /**********************/
  /* initialize indexes */
  n_v = KST::vectorList.count();
  n_c = curves.count();
  if (new_index == -2) { // initialize for new curve
    isNew = true;
    index = n_c;
  } else if (n_c < 1) {
    isNew = true;
    index = 0;
  } else if (new_index >= 0 && new_index < n_c) { // initialize specific curve
    index = new_index;
  } else if (Select->count() > 0) { // initialize for old default
    index = Select->currentItem();
  } else { // initialize for last in list
    index = n_c - 1;
  }

  /*****************************************/
  /* fill the Select combo with curve tags */
  Select->clear();
  for (KstEquationCurveList::iterator i = curves.begin(); i != curves.end(); ++i) {
    Select->insertItem((*i)->tagName());
  }

  if (isNew) {
    QString new_label;
    new_label = QString("C%1-").arg(curves.count()+1) + i18n("<New_Equation>");
    Select->insertItem(new_label);
  }
  if (index >= 0 && index < Select->count()) {
    Select->setCurrentItem(index);
  }

  /*******************************************/
  /* fill the Vector lists with vector names */
  _vectors->update();
  _xVectors->update();
  _scalars->update();

  /***********************************/
  /* set the curve placement window  */
  _curvePlacement->setPlotList(KST::plotList.tagNames(), true);
  _curvePlacement->setColumns(KST::plotList.getPlotCols());
  if (isNew) {
    // guess what placement option is wanted
    if (!KST::plotList.isEmpty() && curves.count() > KST::plotList.count()) {
      _curvePlacement->setNewPlot(false);
      _curvePlacement->setExistingPlot(true);
    } else {
      _curvePlacement->setNewPlot(true);
      _curvePlacement->setExistingPlot(false);
    }
  }

  /***************************************/
  /* Fill fields with the correct values */
  if (n_c > 0 && !isNew) {
    i_eq = Select->currentItem();
    eq = curves[i_eq];

    Equation->setText(eq->equation());

    if (eq->isXStatic()) {
      GenerateX->setChecked(true);
      UseExistingVector->setChecked(false);
    } else {
      GenerateX->setChecked(false);
      UseExistingVector->setChecked(true);
      DoInterpolation->setChecked(eq->doInterp());
      _xVectors->setSelection(eq->vX()->tagName());
    }
    N->setValue(eq->vX()->sampleCount());
    XMin->setText(QString::number(eq->vX()->min()));
    XMax->setText(QString::number(eq->vX()->max()));

    _curveAppearance->setValue(eq->hasLines(), eq->hasPoints(), eq->getColor(), eq->Point.getType());
    Delete->setEnabled(eq->getUsage() == 2);
  } else {
    Equation->clear();
    _curveAppearance->reset();
    Delete->setEnabled(false);
  }
}

void KstEqDialogI::new_I() {
  KstEquationCurvePtr eq;
  int n;
  double x0, x1;
  bool ok;

  QString tag_name = Select->currentText();
  tag_name.replace(i18n("<New_Equation>"), Equation->text());

  /* verify that the curve name is unique */
  if (KST::dataTagNameNotUnique(tag_name)) return;

  if (UseExistingVector->isChecked()) {
    /* find *V */
    KstVectorList::Iterator i = KST::vectorList.findTag(_xVectors->selectedVector());
    if (i == KST::vectorList.end()) {
      kdFatal() << "Bug in kst: the Vector field in plotDialog (Eq) refers to "
                << "a non existant vector..." << endl;
    }
    /** Create the equation here */
    eq = new KstEquationCurve(tag_name, Equation->text(),
                              *i, DoInterpolation->isChecked(),
                              _curveAppearance->color());
  } else {
    n = N->value();
    if (n < 2) {
      KMessageBox::sorry(0L, i18n("The equation must be evaluated at more than 1 point.  Increase the number of samples."));
      return;
    }
    x0 = XMin->text().toDouble(&ok);
    if (!ok) {
      KMessageBox::sorry(0L, i18n("The text you entered for the minimum X value is not a number."));
      return;
    }
    x1 = XMax->text().toDouble(&ok);
    if (!ok) {
      KMessageBox::sorry(0L, i18n("The text you entered for the maximum X value is not a number."));
      return;
    }

    if (x0 == x1) {
      KMessageBox::sorry(0L, i18n("The minimum and maximum X range cannot be equal."));
      return;
    }
    eq = new KstEquationCurve(tag_name, Equation->text(),
                              x0, x1, n, _curveAppearance->color());
  }

  if (!eq->isValid()) {
    KMessageBox::sorry(0L, i18n("There is a syntax error in the equation you entered.  Please fix it."));
    return;
  }

  eq->setHasPoints(_curveAppearance->showPoints());
  eq->setHasLines(_curveAppearance->showLines());
  eq->Point.setType(_curveAppearance->pointType());

  KstPlot *plot = 0L;
  if (_curvePlacement->existingPlot()) {
    /* assign curve to plot */
    plot = KST::plotList.FindKstPlot(_curvePlacement->plotName());
    plot->addCurve(eq);
  }

  if (_curvePlacement->newPlot()) {
    /* assign curve to plot */
    plot = KST::plotList.addPlot(QString::null, _curvePlacement->columns());
    _curvePlacement->appendPlot(plot->tagName(), true);
    plot->addCurve(eq);
    plot->GenerateDefaultLabels();
  }

  KST::dataObjectList.append(eq.data());
  eq = 0L; // drop the reference before we update
  emit modified();
}


void KstEqDialogI::edit_I() {
  KstEquationCurvePtr eq;
  int index;
  int n;
  double x0, x1;
  bool ok;

  if (Select->count() < 1) {
    new_I();
    return;
  }

  KstEquationCurveList curves = kstObjectSubList<KstDataObject, KstEquationCurve>(KST::dataObjectList);
  index = Select->currentItem();
  if (index < 0 || unsigned(index) >= curves.count()) {
    new_I();
    return;
  }

  eq = curves[index];

  /* verify that the curve name is unique */
  QString tag_name = Select->currentText();

  if (tag_name != eq->tagName()) {
    if (KST::dataTagNameNotUnique(tag_name))
      return;
  }
  eq->setTagName(tag_name);

  eq->setEquation(Equation->text());

  eq->setColor(_curveAppearance->color());
  eq->setHasPoints(_curveAppearance->showPoints());
  eq->setHasLines(_curveAppearance->showLines());
  eq->Point.setType(_curveAppearance->pointType());

  if (UseExistingVector->isChecked()) {
    /* find *V */
    KstVectorList::Iterator i = KST::vectorList.findTag(_xVectors->selectedVector());
    if (i == KST::vectorList.end()) {
      kdFatal() << "Bug in kst: the Vector field in plotDialog (Eq) refers to "
                << "a non existant vector..." << endl;
    }

    eq->setExistingXVector(*i, DoInterpolation->isChecked());
  } else {
    n = N->value();
    if (n < 2) {
      KMessageBox::sorry(0L, i18n("The equation must be evaluated at more than one point.  Increase Num Samples."));
      return;
    }
    x0 = XMin->text().toDouble(&ok);
    if (!ok) {
      KMessageBox::sorry(0L, i18n("The text you entered for the minimum X value is not a number."));
      return;
    }
    x1 = XMax->text().toDouble(&ok);
    if (!ok) {
      KMessageBox::sorry(0L, i18n("The text you entered for the maximum X value is not a number."));
      return;
    }

    if (x0 == x1) {
      KMessageBox::sorry(0L, i18n("The minimum and maximum X range cannot be equal."));
      return;
    }
    eq->setStaticXVector(x0, x1, n);
  }

  eq->update();
  eq = 0L; // drop the reference before we update
  curves.clear();
  emit modified();
}

void KstEqDialogI::delete_I() {
  QString tag;
  int index;

  index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active equation to delete."));
    return;
  }

  tag = Select->currentText();

  if (tag.isEmpty())
    return;

  KST::dataObjectList.removeTag(tag);
  emit modified();
}

#include "ksteqdialog_i.moc"
