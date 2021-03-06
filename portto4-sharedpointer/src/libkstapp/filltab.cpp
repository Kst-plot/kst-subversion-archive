/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                   netterfield@astro.utoronto.ca                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filltab.h"

#include <QDebug>

namespace Kst {

FillTab::FillTab(QWidget *parent)
  : DialogTab(parent), _multiEdit(false)
 {

  setupUi(this);
  setTabTitle(tr("Fill"));

  _style->addItem("NoBrush", Qt::NoBrush);
  _style->addItem("SolidPattern", Qt::SolidPattern);
  _style->addItem("Dense1Pattern", Qt::Dense1Pattern);
  _style->addItem("Dense2Pattern", Qt::Dense2Pattern);
  _style->addItem("Dense3Pattern", Qt::Dense3Pattern);
  _style->addItem("Dense4Pattern", Qt::Dense4Pattern);
  _style->addItem("Dense5Pattern", Qt::Dense5Pattern);
  _style->addItem("Dense6Pattern", Qt::Dense6Pattern);
  _style->addItem("Dense7Pattern", Qt::Dense7Pattern);
  _style->addItem("HorPattern", Qt::HorPattern);
  _style->addItem("VerPattern", Qt::VerPattern);
  _style->addItem("CrossPattern", Qt::CrossPattern);
  _style->addItem("BDiagPattern", Qt::BDiagPattern);
  _style->addItem("FDiagPattern", Qt::FDiagPattern);
  _style->addItem("DiagCrossPattern", Qt::DiagCrossPattern);

  connect(_color, SIGNAL(changed(const QColor &)), this, SIGNAL(modified()));
  connect(_style, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
  connect(_gradientEditor, SIGNAL(changed(const QGradient &)), this, SIGNAL(modified()));
  connect(_gradientReset, SIGNAL(pressed()), this, SIGNAL(modified()));
  connect(_useGradient, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_gradientReset, SIGNAL(pressed()), _gradientEditor, SLOT(resetGradient()));
  connect(_useGradient, SIGNAL(stateChanged(int)), this, SLOT(updateButtons()));

  updateButtons();
}


FillTab::~FillTab() {
}


void FillTab::updateButtons() { 
  if (!_multiEdit) {
    _color->setEnabled(!_useGradient->isChecked());
    _style->setEnabled(!_useGradient->isChecked());
    _gradientReset->setEnabled(_useGradient->isChecked());
    _gradientEditor->setEnabled(_useGradient->isChecked());
  }
}


QColor FillTab::color() const {
  return _color->color();
}


void FillTab::setColor(const QColor &color) {
  if (color.isValid()) {
    _color->setColor(color);
  } else {
    _color->setColor(Qt::white);
  }
}


bool FillTab::colorDirty() const {
  return _color->colorDirty();
}


Qt::BrushStyle FillTab::style() const {
  return Qt::BrushStyle(_style->itemData(_style->currentIndex()).toInt());
}


bool FillTab::styleDirty() const {
  return _style->currentIndex() != -1;
}


void FillTab::setStyle(Qt::BrushStyle style) {
  if (style == Qt::LinearGradientPattern) {
    _style->setCurrentIndex(Qt::SolidPattern);
  } else {
    _style->setCurrentIndex(_style->findData(QVariant(style)));
  }
}


QGradient FillTab::gradient() const {
  if (_useGradient->isChecked()) {
    return _gradientEditor->gradient();
  } else {
    return QGradient();
  }
}


bool FillTab::gradientDirty() const {
  return _gradientEditor->dirty();
}


void FillTab::setGradient(const QGradient &gradient) {
  _useGradient->setChecked(!gradient.stops().empty());
  _gradientEditor->setGradient(gradient);
  updateButtons();
}


bool FillTab::useGradient() const {
  return _useGradient->isChecked();
}


bool FillTab::useGradientDirty() const {
  return _useGradient->checkState() != Qt::PartiallyChecked;
}


void FillTab::setUseGradient(const bool useGradient) {
  _useGradient->setChecked(useGradient);
  updateButtons();
}


void FillTab::clearTabValues() {
  _useGradient->setCheckState(Qt::PartiallyChecked);
  _style->setCurrentIndex(-1);

  _color->clearSelection();

  _color->setEnabled(true);
  _style->setEnabled(true);
  _gradientReset->setEnabled(true);
  _gradientEditor->setEnabled(true);
}


void FillTab::enableSingleEditOptions(bool enabled) {
  _multiEdit = !enabled;
  if (enabled) {
    _useGradient->setTristate(false);
  }
}
}

// vim: ts=2 sw=2 et
