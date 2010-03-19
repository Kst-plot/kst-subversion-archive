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

#include <kconfig.h>
#include <kconfiggroup.h>

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


void KstLegendDefaults::writeConfig(KConfig *config) {
  KConfigGroup cfg(config, "LegendDefaults");

  cfg.writeEntry("LegendFontSize", KST::legendDefaults.fontSize());
  cfg.writeEntry("LegendFontColor", KST::legendDefaults.fontColor());
  cfg.writeEntry("LegendFont", KST::legendDefaults.font());
  cfg.writeEntry("LegendTransparent", KST::legendDefaults.transparent());
  cfg.writeEntry("LegendForegroundColor", KST::legendDefaults.foregroundColor());
  cfg.writeEntry("LegendBackgroundColor", KST::legendDefaults.backgroundColor());
  cfg.writeEntry("LegendVertical", KST::legendDefaults.vertical());
  cfg.writeEntry("LegendTrackContents", KST::legendDefaults.trackContents());
  cfg.writeEntry("LegendBorder", KST::legendDefaults.border());
  cfg.writeEntry("LegendMargin", KST::legendDefaults.margin());
  cfg.writeEntry("LegendScaleLineWidth", KST::legendDefaults.scaleLineWidth());
}


void KstLegendDefaults::readConfig(KConfig *config) {
  KConfigGroup cfg(config, "LegendDefaults");
  QColor color;
/* xxx
  _fontSize = cfg.readNumEntry("LegendFontSize", 12);
  color = KstSettings::globalSettings()->foregroundColor;
  _fontColor = cfg.readColorEntry("LegendFontColor", &color);
  _font = cfg.readEntry("LegendFont", KstApp::inst()->defaultFont());
  _transparent = cfg.readBoolEntry("LegendTransparent", false);
  color = KstSettings::globalSettings()->foregroundColor;
  _foregroundColor = cfg.readColorEntry("LegendForegroundColor", &color);
  color = KstSettings::globalSettings()->backgroundColor;
  _backgroundColor = cfg.readColorEntry("LegendBackgroundColor", &color);
  _vertical = cfg.readBoolEntry("LegendVertical", true);
  _trackContents = cfg.readBoolEntry("LegendTrackContents", true);
  _border = cfg.readNumEntry("LegendBorder", 2);
  _margin = cfg.readNumEntry("LegendMargin", 5);
  _scaleLineWidth = cfg.readNumEntry("LegendScaleLineWidth", 1);
*/
}
