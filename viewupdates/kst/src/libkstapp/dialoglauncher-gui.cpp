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

#include "dialoglauncher-gui.h"
#include "kstcsddialog_i.h"
#include "kstcurvedialog_i.h"
#include "ksteqdialog_i.h"
#include "ksthsdialog_i.h"
#include "kstimagedialog_i.h"
#include "kstmatrixdialog_i.h"
#include "kstplugindialog_i.h"
#include "kstpsddialog_i.h"
#include "kstvectordialog_i.h"


KstGuiDialogs::KstGuiDialogs()
: KstDialogs() {
}


KstGuiDialogs::~KstGuiDialogs() {
}


void KstGuiDialogs::showHistogramDialog(const QString& name) {
  KstHsDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showPluginDialog(const QString& name) {
  KstPluginDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showEquationDialog(const QString& name) {
  KstEqDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showCSDDialog(const QString& name) {
  KstCsdDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showPSDDialog(const QString& name) {
  KstPsdDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::newVectorDialog(QWidget *parent, const char *createdSlot, const char *selectedSlot, const char *updateSlot) {
  KstVectorDialogI *ad = new KstVectorDialogI(parent, "vector dialog");
  if (createdSlot) {
    QObject::connect(ad, SIGNAL(vectorCreated(KstVectorPtr)), parent, createdSlot);
  }
  if (selectedSlot) {
    QObject::connect(ad, SIGNAL(vectorCreated(KstVectorPtr)), parent, selectedSlot);
  }
  if (updateSlot) {
    QObject::connect(ad, SIGNAL(modified()), parent, updateSlot);
  }
  ad->show();
  ad->exec();
  delete ad;
}


void KstGuiDialogs::showVectorDialog(const QString& name) {
  KstVectorDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::newMatrixDialog(QWidget *parent, const char *createdSlot, const char *selectedSlot, const char *updateSlot) {
  KstMatrixDialogI *ad = new KstMatrixDialogI(parent, "matrix dialog");
  if (createdSlot) {
    QObject::connect(ad, SIGNAL(matrixCreated(KstMatrixPtr)), parent, createdSlot);
  }
  if (selectedSlot) {
    QObject::connect(ad, SIGNAL(matrixCreated(KstMatrixPtr)), parent, selectedSlot);
  }
  if (updateSlot) {
    QObject::connect(ad, SIGNAL(modified()), parent, updateSlot);
  }
  ad->show();
  ad->exec();
  delete ad;
}


void KstGuiDialogs::showMatrixDialog(const QString& name) {
  KstMatrixDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showImageDialog(const QString& name) {
  KstImageDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showCurveDialog(const QString& name) {
  KstCurveDialogI::globalInstance()->showEdit(name);
}

// vim: ts=2 sw=2 et
