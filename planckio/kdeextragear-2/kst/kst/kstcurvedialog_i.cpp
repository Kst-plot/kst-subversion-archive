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

#include "kstcurvedialog_i.h"

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>

#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "vectorselector.h"
#include "kstpoint.h"
#include "kstdatacollection.h"
#include "kstvectordialog_i.h"
#include "kstdoc.h"

KstCurveDialogI *KstCurveDialogI::_inst = 0L;
static KStaticDeleter<KstCurveDialogI> _cuInst;

KstCurveDialogI *KstCurveDialogI::globalInstance() {
  if (!_inst) {
    _inst = _cuInst.setObject(new KstCurveDialogI);
  }
return _inst;
}


KstCurveDialogI::KstCurveDialogI(QWidget* parent,
                                 const char* name, bool modal, WFlags fl)
: KstCurveDialog(parent, name, modal, fl) {
  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(_xVector, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
  connect(_yVector, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
  connect(_xError, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
  connect(_yError, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
  _xError->provideNoneVector(true);
  _yError->provideNoneVector(true);
}

KstCurveDialogI::~KstCurveDialogI() {
}

void KstCurveDialogI::show_I() {
  update();
  show();
  raise();
}

void KstCurveDialogI::show_I(const QString &field) {
  int i = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList).findIndexTag(field);
  update(i);
  show();
  raise();
}

void KstCurveDialogI::show_New() {
  update(-2);
  show();
  raise();
}

void KstCurveDialogI::update(int new_index) {
  int i_curve;
  KstVCurvePtr curve;
  int index;
  bool isNew = false;
  int n_v, n_c;

  KstVCurveList curves = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  if (new_index == -1) {
    if (curves.findTag(Select->currentText()) != curves.end()) {
      QString save = Select->currentText();
      Select->blockSignals(true);
      Select->clear();
      for (KstVCurveList::iterator i = curves.begin(); i != curves.end(); ++i) {
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
  for (KstVCurveList::iterator i = curves.begin(); i != curves.end(); ++i) {
    Select->insertItem((*i)->tagName());
  }

  if (isNew) {
    QString new_label;
    new_label.sprintf("C%d-", curves.count()+1);
    new_label += i18n("<New_Curve>");

    Select->insertItem(new_label);
  }
  if ((index>=0) && (index<Select->count())) {
    Select->setCurrentItem(index);
  }

  /*******************************************/
  /* fill the Vector lists with vector names */
  _xVector->update();
  _yVector->update();
  _xError->update();
  _yError->update();

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

  /****************************************************/
  /* set the vector pull downs to the correct vectors */
  if (n_c > 0 && !isNew) {
    i_curve = Select->currentItem();
    curve = curves[i_curve];

    _xVector->setSelection(curve->getXVTag());
    _yVector->setSelection(curve->getYVTag());
    _xError->setSelection(curve->getXETag());
    _yError->setSelection(curve->getYETag());

    _curveAppearance->setValue(curve->hasLines(), curve->hasPoints(), curve->getColor(), curve->Point.getType());
    Delete->setEnabled(curve->getUsage() == 2);
  } else { /* no curves defined - initialize what we can to vector 0 */
    _curveAppearance->reset();
    Delete->setEnabled(false);
  }
}


void KstCurveDialogI::new_I() {
  KstVectorList::Iterator VX, VY;
  KstVectorList::Iterator EX, EY;
  KstVCurvePtr curve;
  KstPlot *plot;

  if (_xVector->selectedVector().isEmpty()) {
    KMessageBox::sorry(0L, i18n("New curve not made: define vectors first."));
    return;
  }

  /* find VX and VY */
  VX = KST::vectorList.findTag(_xVector->selectedVector());
  VY = KST::vectorList.findTag(_yVector->selectedVector());
  EX = KST::vectorList.findTag(_xError->selectedVector());
  EY = KST::vectorList.findTag(_yError->selectedVector());
  if (VX == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the XVector field in plotDialog refers to "
              << "a non existant vector...." << endl;
  }
  if (VY == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the YVector field in plotDialog refers to "
              << "a non existant vector...." << endl;
  }


  QString tag_name = Select->currentText();
  tag_name.replace(i18n("<New_Curve>"), (*VY)->tagName());

  /* verify that the curve name is unique */
  if (KST::dataTagNameNotUnique(tag_name))
    return;

  /* create the curve */
  curve = new KstVCurve(tag_name, *VX, *VY,
                        EX != KST::vectorList.end() ? *EX : KstVectorPtr(0L),
                        EY != KST::vectorList.end() ? *EY : KstVectorPtr(0L),
                        _curveAppearance->color());
  curve->setHasPoints(_curveAppearance->showPoints());
  curve->setHasLines(_curveAppearance->showLines());
  curve->Point.setType(_curveAppearance->pointType());

  if (_curvePlacement->existingPlot()) {
    /* assign curve to plot */
    plot = KST::plotList.FindKstPlot(_curvePlacement->plotName());
    plot->addCurve(curve);
  }

  if (_curvePlacement->newPlot()) {
    /* assign curve to plot */
    plot = KST::plotList.addPlot(QString::null, _curvePlacement->columns());
    _curvePlacement->appendPlot(plot->tagName(), true);
    plot->addCurve(curve);
    plot->GenerateDefaultLabels();
  }

  KST::dataObjectList.append(curve.data());
  curve = 0L;
  emit modified();
}

void KstCurveDialogI::edit_I() {
  int index;
  KstVCurvePtr curve;

  KstVCurveList curves = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);
  index = Select->currentItem();
  if (index < 0 || unsigned(index) >= curves.count()) {
    new_I();
    return;
  }

  QString tag_name = Select->currentText();

  /* verify that the curve name is unique */
  if (tag_name != curves[index]->tagName()) {
    if (KST::dataTagNameNotUnique(tag_name))
      return;
  }

  curve = curves[index];
  curve->setTagName(tag_name);

  { // leave this scope here to destroy the iterator
    KstVectorList::Iterator i = KST::vectorList.findTag(_xVector->selectedVector());
    if (i != KST::vectorList.end())
      curve->setXVector(*i);

    i = KST::vectorList.findTag(_yVector->selectedVector());
    if (i != KST::vectorList.end())
      curve->setYVector(*i);

    i = KST::vectorList.findTag(_xError->selectedVector());
    curve->setXError(*i);

    i = KST::vectorList.findTag(_yError->selectedVector());
    curve->setYError(*i);
  }

  curve->setColor(_curveAppearance->color());
  curve->setHasPoints(_curveAppearance->showPoints());
  curve->setHasLines(_curveAppearance->showLines());
  curve->Point.setType(_curveAppearance->pointType());

  curve->update(-1);
  curve = 0L;
  curves.clear();
  emit modified();
}


void KstCurveDialogI::delete_I() {
  QString tag;
  int index;

  index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active curve to delete."));
    return;
  }

  tag = Select->currentText();

  if (tag.isEmpty()) {
    return;
  }
  KST::dataObjectList.removeTag(tag);
  emit modified();
}

#include "kstcurvedialog_i.moc"
