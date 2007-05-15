/***************************************************************************
                       kstvvdialog_i.h  -  Dialog for KstVectorView objects.
                             -------------------
    begin                :
    copyright            : (C) 2007 Kst
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
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qvbox.h>

// include files for KDE
#include <kcombobox.h>
#include "ksdebug.h"
#include <kmessagebox.h>

// application specific includes
#include "kstvvdialog_i.h"
#include "curveappearancewidget.h"
#include "curveplacementwidget.h"
#include "editmultiplewidget.h"
#include "vectorviewdialogwidget.h"
#include "kst2dplot.h"
#include "kstdataobjectcollection.h"
#include "kstsettings.h"
#include "kstuinames.h"
#include "kstvcurve.h"
#include "kstviewwindow.h"
#include "vectorselector.h"
#include "scalarselector.h"

const QString& KstVvDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<KstVvDialogI> KstVvDialogI::_inst;

KstVvDialogI *KstVvDialogI::globalInstance() {
  if (!_inst) {
    _inst = new KstVvDialogI(KstApp::inst());
  }
  return _inst;
}


KstVvDialogI::KstVvDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new VectorViewDialogWidget(_contents);
  setMultiple(false);

  connect(_w->_xVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));
  connect(_w->_yVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));

  connect(_w->_FlagVector, SIGNAL(newVectorCreated(const QString&)), this, SIGNAL(modified()));

  connect(_w->_xMinScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_xMaxScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_yMinScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));
  connect(_w->_yMaxScalar, SIGNAL(newScalarCreated()), this, SIGNAL(modified()));

  _w->_FlagVector->provideNoneVector(true);
  _w->_FlagVector->allowNewVectors(false);

  connect(_w->_xMinCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_w->_xMaxCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_w->_yMinCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));
  connect(_w->_yMaxCheckbox, SIGNAL(clicked()), this, SLOT(updateButtons()));

  _w->_xMinScalar->allowDirectEntry(true);
  _w->_xMaxScalar->allowDirectEntry(true);
  _w->_yMinScalar->allowDirectEntry(true);
  _w->_yMaxScalar->allowDirectEntry(true);

  connect(_w->_realtimeButton, SIGNAL(clicked()), this, SLOT(realtimeClicked()));
  connect(_w->_currentButton, SIGNAL(clicked()), this, SLOT(currentClicked()));

  QColor qc = _w->_curveAppearance->color();
  _w->_curveAppearance->setValue(false, false, true, qc, 0, 0, 0, 1, 0);
}


KstVvDialogI::~KstVvDialogI() {
}


void KstVvDialogI::updateWindow() {
  _w->_curvePlacement->update();
}

void KstVvDialogI::updatePlotList()
{
    QString old;
    if (_w->_plotList->count()) {
	old = _w->_plotList->currentText();
    }

    QStringList plots = KstData::self()->plotList();
    _w->_plotList->clear();
    for (QStringList::ConstIterator i = plots.begin(); i != plots.end(); ++i) {
	_w->_plotList->insertItem(*i);
    }

    if (!old.isNull() && plots.contains(old)) {
	_w->_plotList->setCurrentText(old);
    }
}


void KstVvDialogI::fillFieldsForEdit() {
  KstVectorViewPtr vp = kst_cast<KstVectorView>(_dp);
  if (!vp) {
    return; // shouldn't be needed
  }

  vp->readLock();

  _tagName->setText(vp->tagName());

  _w->_xVector->setSelection(vp->in_xVTag());
  _w->_yVector->setSelection(vp->in_yVTag());

  if (vp->hasFlag()) { _w->_FlagVector->setSelection(vp->FlagTag()); }

  _w->_xMinCheckbox->setChecked(vp->useXmin());
  _w->_xMaxCheckbox->setChecked(vp->useXmax());
  _w->_yMinCheckbox->setChecked(vp->useYmin());
  _w->_yMaxCheckbox->setChecked(vp->useYmax());

  if (vp->xMinScalar()) { _w->_xMinScalar->setSelection(vp->xMinScalar()->tag().displayString()); }
  if (vp->xMaxScalar()) { _w->_xMaxScalar->setSelection(vp->xMaxScalar()->tag().displayString()); }
  if (vp->yMinScalar()) { _w->_yMinScalar->setSelection(vp->yMinScalar()->tag().displayString()); }
  if (vp->yMaxScalar()) { _w->_yMaxScalar->setSelection(vp->yMaxScalar()->tag().displayString()); }

  vp->unlock();
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


void KstVvDialogI::fillFieldsForNew() {
  // set tag name
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);
  _legendText->show();
  _legendLabel->show();

  // set the curve placement window
  _w->_curvePlacement->update();

  // for some reason the lower widget needs to be shown first to prevent overlapping?
  _w->_curveAppearance->hide();
  _w->_curvePlacement->show();
  _w->_curveAppearance->show();
  _w->_curveAppearance->reset();

  QColor qc = _w->_curveAppearance->color();
  _w->_curveAppearance->setValue(true, false, false, qc, 0, 0, 0, 0, 0);

  updateButtons();
  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}


void KstVvDialogI::update() {
  _w->_curvePlacement->update();
  _w->_xVector->update();
  _w->_yVector->update();

  _w->_FlagVector->update();

  _w->_xMinScalar->update();
  _w->_xMaxScalar->update();
  _w->_yMinScalar->update();
  _w->_yMaxScalar->update();

  updatePlotList();
}


bool KstVvDialogI::newObject() {
  QString tag_name = _tagName->text();
  if (tag_name == defaultTag) {
    tag_name = KST::suggestVectorViewName(KstObjectTag::fromString(_w->_xVector->selectedVector()));
  }

  // verify that the curve name is unique
  if (KstData::self()->dataTagNameNotUnique(tag_name)) {
    _tagName->setFocus();
    return false;
  }

  if (_w->_xVector->selectedVector().isEmpty()) {
    KMessageBox::sorry(this, i18n("New VectorView not made: define X vector first."));
    return false;
  }

  if (_w->_yVector->selectedVector().isEmpty()) {
    KMessageBox::sorry(this, i18n("New VectorView not made: define Y vector first."));
    return false;
  }

  KstVectorViewPtr vv;

  KST::vectorList.lock().readLock();
  KstVectorPtr vx = *KST::vectorList.findTag(_w->_xVector->selectedVector());
  KstVectorPtr vy = *KST::vectorList.findTag(_w->_yVector->selectedVector());
  KST::vectorList.lock().unlock();
  if (!vx) {
    kstdFatal() << "Bug in kst: the Vector field (Vx) refers to "
                << " a non existent vector..." << endl;
  }
  if (!vy) {
    kstdFatal() << "Bug in kst: the Vector field (Vy) refers to "
                << " a non existent vector..." << endl;
  }

  vx->readLock();
  vy->readLock();
  vv = new KstVectorView(tag_name, vx, vy);
  vx->unlock();
  vy->unlock();

  KstVCurvePtr vc = new KstVCurve(KST::suggestCurveName(vv->tag(), true), vv->vX(), vv->vY(), 0L, 0L, 0L, 0L, _w->_curveAppearance->color());

  vc->setHasPoints(_w->_curveAppearance->showPoints());
  vc->setHasLines(_w->_curveAppearance->showLines());
  vc->setHasBars(_w->_curveAppearance->showBars());
  vc->pointType = _w->_curveAppearance->pointType();
  vc->setLineWidth(_w->_curveAppearance->lineWidth());
  vc->setLineStyle(_w->_curveAppearance->lineStyle());
  vc->setBarStyle(_w->_curveAppearance->barStyle());
  vc->setPointDensity(_w->_curveAppearance->pointDensity());

  QString legend_text = _legendText->text();
  if (legend_text == defaultTag) {
    vc->setLegendText(QString(""));
  } else {
    vc->setLegendText(legend_text);
  }

  KstViewWindow *w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(_w->_curvePlacement->_plotWindow->currentText()));
  if (!w) {
    QString n = KstApp::inst()->newWindow(KST::suggestWinName());
    w = static_cast<KstViewWindow*>(KstApp::inst()->findWindow(n));
  }
  if (w) {
    Kst2DPlotPtr plot;
    if (_w->_curvePlacement->existingPlot()) {
      /* assign curve to plot */
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(_w->_curvePlacement->plotName()));
      if (plot) {
        plot->addCurve(vc.data());
      }
    }

    if (_w->_curvePlacement->newPlot()) {
      /* assign curve to plot */
      QString name = w->createPlot(KST::suggestPlotName());
      if (_w->_curvePlacement->reGrid()) {
        w->view()->cleanup(_w->_curvePlacement->columns());
      }
      plot = kst_cast<Kst2DPlot>(w->view()->findChild(name));
      if (plot) {
        _w->_curvePlacement->update();
        _w->_curvePlacement->setCurrentPlot(plot->tagName());
        plot->addCurve(vc.data());
        plot->generateDefaultLabels();
      }
    }
  }

  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(vv.data());
  KST::dataObjectList.append(vc.data());
  KST::dataObjectList.lock().unlock();

  vv = 0L;
  vc = 0L;
  emit modified();

  return true;
}


bool KstVvDialogI::editSingleObject(KstVectorViewPtr vvPtr) {
  if (_xVectorDirty) {
    KST::vectorList.lock().readLock();
    vvPtr->setXVector(*KST::vectorList.findTag(_w->_xVector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  if (_yVectorDirty) {
    KST::vectorList.lock().readLock();
    vvPtr->setYVector(*KST::vectorList.findTag(_w->_yVector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  if (_interpTypeDirty) {
    vvPtr->setInterp(KstVectorView::InterpType(_w->_interp->currentItem()));
  }

  if (_xMinScalarDirty) {
    KstScalarPtr s = _w->_xMinScalar->selectedScalarPtr();
    if (s) {vvPtr->setXminScalar(s); }
  }

  if (_xMaxScalarDirty) {
    KstScalarPtr s = _w->_xMaxScalar->selectedScalarPtr();
    if (s) {vvPtr->setXmaxScalar(s); }
  }

  if (_yMinScalarDirty) {
    KstScalarPtr s = _w->_yMinScalar->selectedScalarPtr();
    if (s) {vvPtr->setYminScalar(s); }
  }

  if (_yMaxScalarDirty) {
    KstScalarPtr s = _w->_yMaxScalar->selectedScalarPtr();
    if (s) {vvPtr->setYmaxScalar(s); }
  }

  if (_FlagVectorDirty) {
    KST::vectorList.lock().readLock();
    vvPtr->setFlagVector(*KST::vectorList.findTag(_w->_FlagVector->selectedVector()));
    KST::vectorList.lock().unlock();
  }

  if (_useXminDirty) {
    vvPtr->setUseXmin(_w->_xMinCheckbox->isChecked());
  }

  if (_useXmaxDirty) {
    vvPtr->setUseXmax(_w->_xMaxCheckbox->isChecked());
  }

  if (_useYminDirty) {
    vvPtr->setUseYmin(_w->_yMinCheckbox->isChecked());
  }

  if (_useYmaxDirty) {
    vvPtr->setUseYmax(_w->_yMaxCheckbox->isChecked());
  }

  vvPtr->writeLock();
  vvPtr->setDirty();
  vvPtr->unlock();

  return true;
}


bool KstVvDialogI::editObject() {

  KstVectorViewList vvList = kstObjectSubList<KstDataObject,KstVectorView>(KST::dataObjectList);

  if (_editMultipleMode) {
    //currently no edit multiple for vectorviews.
  } else {
    KstVectorViewPtr vp = kst_cast<KstVectorView>(_dp);
    // verify that the curve name is unique
    QString tag_name = _tagName->text();
    if (!vp || (tag_name != vp->tagName() && KstData::self()->dataTagNameNotUnique(tag_name))) {
      _tagName->setFocus();
      return false;
    }
    vp->setTagName(KstObjectTag(tag_name, vp->tag().context())); // FIXME: doesn't allow changing tag context

    _xVectorDirty = true;
    _yVectorDirty = true;
    _interpTypeDirty = true;

    _useXminDirty = true;
    _xMinScalarDirty = true;

    _useXmaxDirty = true;
    _xMaxScalarDirty = true;

    _useYminDirty = true;
    _yMinScalarDirty = true;

    _useYmaxDirty = true;
    _yMaxScalarDirty = true;

    _FlagVectorDirty = true;

    // then edit the object
    if (!editSingleObject(vp)) {
      return false;
    }
  }
  emit modified();

  return true;
}

void KstVvDialogI::updateButtons() {
  _w->_xMinScalar->setEnabled(_w->_xMinCheckbox->isChecked());
  _w->_xMaxScalar->setEnabled(_w->_xMaxCheckbox->isChecked());
  _w->_yMinScalar->setEnabled(_w->_yMinCheckbox->isChecked());
  _w->_yMaxScalar->setEnabled(_w->_yMaxCheckbox->isChecked());
}

void KstVvDialogI::realtimeClicked() {
  Kst2DPlotPtr plot = Kst2DPlot::findPlotByName(_w->_plotList->currentText()); 

  _w->_xMinCheckbox->setChecked(true);
  _w->_xMinScalar->setSelection((plot->scalars())["xmin"]->tag().displayString());

  _w->_xMaxCheckbox->setChecked(true);
  _w->_xMaxScalar->setSelection((plot->scalars())["xmax"]->tag().displayString());

  _w->_yMinCheckbox->setChecked(true);
  _w->_yMinScalar->setSelection((plot->scalars())["ymin"]->tag().displayString());

  _w->_yMaxCheckbox->setChecked(true);
  _w->_yMaxScalar->setSelection((plot->scalars())["ymax"]->tag().displayString());

  updateButtons();
}

void KstVvDialogI::currentClicked() {
    Kst2DPlotPtr plot = Kst2DPlot::findPlotByName(_w->_plotList->currentText());

    KstScalarPtr sp;
    double v;

    _w->_xMinCheckbox->setChecked(true);
    v = (plot->scalars())["xmin"]->value();
    _w->_xMinScalar->setSelection(QString::number(v));

    _w->_xMaxCheckbox->setChecked(true);
    v = (plot->scalars())["xmax"]->value();
    _w->_xMaxScalar->setSelection(QString::number(v));

    _w->_yMinCheckbox->setChecked(true);
    v = (plot->scalars())["ymin"]->value();
    _w->_yMinScalar->setSelection(QString::number(v));

    _w->_yMaxCheckbox->setChecked(true);
    v = (plot->scalars())["ymax"]->value();
    _w->_yMaxScalar->setSelection(QString::number(v));

    updateButtons();
}

void KstVvDialogI::populateEditMultiple() {
}

void KstVvDialogI::cleanup() {
}

void KstVvDialogI::setXVector(const QString& name) {
  _w->_xVector->setSelection(name);
}

void KstVvDialogI::setYVector(const QString& name) {
  _w->_xVector->setSelection(name);
}


#include "kstvvdialog_i.moc"
// vim: ts=2 sw=2 et
