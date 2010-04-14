/***************************************************************************
                            kstlegenddefaults.cpp
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 The University of British Columbia
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

#include "kst.h"
#include "kstlegenddefaults.h"
#include "kstsettings.h"
#include "stdinsource.h"

KstLegendDefaults KST::legendDefaults;

KstLegendDefaults::KstLegendDefaults() {
  _fontSize = 12;
  _fontColor = QColor("black");
  _transparent = false;
  _foregroundColor = QColor("black");
  _backgroundColor = QColor("white");
  _vertical = true;
  _trackContents = true;
  _border = 2;
  _margin = 5;
  _scaleLineWidth = 1;
}


const QColor& KstLegendDefaults::fontColor() const {
  return _fontColor;
}


const QColor& KstLegendDefaults::foregroundColor() const {
  return _foregroundColor;
}


const QColor& KstLegendDefaults::backgroundColor() const {
  return _backgroundColor;
}


const QString& KstLegendDefaults::font() const {
  return _font;
}


int KstLegendDefaults::fontSize() const {
  return _fontSize;
}


bool KstLegendDefaults::vertical() const {
  return _vertical;
}


bool KstLegendDefaults::transparent() const {
  return _transparent;
}


bool KstLegendDefaults::trackContents() const {
  return _trackContents;
}


int KstLegendDefaults::border() const {
  return _border;
}


int KstLegendDefaults::margin() const {
  return _margin;
}


int KstLegendDefaults::scaleLineWidth() const {
  return _scaleLineWidth;
}


void KstLegendDefaults::setFontColor(const QColor& color) {
  _fontColor = color;
}


void KstLegendDefaults::setForegroundColor(const QColor& color) {
  _foregroundColor = color;
}


void KstLegendDefaults::setBackgroundColor(const QColor& color) {
  _backgroundColor = color;
}


void KstLegendDefaults::setFont(const QString& font) {
  _font = font;
}


void KstLegendDefaults::setFontSize(int size) {
  _fontSize = size;
}


void KstLegendDefaults::setVertical(bool vertical) {
  _vertical = vertical;
}


void KstLegendDefaults::setTransparent(bool transparent) {
  _transparent = transparent;
}


void KstLegendDefaults::setTrackContents(bool trackContents) {
  _trackContents = trackContents;
}


void KstLegendDefaults::setBorder(int border) {
  _border = border;
}


void KstLegendDefaults::setMargin(int margin) {
  _margin = margin;
}


void KstLegendDefaults::setScaleLineWidth(int scaleLineWidth) {
  _scaleLineWidth = scaleLineWidth;
}


void KstLegendDefaults::writeConfig(QSettings *cfg) {
  cfg->beginGroup("LegendDefaults");

  cfg->setValue("LegendFontSize", KST::legendDefaults.fontSize());
  cfg->setValue("LegendFontColor", KST::legendDefaults.fontColor());
  cfg->setValue("LegendFont", KST::legendDefaults.font());
  cfg->setValue("LegendTransparent", KST::legendDefaults.transparent());
  cfg->setValue("LegendForegroundColor", KST::legendDefaults.foregroundColor());
  cfg->setValue("LegendBackgroundColor", KST::legendDefaults.backgroundColor());
  cfg->setValue("LegendVertical", KST::legendDefaults.vertical());
  cfg->setValue("LegendTrackContents", KST::legendDefaults.trackContents());
  cfg->setValue("LegendBorder", KST::legendDefaults.border());
  cfg->setValue("LegendMargin", KST::legendDefaults.margin());
  cfg->setValue("LegendScaleLineWidth", KST::legendDefaults.scaleLineWidth());

  cfg->endGroup();
}


void KstLegendDefaults::readConfig(QSettings *cfg) {
  cfg->beginGroup("LegendDefaults");
  QColor color;
  _fontSize = cfg->value("LegendFontSize", 12).toInt();
  color = KstSettings::globalSettings()->foregroundColor;
  _fontColor = (cfg->value("LegendFontColor", color)).value<QColor>();
  _font = cfg->value("LegendFont", KstApp::inst()->defaultFont()).toString();
  _transparent = cfg->value("LegendTransparent", false).toBool();
  color = KstSettings::globalSettings()->foregroundColor;
  _foregroundColor = cfg->value("LegendForegroundColor", color).QVariant::value<QColor>();
  color = KstSettings::globalSettings()->backgroundColor;
  _backgroundColor = cfg->value("LegendBackgroundColor", color).QVariant::value<QColor>();
  _vertical = cfg->value("LegendVertical", true).toBool();
  _trackContents = cfg->value("LegendTrackContents", true).toBool();
  _border = cfg->value("LegendBorder", 2).toInt();
  _margin = cfg->value("LegendMargin", 5).toInt();
  _scaleLineWidth = cfg->value("LegendScaleLineWidth", 1).toInt();
  cfg->endGroup();
}
