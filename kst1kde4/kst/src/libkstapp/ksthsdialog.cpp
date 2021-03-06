/***************************************************************************
                      ksthsdialog.cpp  -  Part of KST
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
#include <QFontMetrics>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyle>

#include "ksthsdialog.h"
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "kst2dplot.h"
#include "kstchoosecolordialog.h"
#include "kstdataobjectcollection.h"
#include "kstsettings.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "vectorselector.h"

const QString& KstHsDialog::defaultTag = KGlobal::staticQString("<Auto Name>");

QPointer<KstHsDialog> KstHsDialog::_inst;

KstHsDialog *KstHsDialog::globalInstance() {
  if (!_inst) {
    _inst = new KstHsDialog(KstApp::inst());
  }
  return _inst;
}


KstHsDialog::KstHsDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
: KstDataDialog(parent) {
  _w = new Ui::HistogramDialogWidget();
  _w->setupUi(_contents);

  setMultiple(true);

  connect(_w->AutoBin, SIGNAL(clicked()), this, SLOT(autoBin()));
  connect(_w->_vector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_realTimeAutoBin, SIGNAL(clicked()), this, SLOT(updateButtons()));

  //
  // connections for multiple edit mode...
  //

  connect(_w->_realTimeAutoBin, SIGNAL(clicked()), this, SLOT(setRealTimeAutoBinDirty()));
  connect(_w->NormIsPercent, SIGNAL(clicked()), this, SLOT(setNormIsPercentDirty()));
  connect(_w->NormIsFraction, SIGNAL(clicked()), this, SLOT(setNormIsFractionDirty()));
  connect(_w->PeakIs1, SIGNAL(clicked()), this, SLOT(setPeakIs1Dirty()));
  connect(_w->NormIsNumber, SIGNAL(clicked()), this, SLOT(setNormIsNumberDirty()));

  //
  // connections for apply button...
  //

  connect(_w->_vector, SIGNAL(selectionChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->_vector, SIGNAL(selectionChangedLabel(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->Min, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->Max, SIGNAL(textChanged(const QString&)), this, SLOT(wasModifiedApply()));
  connect(_w->AutoBin, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_realTimeAutoBin, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->NormIsPercent, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->NormIsFraction, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->NormIsNumber, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->PeakIs1, SIGNAL(clicked()), this, SLOT(wasModifiedApply()));
  connect(_w->_curveAppearance, SIGNAL(changed()), this, SLOT(wasModifiedApply()));

  QColor qc = _w->_curveAppearance->color();
  _w->_curveAppearance->setValue(false, false, true, qc, 0, 0, 0, 1, 0);
  _w->N->setMinimumWidth(fontMetrics().width("888888") + QStyle::PM_ScrollBarExtent );
}


KstHsDialog::~KstHsDialog() {
}


void KstHsDialog::updateWindow() {
  _w->_curvePlacement->update();
}


void KstHsDialog::fillFieldsForEdit() {
  KstHistogramPtr hp;

  hp = kst_cast<KstHistogram>(_dp);
  if (hp) {
    hp->readLock();
  
    _tagName->setText(hp->tagName());
  
    _w->_vector->setSelection(hp->vTag());
  
    _w->N->setValue(hp->nBins());
    _w->Min->setText(QString::number(hp->vX()->min() - (hp->width()/2.0)));
    _w->Max->setText(QString::number(hp->vX()->max() + (hp->width()/2.0)));
    _w->_realTimeAutoBin->setChecked(hp->realTimeAutoBin());
  
    if (hp->isNormPercent()) {
      _w->NormIsPercent->setChecked(true);
    } else if (hp->isNormFraction()) {
      _w->NormIsFraction->setChecked(true);
    } else if (hp->isNormOne()) {
      _w->PeakIs1->setChecked(true);
    } else {
      _w->NormIsNumber->setChecked(true);
    }
  
  
    hp->unlock();
    updateButtons();
  
    // can't edit curve props from here....
    _w->_curveAppearance->hide();
    _w->_curvePlacement->hide();
    _legendText->hide();
    _legendLabel->hide();
  
    adjustSize();
    resize(minimumSizeHint());
    setFixedHeight(height());
  }
}


void KstHsDialog::fillFieldsForNew() {
  QColor color;

  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);
  _legendText->show();
  _legendLabel->show();

  _w->_curvePlacement->update();

  //
  // for some reason the lower widget needs to be shown first to prevent overlapping?...
  //

  _w->_curveAppearance->hide();
  _w->_curvePlacement->show();
  _w->_curveAppearance->show();
  _w->_curveAppearance->reset();

  color = _w->_curveAppearance->color();
  _w->_curveAppearance->setValue(false, false, true, color, 0, 0, 0, 1, 0);

  _w->_realTimeAutoBin->setChecked(false);
  updateButtons();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstHsDialog::update() {
  _w->_curvePlacement->update();
  _w->_vector->update();
}


bool KstHsDialog::newObject() {
  QString tag_name = _tagName->text();

  if (tag_name == defaultTag) {
    tag_name = KST::suggestHistogramName(KstObjectTag::fromString(_w->_vector->selectedVector()));
  }

  //
  // verify that the curve name is unique...
  //

  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (_w->_vector->selectedVector().isEmpty()) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("New Histogram not made: define vectors first."));
    return false;
  }

  //
  // find max and min...
  //

  double new_min = _w->Min->text().toDouble();
  double new_max = _w->Max->text().toDouble();

  if (new_max < new_min) {
    double m = new_max;

    new_max = new_min;
    new_min = m;
  }

  if (new_max == new_min) {
    QMessageBox::warning(this, QObject::tr("kst"), QObject::tr("Max and Min can not be equal."));
    return false;
  }

  int new_n_bins = _w->N->text().toInt();
  if (new_n_bins < 1) {
    QMessageBox::warning(this, QObject::tr("kst"), QObject::tr("You must have one or more bins in a histogram."));
    return false;
  }

  KstHsNormType new_norm_mode;
  if (_w->NormIsPercent->isChecked()) {
    new_norm_mode = KST_HS_PERCENT;
  } else if (_w->NormIsFraction->isChecked()) {
    new_norm_mode = KST_HS_FRACTION;
  } else if (_w->PeakIs1->isChecked()) {
    new_norm_mode = KST_HS_MAX_ONE;
  } else {
    new_norm_mode = KST_HS_NUMBER;
  }

  KstHistogramPtr hs;

  KST::vectorList.lock().readLock();
  KstVectorPtr vp = *KST::vectorList.findTag(_w->_vector->selectedVector());
  KST::vectorList.lock().unlock();
  if (vp) {
    KstVCurvePtr vc;
    KstViewWindow *w;
    QColor color;

    vp->readLock();
    hs = new KstHistogram(tag_name, vp, new_min, new_max,
                          new_n_bins, new_norm_mode);
    vp->unlock();

    hs->setRealTimeAutoBin(_w->_realTimeAutoBin->isChecked());
  
// xxx    color = KstApp::inst()->chooseColorDlg()->getColorForCurve(hs->vX(), hs->vY());
    if (!color.isValid()) {
      color = _w->_curveAppearance->color();
    }
    vc = new KstVCurve(KST::suggestCurveName(hs->tag(), true), hs->vX(), hs->vY(), KstVectorPtr(), KstVectorPtr(), KstVectorPtr(), KstVectorPtr(), color);
  
    vc->setHasPoints(_w->_curveAppearance->showPoints());
    vc->setHasLines(_w->_curveAppearance->showLines());
    vc->setHasBars(_w->_curveAppearance->showBars());
    vc->setPointStyle(_w->_curveAppearance->pointType());
    vc->setLineWidth(_w->_curveAppearance->lineWidth());
    vc->setLineStyle(_w->_curveAppearance->lineStyle());
    vc->setBarStyle(_w->_curveAppearance->barStyle());
    vc->setPointDensity(_w->_curveAppearance->pointDensity());
  
    QString legendText = _legendText->text();

    if (legendText == defaultTag) {
      vc->setLegendText(QString(""));
    } else {
      vc->setLegendText(legendText);
    }
/* xxx  
    w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_w->_curvePlacement->_plotWindow->currentText()));
*/
    if (!w) {
      QString n = KstApp::inst()->newWindow(KST::suggestWinName());

// xxx      w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
    }
    
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
    KST::dataObjectList.append(hs);
    KST::dataObjectList.append(vc);
    KST::dataObjectList.lock().unlock();
  
    hs = 0L;
    vc = 0L;

    emit modified();
  }

  return true;
}


bool KstHsDialog::editSingleObject(KstHistogramPtr hsPtr) {
  double new_min;
  double new_max;

  hsPtr->readLock();
  new_min = hsPtr->xMin();
  new_max = hsPtr->xMax();
  hsPtr->unlock();

  if (_minDirty) {
    new_min = _w->Min->text().toDouble();
  }

  if (_maxDirty) {
    new_max = _w->Max->text().toDouble();
  }

  if (new_max < new_min) {
    double m = new_max;

    new_max = new_min;
    new_min = m;
  }

  if (new_max == new_min) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Max and Min can not be equal."));
    _w->Min->setFocus();
    return false;
  }

  int new_n_bins = _w->N->text().toInt();
  if (_nDirty && new_n_bins < 1) {
    QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("You must have one or more bins in a histogram."));
    _w->N->setFocus();
    return false;
  }

  if (_vectorDirty) {
    KST::vectorList.lock().readLock();
    hsPtr->setVector(*KST::vectorList.findTag(_w->_vector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  hsPtr->writeLock();

  if (_nDirty) {
    hsPtr->setNBins(new_n_bins);
  }

  if (_minDirty || _maxDirty) {
    hsPtr->setXRange(new_min, new_max);
  }

  if (_realTimeAutoBinDirty) {
    hsPtr->setRealTimeAutoBin(_w->_realTimeAutoBin->isChecked());
  }

  if (_normIsPercentDirty || _normIsFractionDirty || _peakIs1Dirty || _normIsNumberDirty) {
    if (_w->NormIsPercent->isChecked()) {
      hsPtr->setIsNormPercent();
    } else if (_w->NormIsFraction->isChecked()) {
      hsPtr->setIsNormFraction();
    } else if (_w->PeakIs1->isChecked()) {
      hsPtr->setIsNormOne();
    } else {
      hsPtr->setIsNormNum();
    }
  }

  hsPtr->setRecursed(false);
  if (hsPtr->recursion()) {
    hsPtr->setRecursed(true);
    hsPtr->unlock();
    QMessageBox::critical(this, QObject::tr("Kst"), QObject::tr("There is a recursion resulting from the histogram you entered."));
    return false;
  }

  hsPtr->setDirty();
  hsPtr->unlock();

  return true;
}


bool KstHsDialog::editObject() {
  KstHistogramList hsList;

  hsList = kstObjectSubList<KstDataObject,KstHistogram>(KST::dataObjectList);

  if (_editMultipleMode) {
    _vectorDirty = _w->_vector->_vector->currentIndex() != 0;
    _nDirty = _w->N->text() != " ";
    _minDirty = !_w->Min->text().isEmpty();
    _maxDirty = !_w->Max->text().isEmpty();

    bool didEdit = false;
    int i;

    for (i = 0; i < _editMultipleWidget->_objectList->count(); i++) {
      if (_editMultipleWidget->_objectList->item(i)->isSelected()) {
        KstHistogramList::iterator hsIter;
        KstHistogramPtr hsPtr;

        hsIter = hsList.findTag(_editMultipleWidget->_objectList->item(i)->text());
        if (hsIter == hsList.end()) {
          return false;
        }

        hsPtr = *hsIter;

        if (!editSingleObject(hsPtr)) {
          return false;
        }
        didEdit = true;
      }
    }
    if (!didEdit) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("Select one or more objects to edit."));
      return false;
    }
  } else {
    KstHistogramPtr hp;
    QString tag_name;

    hp = kst_cast<KstHistogram>(_dp);

    //
    // verify that the curve name is unique...
    //

    tag_name = _tagName->text();
    if (!hp || (tag_name != hp->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();
      return false;
    }

    hp->writeLock();
    hp->setTagName(tag_name);
    hp->unlock();

    //
    // then edit the object...
    //

    _vectorDirty = true;
    _minDirty = true;
    _maxDirty = true;
    _nDirty = true;
    _realTimeAutoBinDirty = true;
    _normIsPercentDirty = true;
    _normIsFractionDirty = true;
    _peakIs1Dirty = true;
    _normIsNumberDirty = true;
    if (!editSingleObject(hp)) {
      return false;
    }
  }

  emit modified();

  return true;
}


void KstHsDialog::autoBin() {
  KstReadLocker ml(&KST::vectorList.lock());

  if (!KST::vectorList.isEmpty()) {
    KstVectorList::iterator i;
    double max;
    double min;
    int n;

    i = KST::vectorList.findTag(_w->_vector->selectedVector());
    if (i != KST::vectorList.end()) {
      (*i)->readLock();
      KstHistogram::AutoBin(KstVectorPtr(*i), &n, &max, &min);
      (*i)->unlock();

      _w->N->setValue(n);
      _w->Min->setText(QString::number(min));
      _w->Max->setText(QString::number(max));
    }
  }
}


void KstHsDialog::updateButtons() {
  if (!_editMultipleMode && _w->_realTimeAutoBin->isChecked()) {
    autoBin();
  }

  _w->Min->setEnabled(!_w->_realTimeAutoBin->isChecked());
  _w->Max->setEnabled(!_w->_realTimeAutoBin->isChecked());
  _w->N->setEnabled(!_w->_realTimeAutoBin->isChecked());
  _w->AutoBin->setEnabled(!_w->_realTimeAutoBin->isChecked() && !_editMultipleMode);
}


void KstHsDialog::populateEditMultiple() {
  KstHistogramList hslist;

  hslist = kstObjectSubList<KstDataObject,KstHistogram>(KST::dataObjectList);
  _editMultipleWidget->_objectList->insertItems(0, hslist.tagNames());

  _w->Min->setText("");
  _w->Max->setText("");

  _w->N->setMinimum(_w->N->minimum() - 1);
  _w->N->setSpecialValueText(" ");
  _w->N->setValue(_w->N->minimum());

  _w->_vector->_vector->insertItem(0, "");
  _w->_vector->_vector->setCurrentIndex(0);
  _w->_realTimeAutoBin->setTristate(true);
  _w->_realTimeAutoBin->setChecked(Qt::PartiallyChecked);

  _w->NormIsPercent->setChecked(false);
  _w->NormIsFraction->setChecked(false);
  _w->PeakIs1->setChecked(false);
  _w->NormIsNumber->setChecked(false);
  _tagName->setText("");
  _tagName->setEnabled(false);
  _w->AutoBin->setEnabled(false);

  _w->Min->setEnabled(true);
  _w->Max->setEnabled(true);

  _minDirty = false;
  _maxDirty = false;
  _nDirty = false;
  _vectorDirty = false;
  _realTimeAutoBinDirty = false;
  _normIsPercentDirty = false;
  _normIsFractionDirty = false;
  _peakIs1Dirty = false;
  _normIsNumberDirty = false;
}


void KstHsDialog::setRealTimeAutoBinDirty() {
  _w->_realTimeAutoBin->setTristate(false);
  _realTimeAutoBinDirty = true;
}


void KstHsDialog::cleanup() {
  if (_editMultipleMode) {
    _w->N->setMinimum(_w->N->minimum() + 1);
    _w->N->setSpecialValueText(QString::null);
  }
}


void KstHsDialog::setVector(const QString& name) {
  _w->_vector->setSelection(name);
}

#include "ksthsdialog.moc"
