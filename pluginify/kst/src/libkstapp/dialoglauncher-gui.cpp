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
#include "kstbasicdialog_i.h"
#include "kstpsddialog_i.h"
#include "kstvectordialog_i.h"


KstGuiDialogs::KstGuiDialogs()
: KstDialogs() {
}


KstGuiDialogs::~KstGuiDialogs() {
}


void KstGuiDialogs::showHistogramDialog(const QString& name) {
  if ( name.isEmpty() )
    KstHsDialogI::globalInstance()->show();
  else
    KstHsDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showCPluginDialog(const QString& name) {
  if ( name.isEmpty() )
    KstPluginDialogI::globalInstance()->show();
  else
    KstPluginDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showBasicPluginDialog(const QString& name) {
  if ( name.isEmpty() )
    KstBasicDialogI::globalInstance()->show();
  else
    KstBasicDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showEquationDialog(const QString& name) {
  if ( name.isEmpty() )
    KstEqDialogI::globalInstance()->show();
  else
    KstEqDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showCSDDialog(const QString& name) {
  if ( name.isEmpty() )
    KstCsdDialogI::globalInstance()->show();
  else
    KstCsdDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showPSDDialog(const QString& name) {
  if ( name.isEmpty() )
    KstPsdDialogI::globalInstance()->show();
  else
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
  if ( name.isEmpty() )
    KstVectorDialogI::globalInstance()->show();
  else
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
  if ( name.isEmpty() )
    KstMatrixDialogI::globalInstance()->show();
  else
    KstMatrixDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showImageDialog(const QString& name) {
  if ( name.isEmpty() )
    KstImageDialogI::globalInstance()->show();
  else
    KstImageDialogI::globalInstance()->showEdit(name);
}


void KstGuiDialogs::showCurveDialog(const QString& name) {
  if ( name.isEmpty() )
    KstCurveDialogI::globalInstance()->show();
  else
    KstCurveDialogI::globalInstance()->showEdit(name);
}

// vim: ts=2 sw=2 et
