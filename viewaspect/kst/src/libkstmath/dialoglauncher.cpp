/***************************************************************************
                              dialoglauncher.cpp
                             -------------------
    begin                : Nov. 24, 2004
    copyright            : (C) 2004 The University of Toronto
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

#include "dialoglauncher.h"

static KStaticDeleter<KstDialogs> sdDialogs;
KstDialogs *KstDialogs::_self = 0L;

KstDialogs *KstDialogs::self() {
  if (!_self) {
    _self = sdDialogs.setObject(_self, new KstDialogs);
  }
  return _self;
}


void KstDialogs::replaceSelf(KstDialogs *newInstance) {
  delete _self;
  _self = 0L;
  _self = sdDialogs.setObject(_self, newInstance);
}


KstDialogs::KstDialogs() {
}


KstDialogs::~KstDialogs() {
}


void KstDialogs::showHistogramDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::showPluginDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::showEquationDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::showCSDDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::showPSDDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::newVectorDialog(QWidget *parent, const char *createdSlot, const char *selectedSlot, const char *updateSlot) {
  Q_UNUSED(parent)
  Q_UNUSED(createdSlot)
  Q_UNUSED(selectedSlot)
  Q_UNUSED(updateSlot)
}


void KstDialogs::showVectorDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::newMatrixDialog(QWidget *parent, const char *createdSlot, const char *selectedSlot, const char *updateSlot) {
  Q_UNUSED(parent)
  Q_UNUSED(createdSlot)
  Q_UNUSED(selectedSlot)
  Q_UNUSED(updateSlot)
}


void KstDialogs::showMatrixDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::showImageDialog(const QString& name) {
  Q_UNUSED(name)
}


void KstDialogs::showCurveDialog(const QString& name) {
  Q_UNUSED(name)
}


// vim: ts=2 sw=2 et
