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

#include <QApplication>

namespace Kst {

DialogLauncher *DialogLauncher::_self = 0L;
void DialogLauncher::cleanup() {
    delete _self;
    _self = 0;
}


DialogLauncher *DialogLauncher::self() {
  if (!_self) {
    _self = new DialogLauncher;
    qAddPostRoutine(DialogLauncher::cleanup);
  }
  return _self;
}


void DialogLauncher::replaceSelf(DialogLauncher *newInstance) {
  delete _self;
  _self = 0L;
  _self = newInstance;
}


DialogLauncher::DialogLauncher() {
}


DialogLauncher::~DialogLauncher() {
}


void DialogLauncher::showVectorDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showMatrixDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showScalarDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showStringDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showCurveDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showImageDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showEquationDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showEventMonitorDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showHistogramDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showPowerSpectrumDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showCSDDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncher::showBasicPluginDialog(ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}

}

// vim: ts=2 sw=2 et
