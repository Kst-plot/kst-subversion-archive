/***************************************************************************
                            dialoglauncher-gui.cpp
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
#include "ksteqdialog_i.h"
#include "ksthsdialog_i.h"
#include "kstimagedialog_i.h"
#include "kstmatrixdialog_i.h"
#include "kstplugindialog_i.h"
#include "kstpsddialog_i.h"
#include "kstcurvedialog_i.h"

namespace KstDialogs {
  void showHistogramDialog(const QString& name) {
    KstHsDialogI::globalInstance()->show_Edit(name);
  }


  void showPluginDialog(const QString& name) {
    KstPluginDialogI::globalInstance()->show_Edit(name);
  }


  void showEquationDialog(const QString& name) {
    KstEqDialogI::globalInstance()->show_Edit(name);
  }


  void showPSDDialog(const QString& name) {
    KstPsdDialogI::globalInstance()->show_Edit(name);
  }


  void showMatrixDialog(const QString& name) {
    KstMatrixDialogI::globalInstance()->show_Edit(name);
  }


  void showImageDialog(const QString& name) {
    KstImageDialogI::globalInstance()->show_Edit(name);
  }


  void showCurveDialog(const QString& name) {
    KstCurveDialogI::globalInstance()->show_Edit(name);
  }


}

// vim: ts=2 sw=2 et
