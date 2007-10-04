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

#include "dialoglaunchergui.h"

#include "application.h"
#include "curvedialog.h"
#include "equationdialog.h"
#include "vectordialog.h"

namespace Kst {

DialogLauncherGui::DialogLauncherGui() {
}


DialogLauncherGui::~DialogLauncherGui() {
}


void DialogLauncherGui::showVectorDialog(Kst::ObjectPtr objectPtr) {
  VectorDialog dialog(objectPtr, kstApp->mainWindow());
  dialog.exec();
}


void DialogLauncherGui::showMatrixDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showScalarDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showStringDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showCurveDialog(Kst::ObjectPtr objectPtr) {
  CurveDialog dialog(objectPtr, kstApp->mainWindow());
  dialog.exec();
}


void DialogLauncherGui::showImageDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showEquationDialog(Kst::ObjectPtr objectPtr) {
  EquationDialog(objectPtr, kstApp->mainWindow()).exec();
}


void DialogLauncherGui::showHistogramDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showPSDDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showCSDDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}


void DialogLauncherGui::showBasicPluginDialog(Kst::ObjectPtr objectPtr) {
  Q_UNUSED(objectPtr);
}

}

// vim: ts=2 sw=2 et
