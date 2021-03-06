/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "powerspectrumdialog.h"

#include "dialogpage.h"

#include "psd.h"

#include "view.h"
#include "plotitem.h"
#include "tabwidget.h"
#include "mainwindow.h"
#include "application.h"
#include "plotrenderitem.h"
#include "curve.h"
#include "document.h"
#include "objectstore.h"

#include "defaultnames.h"
#include "datacollection.h"
#include "dataobjectcollection.h"

namespace Kst {

PowerSpectrumTab::PowerSpectrumTab(QWidget *parent)
  : DataTab(parent) {

  setupUi(this);
  setTabTitle(tr("Power Spectrum"));

}


PowerSpectrumTab::~PowerSpectrumTab() {
}


VectorPtr PowerSpectrumTab::vector() const {
  return _vector->selectedVector();
}


CurveAppearance* PowerSpectrumTab::curveAppearance() const {
  return _curveAppearance;
}


CurvePlacement* PowerSpectrumTab::curvePlacement() const {
  return _curvePlacement;
}


FFTOptions* PowerSpectrumTab::FFTOptionsWidget() const {
  return _FFTOptions;
}


PowerSpectrumDialog::PowerSpectrumDialog(ObjectPtr dataObject, QWidget *parent)
  : DataDialog(dataObject, parent) {

  if (editMode() == Edit)
    setWindowTitle(tr("Edit Power Spectrum"));
  else
    setWindowTitle(tr("New Power Spectrum"));

  _powerSpectrumTab = new PowerSpectrumTab(this);
  addDataTab(_powerSpectrumTab);

  //FIXME need to do validation to enable/disable ok button...
}


PowerSpectrumDialog::~PowerSpectrumDialog() {
}


QString PowerSpectrumDialog::tagString() const {
  return DataDialog::tagString();
}


ObjectPtr PowerSpectrumDialog::createNewDataObject() const {
  Q_ASSERT(_document && _document->objectStore());
  ObjectTag tag = _document->objectStore()->suggestObjectTag<PSD>(tagString(), ObjectTag::globalTagContext);
  PSDPtr powerspectrum = _document->objectStore()->createObject<PSD>(tag);
  Q_ASSERT(powerspectrum);

  powerspectrum->setVector(_powerSpectrumTab->vector());
  powerspectrum->setFreq(_powerSpectrumTab->FFTOptionsWidget()->sampleRate());
  powerspectrum->setAverage(_powerSpectrumTab->FFTOptionsWidget()->interleavedAverage());
  powerspectrum->setLen(_powerSpectrumTab->FFTOptionsWidget()->FFTLength());
  powerspectrum->setApodize(_powerSpectrumTab->FFTOptionsWidget()->apodize());
  powerspectrum->setRemoveMean(_powerSpectrumTab->FFTOptionsWidget()->removeMean());
  powerspectrum->setVUnits(_powerSpectrumTab->FFTOptionsWidget()->vectorUnits());
  powerspectrum->setRUnits(_powerSpectrumTab->FFTOptionsWidget()->rateUnits());
  powerspectrum->setApodizeFxn(_powerSpectrumTab->FFTOptionsWidget()->apodizeFunction());
  powerspectrum->setGaussianSigma(_powerSpectrumTab->FFTOptionsWidget()->sigma());
  powerspectrum->setOutput(_powerSpectrumTab->FFTOptionsWidget()->output());
  powerspectrum->setInterpolateHoles(_powerSpectrumTab->FFTOptionsWidget()->interpolateOverHoles());

  powerspectrum->writeLock();
  powerspectrum->update(0);
  powerspectrum->unlock();

  //FIXME this should be a command...
  //FIXME need some smart placement...

  tag = _document->objectStore()->suggestObjectTag<Curve>(powerspectrum->tag().tagString(), ObjectTag::globalTagContext);
  CurvePtr curve = _document->objectStore()->createObject<Curve>(tag);
  Q_ASSERT(curve);

  curve->setXVector(powerspectrum->vX());
  curve->setYVector(powerspectrum->vY());
  curve->setColor(_powerSpectrumTab->curveAppearance()->color());
  curve->setHasPoints(_powerSpectrumTab->curveAppearance()->showPoints());
  curve->setHasLines(_powerSpectrumTab->curveAppearance()->showLines());
  curve->setHasBars(_powerSpectrumTab->curveAppearance()->showBars());
  curve->setLineWidth(_powerSpectrumTab->curveAppearance()->lineWidth());
  curve->setLineStyle(_powerSpectrumTab->curveAppearance()->lineStyle());
  curve->pointType = _powerSpectrumTab->curveAppearance()->pointType();
  curve->setPointDensity(_powerSpectrumTab->curveAppearance()->pointDensity());
  curve->setBarStyle(_powerSpectrumTab->curveAppearance()->barStyle());

  curve->writeLock();
  curve->update(0);
  curve->unlock();

  PlotItem *plotItem = 0;
  switch (_powerSpectrumTab->curvePlacement()->place()) {
  case CurvePlacement::NoPlot:
    break;
  case CurvePlacement::ExistingPlot:
    {
      plotItem = static_cast<PlotItem*>(_powerSpectrumTab->curvePlacement()->existingPlot());
      break;
    }
  case CurvePlacement::NewPlot:
    {
      CreatePlotForCurve *cmd = new CreatePlotForCurve(
        _powerSpectrumTab->curvePlacement()->createLayout(),
        _powerSpectrumTab->curvePlacement()->appendToLayout());
      cmd->createItem();

      plotItem = static_cast<PlotItem*>(cmd->item());
      break;
    }
  default:
    break;
  }

  PlotRenderItem *renderItem = plotItem->renderItem(PlotRenderItem::Cartesian);
  renderItem->addRelation(kst_cast<Relation>(curve));
  plotItem->update();

  return ObjectPtr(powerspectrum.data());
}


ObjectPtr PowerSpectrumDialog::editExistingDataObject() const {
  qDebug() << "editExistingDataObject" << endl;
  return 0;
}

}

// vim: ts=2 sw=2 et
