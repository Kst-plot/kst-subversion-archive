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

#include "labeltab.h"

namespace Kst {

LabelTab::LabelTab(PlotItem* plotItem, QWidget *parent)
  : DialogTab(parent), _plotItem(plotItem) {

  setupUi(this);
  setTabTitle(tr("Labels"));

  QFont font;
  setGlobalFont(font);

  connect(_topFamily, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified()));
  connect(_leftFamily, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified()));
  connect(_bottomFamily, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified()));
  connect(_rightFamily, SIGNAL(textChanged(const QString&)), this, SIGNAL(modified()));

  connect(_topFontSize, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));
  connect(_leftFontSize, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));
  connect(_bottomFontSize, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));
  connect(_rightFontSize, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));

  connect(_topBold, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_leftBold, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_bottomBold, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_rightBold, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));

  connect(_topUnderline, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_leftUnderline, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_bottomUnderline, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_rightUnderline, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));

  connect(_topItalic, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_leftItalic, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_bottomItalic, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));
  connect(_rightItalic, SIGNAL(stateChanged(int)), this, SIGNAL(modified()));

  connect(_topFamily, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
  connect(_leftFamily, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
  connect(_bottomFamily, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
  connect(_rightFamily, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));

  connect(_applyGlobalsButton, SIGNAL(pressed()), this, SLOT(applyGlobals()));
  connect(_autoLabel, SIGNAL(pressed()), this, SLOT(autoLabel()));
}


LabelTab::~LabelTab() {
}


void LabelTab::update() {
}


QString LabelTab::leftLabel() const {
  return _leftLabel->text();
}


void LabelTab::setLeftLabel(const QString &label) {
  _leftLabel->setText(label);
}


QString LabelTab::bottomLabel() const {
  return _bottomLabel->text();
}


void LabelTab::setBottomLabel(const QString &label) {
  _bottomLabel->setText(label);
}


QString LabelTab::rightLabel() const {
  return _rightLabel->text();
}


void LabelTab::setRightLabel(const QString &label) {
  _rightLabel->setText(label);
}


QString LabelTab::topLabel() const {
  return _topLabel->text();
}


void LabelTab::setTopLabel(const QString &label) {
  _topLabel->setText(label);
}


void LabelTab::setGlobalFont(const QFont &font) {
  _globalLabelFontFamily->setCurrentFont(font);
  _globalLabelFontSize->setValue(font.pointSize());
  _globalLabelBold->setChecked(font.bold());
  _globalLabelUnderline->setChecked(font.underline());
  _globalLabelItalic->setChecked(font.italic());
}


QFont LabelTab::leftLabelFont() const {
  QFont font(_leftFamily->currentFont());
  font.setPointSize(_leftFontSize->value());
  font.setItalic(_leftItalic->isChecked());
  font.setBold(_leftBold->isChecked());
  font.setUnderline(_leftUnderline->isChecked());
  return font;
}


void LabelTab::setLeftLabelFont(const QFont &font) {
  _leftFamily->setCurrentFont(font);
  _leftFontSize->setValue(font.pointSize());
  _leftBold->setChecked(font.bold());
  _leftUnderline->setChecked(font.underline());
  _leftItalic->setChecked(font.italic());
}


QFont LabelTab::rightLabelFont() const {
  QFont font(_rightFamily->currentFont());
  font.setPointSize(_rightFontSize->value());
  font.setItalic(_rightItalic->isChecked());
  font.setBold(_rightBold->isChecked());
  font.setUnderline(_rightUnderline->isChecked());
  return font;
}


void LabelTab::setRightLabelFont(const QFont &font) {
  _rightFamily->setCurrentFont(font);
  _rightFontSize->setValue(font.pointSize());
  _rightBold->setChecked(font.bold());
  _rightUnderline->setChecked(font.underline());
  _rightItalic->setChecked(font.italic());
}


QFont LabelTab::topLabelFont() const {
  QFont font(_topFamily->currentFont());
  font.setPointSize(_topFontSize->value());
  font.setItalic(_topItalic->isChecked());
  font.setBold(_topBold->isChecked());
  font.setUnderline(_topUnderline->isChecked());
  return font;
}


void LabelTab::setTopLabelFont(const QFont &font) {
  _topFamily->setCurrentFont(font);
  _topFontSize->setValue(font.pointSize());
  _topBold->setChecked(font.bold());
  _topUnderline->setChecked(font.underline());
  _topItalic->setChecked(font.italic());
}


QFont LabelTab::bottomLabelFont() const {
  QFont font(_bottomFamily->currentFont());
  font.setPointSize(_bottomFontSize->value());
  font.setItalic(_bottomItalic->isChecked());
  font.setBold(_bottomBold->isChecked());
  font.setUnderline(_bottomUnderline->isChecked());
  return font;
}


void LabelTab::setBottomLabelFont(const QFont &font) {
  _bottomFamily->setCurrentFont(font);
  _bottomFontSize->setValue(font.pointSize());
  _bottomBold->setChecked(font.bold());
  _bottomUnderline->setChecked(font.underline());
  _bottomItalic->setChecked(font.italic());
}


void LabelTab::applyGlobals() {
  QFont font(_globalLabelFontFamily->currentFont());
  font.setPointSize(_globalLabelFontSize->value());
  font.setItalic(_globalLabelItalic->isChecked());
  font.setBold(_globalLabelBold->isChecked());
  font.setUnderline(_globalLabelUnderline->isChecked());

  setLeftLabelFont(font);
  setRightLabelFont(font);
  setTopLabelFont(font);
  setBottomLabelFont(font);
}


void LabelTab::autoLabel() {
  setLeftLabel(_plotItem->leftLabel());
  setBottomLabel(_plotItem->bottomLabel());
  setTopLabel(_plotItem->topLabel());
  setRightLabel(_plotItem->rightLabel());
  emit modified();
}


}

// vim: ts=2 sw=2 et