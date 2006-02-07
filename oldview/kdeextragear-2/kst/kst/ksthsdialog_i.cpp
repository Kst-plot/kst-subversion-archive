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
#include "ksthsdialog_i.h"

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>
#include <kdebug.h>

#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "vectorselector.h"
#include "kstvectordialog_i.h"
#include "kstdoc.h"
#include "kstdatacollection.h"

KstHsDialogI *KstHsDialogI::_inst = 0L;
static KStaticDeleter<KstHsDialogI> _hsInst;

KstHsDialogI *KstHsDialogI::globalInstance() {
  if (!_inst) {
    _inst = _hsInst.setObject(new KstHsDialogI);
  }
return _inst;
}


KstHsDialogI::KstHsDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstHsDialog(parent, name, modal, fl) {
  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(AutoBin, SIGNAL(clicked()), this, SLOT(autoBin()));
  connect(_vector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  _curveAppearance->setUsePoints(false);
  _curveAppearance->setMustUseLines(true);
}

KstHsDialogI::~KstHsDialogI() {
}

void KstHsDialogI::show_I() {
  update();
  show();
  raise();
}

void KstHsDialogI::show_New() {
  update(-2);
  show();
  raise();
}

void KstHsDialogI::show_I(const QString &field) {
  int i = kstObjectSubList<KstDataObject, KstHistogram>(KST::dataObjectList).findIndexTag(field);
  update(i);
  show();
  raise();
}

void KstHsDialogI::update(int new_index) {
  int index;
  unsigned int i_hs;
  KstHistogramPtr hs;
  bool isNew = false;
  int n_v, n_c;

  KstHistogramList curves = kstObjectSubList<KstDataObject, KstHistogram>(KST::dataObjectList);

  if (new_index == -1) {
    if (curves.findTag(Select->currentText()) != curves.end()) {
      QString save = Select->currentText();
      Select->blockSignals(true);
      Select->clear();
      for (KstHistogramList::iterator i = curves.begin(); i != curves.end(); ++i) {
        Select->insertItem((*i)->tagName());
      }
      Select->setCurrentText(save);
      Select->blockSignals(false);
      return;
    }
  }

  /**********************/
  /* initialize indexes */
  KST::vectorList.lock().readLock();
  n_v = KST::vectorList.count();
  KST::vectorList.lock().readUnlock();
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
    index = n_c-1;
  }

  /*****************************************/
  /* fill the Select combo with curve tags */
  Select->clear();
  for (KstHistogramList::iterator i = curves.begin(); i != curves.end(); ++i) {
    Select->insertItem((*i)->tagName());
  }

  if (isNew) {
    QString new_label = QString("C%1-").arg(curves.count()+1) + i18n("<New_Histogram>");
    Select->insertItem(new_label);
  }
  if (index >= 0 && index < Select->count()) {
    Select->setCurrentItem(index);
  }

  /******************************************/
  /* fill the Vector list with vector names */
  _vector->update();

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
  i_hs = Select->currentItem();
  if (n_c > 0 && int(i_hs) < n_c) {
    hs = curves[i_hs];

    /* select the proper Vector */
    _vector->setSelection(hs->getVTag());

    if (!isNew) {
      _curveAppearance->setColor(hs->getColor());
      Delete->setEnabled(hs->getUsage() == 2);
    } else {
      Delete->setEnabled(false);
    }

    N->setValue(hs->getBins());
    Min->setText(QString::number(hs->minX()));
    Max->setText(QString::number(hs->maxX()));

    if (hs->isNormPercent()) NormIsPercent->setChecked(true);
    else if (hs->isNormFraction()) NormIsFraction->setChecked(true);
    else if (hs->isNormOne()) PeakIs1->setChecked(true);
    else NormIsNumber->setChecked(true);
  } else {
    Delete->setEnabled(false);
  }
  
    _curveAppearance->redrawCombo();
}

void KstHsDialogI::new_I() {
  KstHistogramPtr hs;
  double new_max, new_min, m;
  int new_n_bins;
  KstHsNormType new_norm_mode;

  QString tag_name = Select->currentText();
  tag_name.replace(i18n("<New_Histogram>"), "HS_" + _vector->selectedVector());

  /* verify that the curve name is unique */
  if (KST::dataTagNameNotUnique(tag_name)) {
    Select->setFocus();
    return;
  }

  if (_vector->selectedVector().isEmpty()) {
    KMessageBox::sorry(0L, i18n("New Histogram not made: define vectors first."));
    return;
  }

  /* find max and min */
  new_min = Min->text().toDouble();
  new_max = Max->text().toDouble();
  if (new_max < new_min) {
    m = new_max;
    new_max = new_min;
    new_min = m;
  }

  if (new_max == new_min) {
    KMessageBox::sorry(0L, i18n("Max and Min can not be equal."));
    return;
  }

  new_n_bins = N->text().toInt();
  if (new_n_bins < 1) {
    KMessageBox::sorry(0L, i18n("You must have one or more bins in a histogram."));
    return;
  }

  if (NormIsPercent->isChecked()) {
    new_norm_mode = KST_HS_PERCENT;
  } else if (NormIsFraction->isChecked()) {
    new_norm_mode = KST_HS_FRACTION;
  } else if (PeakIs1->isChecked()) {
    new_norm_mode = KST_HS_MAX_ONE;
  } else {
    new_norm_mode = KST_HS_NUMBER;
  }

  { // make sure this iterator falls out of scope
    KstReadLocker ml(&KST::vectorList.lock());
    KstVectorList::Iterator i = KST::vectorList.findTag(_vector->selectedVector());
    if (i == KST::vectorList.end()) {
      kdFatal() << "Bug in kst: the Vector field in plotDialog (Hs) refers to "
                << " a non existant vector..." << endl;
    }

    hs = new KstHistogram(tag_name, *i, new_min, new_max,
                          new_n_bins, new_norm_mode, _curveAppearance->color());

  }

  KstPlot *plot = 0L;
  if (_curvePlacement->existingPlot()) {
    /* assign curve to plot */
    plot = KST::plotList.FindKstPlot(_curvePlacement->plotName());
    plot->addCurve(hs);
  }
  if (_curvePlacement->newPlot()) {
    /* assign curve to plot */
    plot = KST::plotList.addPlot(QString::null, _curvePlacement->columns());
    _curvePlacement->appendPlot(plot->tagName(), true);
    plot->addCurve(hs);
    plot->GenerateDefaultLabels();
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(hs.data());
  KST::dataObjectList.lock().writeUnlock();
  hs = 0L;
  emit modified();
}


void KstHsDialogI::edit_I() {
  int index;
  KstHistogramPtr hs;
  double new_max, new_min, m;
  int new_n_bins;

  index = Select->currentItem();
  KstHistogramList curves = kstObjectSubList<KstDataObject, KstHistogram>(KST::dataObjectList);
  if (index < 0 || unsigned(index) >= curves.count()) {
    new_I();
    return;
  }

  /* verify that the curve name is unique */
  QString tag_name = Select->currentText();
  hs = curves[index];
  if (tag_name != hs->tagName()) {
    if (KST::dataTagNameNotUnique(tag_name)) {
      Select->setFocus();
      return;
    }
  }

  /* find max and min */
  new_min = Min->text().toDouble();
  new_max = Max->text().toDouble();
  if (new_max<new_min) {
    m = new_max;
    new_max = new_min;
    new_min = m;
  }

  if (new_max == new_min) {
    KMessageBox::sorry(0L, i18n("Max and Min can not be equal."));
    return;
  }

  new_n_bins = N->text().toInt();
  if (new_n_bins < 1) {
    KMessageBox::sorry(0L, i18n("You must have one or more bins in a histogram."));
    return;
  }

  hs->setTagName(tag_name);
  KST::vectorList.lock().readLock();
  hs->setVector(*KST::vectorList.findTag(_vector->selectedVector()));
  KST::vectorList.lock().readUnlock();
  hs->setColor(_curveAppearance->color());

  hs->setNBins(new_n_bins);
  hs->setXRange(new_min, new_max);

  if (NormIsPercent->isChecked()) hs->setIsNormPercent();
  else if (NormIsFraction->isChecked()) hs->setIsNormFraction();
  else if (PeakIs1->isChecked()) hs->setIsNormOne();
  else hs->setIsNormNum();

  hs->update(-1);
  hs = 0L;
  curves.clear();
  emit modified();
}

void KstHsDialogI::delete_I() {
  int index, i_plot, i_hs;
  KstHistogramPtr hs;
  KstPlot *plot;
  bool need_update = false;

  index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active histogram to delete."));
    return;
  }

  KstHistogramList curves = kstObjectSubList<KstDataObject, KstHistogram>(KST::dataObjectList);
  hs = curves[index];

  /* delete the hs from all plots */
  for (i_plot = 0; i_plot<(int)KST::plotList.count(); i_plot++) {
    plot = KST::plotList.at(i_plot);
    for (i_hs = 0; i_hs < (int)plot->Curves.count(); i_hs++) {
      if (plot->Curves[i_hs]->tagName() == hs->tagName()) {
        plot->Curves.remove(plot->Curves[i_hs]);
        i_hs--;
        need_update = true;
      }
    }
  }

  /* delete the curve */
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.remove(hs.data());
  KST::dataObjectList.lock().writeUnlock();
  hs = 0L;
  curves.clear();
  emit modified();
}

void KstHsDialogI::autoBin() {
  KstReadLocker ml(&KST::vectorList.lock());
  double max, min;
  int n;

  if (KST::vectorList.count() < 1)
    return;

  /* find *V */
  KstVectorList::Iterator i = KST::vectorList.findTag(_vector->selectedVector());
  if (i == KST::vectorList.end()) {
    kdFatal() << "Bug in kst: the Vector field in hsdialog refers to "
                 "a non existant vector....(Boh\?\?)" << endl;
  }

  KstHistogram::AutoBin(KstVectorPtr(*i), &n, &max, &min);

  N->setValue(n);
  Min->setText(QString::number(min));
  Max->setText(QString::number(max));
}

#include "ksthsdialog_i.moc"
// vim: ts=2 sw=2 et
