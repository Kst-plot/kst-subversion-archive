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
#include "kstpsddialog_i.h"

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>
#include <kdebug.h>

#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "vectorselector.h"
#include "kstdatacollection.h"
#include "kstvectordialog_i.h"
#include "kstdoc.h"

KstPsdDialogI *KstPsdDialogI::_inst = 0L;
static KStaticDeleter<KstPsdDialogI> _psInst;

KstPsdDialogI *KstPsdDialogI::globalInstance() {
  if (!_inst) {
    _inst = _psInst.setObject(new KstPsdDialogI);
  }
return _inst;
}


KstPsdDialogI::KstPsdDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstPsdDialog(parent, name, modal, fl) {
  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(New,    SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit,   SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));
  connect(_vector, SIGNAL(newVectorCreated()), this, SIGNAL(modified()));
}

KstPsdDialogI::~KstPsdDialogI() {
}

void KstPsdDialogI::show_I() {
  update();
  show();
  raise();
}

void KstPsdDialogI::show_I(const QString &field) {
  int i = kstObjectSubList<KstDataObject, KstPSDCurve>(KST::dataObjectList).findIndexTag(field);
  update(i);
  show();
  raise();
}

void KstPsdDialogI::show_New() {
  update(-2);
  show();
  raise();
}

void KstPsdDialogI::update(int new_index) {
  int i_curve;
  KstPSDCurvePtr curve;
  int index;
  bool isNew=false;
  int n_v, n_c;

  KstPSDCurveList curves = kstObjectSubList<KstDataObject, KstPSDCurve>(KST::dataObjectList);
  if (new_index == -1) {
    if (curves.findTag(Select->currentText()) != curves.end()) {
      QString save = Select->currentText();
      Select->blockSignals(true);
      Select->clear();
      for (KstPSDCurveList::iterator i = curves.begin(); i != curves.end(); ++i) {
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
  for (KstPSDCurveList::iterator i = curves.begin(); i != curves.end(); ++i) {
    Select->insertItem((*i)->tagName());
  }

  if (isNew) {
    QString new_label = QString("C%1-").arg(curves.count()+1) + i18n("<New_PSD>");
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
  if (n_c > 0) {
    if (isNew) {
      i_curve = 0;
    } else {
      i_curve = Select->currentItem();
    }
    curve = curves[i_curve];

    _vector->setSelection(curve->getVTag());

    if (!isNew) {
      _curveAppearance->setValue(curve->hasLines(), curve->hasPoints(), curve->getColor(), curve->Point.getType());
      Delete->setEnabled(curve->getUsage() == 2);
    } else {
      Delete->setEnabled(false);
    }

    /* set sample rate, Units, FFT len, and vector units */
    FFTLen->setValue(curve->getLen());
    SampRate->setText(QString::number(curve->getFreq()));
    VectorUnits->setText(curve->VUnits);
    RateUnits->setText(curve->RUnits);

    Appodize->setChecked(curve->appodize());
    RemoveMean->setChecked(curve->removeMean());
  } else {
    Delete->setEnabled(false);
  }
   _curveAppearance->redrawCombo();
}

void KstPsdDialogI::new_I() {
  KstPSDCurvePtr curve;
  double new_freq;
  int new_len;

  QString tag_name = Select->currentText();
  tag_name.replace(i18n("<New_PSD>"), "PSD_" + _vector->selectedVector());

  /* verify that the curve name is unique */
  if (KST::dataTagNameNotUnique(tag_name)) {
    Select->setFocus();
    return;
  }

  if (_vector->selectedVector().isEmpty()) {
    KMessageBox::sorry(0L, i18n("New PSD not made: define vectors first."));
    return;
  }

  /* find new_freq */
  new_freq = SampRate->text().toDouble();
  if (new_freq <= 0) {
      KMessageBox::sorry(0L, i18n("The sample rate must be greater than 0."));
      return;
  }

  /* find new_len */
  new_len = FFTLen->text().toInt();
  if (new_len < 2) {
      KMessageBox::sorry(0L, i18n("The FFT length must be greater than 2^2."));
      return;
  }

  {
    KST::vectorList.lock().readLock();
    KstVectorList::Iterator i = KST::vectorList.findTag(_vector->selectedVector());
    if (i == KST::vectorList.end()) {
      kdFatal() << "Bug in kst: the vector field in plotDialog (PSD) refers to "
                << "a non existant vector...." << endl;
    }

    KstVectorPtr p = *i;
    KST::vectorList.lock().readUnlock();

    /* create the psd curve */
    curve = new KstPSDCurve(tag_name, p, new_freq, new_len,
                            VectorUnits->text(), RateUnits->text(),
                            _curveAppearance->color());
  }

  curve->setHasPoints(_curveAppearance->showPoints());
  curve->setHasLines(_curveAppearance->showLines());
  curve->Point.setType(_curveAppearance->pointType());
  curve->setAppodize(Appodize->isChecked());
  curve->setRemoveMean(RemoveMean->isChecked());

  KstPlot *plot = 0L;
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

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(curve.data());
  KST::dataObjectList.lock().writeUnlock();
  curve = 0L;
  emit modified();
}

void KstPsdDialogI::edit_I() {
  int index;
  KstPSDCurvePtr curve;
  double new_freq;
  int new_len;

  index = Select->currentItem();
  KstPSDCurveList curves = kstObjectSubList<KstDataObject, KstPSDCurve>(KST::dataObjectList);
  if (index < 0 || unsigned(index) >= curves.count()) {
    new_I();
    return;
  }
  curve = curves[index];

  /* verify that the curve name is unique */
  QString tag_name = Select->currentText();

  if (tag_name != curve->tagName()) {
    if (KST::dataTagNameNotUnique(tag_name)) {
      Select->setFocus();
      return;
    }
  }
  curve->setTagName(tag_name);
  KST::vectorList.lock().readLock();
  curve->setVector(*KST::vectorList.findTag(_vector->selectedVector()));
  KST::vectorList.lock().readUnlock();

  curve->setColor(_curveAppearance->color());
  curve->setHasPoints(_curveAppearance->showPoints());
  curve->setHasLines(_curveAppearance->showLines());
  curve->Point.setType(_curveAppearance->pointType());

  /* find new_freq */
  new_freq = SampRate->text().toDouble();
  if (new_freq <= 0) {
    KMessageBox::sorry(NULL, i18n("The sample rate must be greater than 0"));
    return;
  }

  /* find new_len */
  new_len = FFTLen->text().toInt();
  if (new_len<2) {
    KMessageBox::sorry(NULL, i18n("The FFT length must be greater than 2^2"));
    return;
  }

  curve->setFreq(new_freq);
  curve->setLen(new_len);

  curve->VUnits = VectorUnits->text();
  curve->RUnits = RateUnits->text();

  curve->setAppodize(Appodize->isChecked());
  curve->setRemoveMean(RemoveMean->isChecked());

  curve->update();
  curve = 0L;
  curves.clear();
  emit modified();
}

void KstPsdDialogI::delete_I() {
  int index, i_plot, i_curve;
  KstPSDCurvePtr curve;
  KstPlot *plot;

  index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active PSD to delete."));
    return;
  }

  KstPSDCurveList curves = kstObjectSubList<KstDataObject, KstPSDCurve>(KST::dataObjectList);
  curve = curves[index];
  if (curve->slaveVectorsUsed()) {
    KMessageBox::sorry(0L, i18n("Cannot delete: This PSD is used by at least one curve. Delete curves first."));
    return;
  }

  /* delete the curve from all plots */
  for (i_plot = 0; i_plot<(int)KST::plotList.count(); i_plot++) {
    plot = KST::plotList.at(i_plot);
    for (i_curve=0; i_curve<(int)plot->Curves.count(); i_curve++) {
      if (plot->Curves[i_curve]->tagName() == curve->tagName()) {
        plot->Curves.remove(plot->Curves[i_curve]);
        i_curve--;
      }
    }
  }

  /* delete the curve */
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.remove(curve.data());
  KST::dataObjectList.lock().writeUnlock();
  curve = 0L;
  curves.clear();
  emit modified();
}

#include "kstpsddialog_i.moc"
// vim: ts=2 sw=2 et
