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
#include "kstcsddialog.h"
#include "kstcurvedialog.h"
#include "ksteqdialog.h"
#include "ksthsdialog.h"
#include "kstimagedialog.h"
#include "kstmatrixdialog.h"
#include "kstplugindialog.h"
#include "kstbasicdialog.h"
#include "kstpsddialog.h"
#include "kstvvdialog.h"
#include "kstvectordialog.h"

KstGuiDialogs::KstGuiDialogs()
: KstDialogs() {
}


KstGuiDialogs::~KstGuiDialogs() {
}


void KstGuiDialogs::showHistogramDialog(const QString& name, bool edit) {
  if (!edit) {
    KstHsDialog::globalInstance()->showNew(name);
  } else {
    KstHsDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showCPluginDialog(const QString& name, bool edit) {
  if (!edit) {
    KstPluginDialog::globalInstance()->showNew(name);
  } else {
    KstPluginDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showBasicPluginDialog(const QString& name, bool edit) {
  if (!edit) {
    KstBasicDialog::globalInstance()->showNew(name);
  } else {
    KstBasicDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showEquationDialog(const QString& name, bool edit) {
  if (!edit) {
    KstEqDialog::globalInstance()->showNew(name);
  } else {
    KstEqDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showCSDDialog(const QString& name, bool edit) {
  if (!edit) {
    KstCsdDialog::globalInstance()->showNew(name);
  } else {
    KstCsdDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showPSDDialog(const QString& name, bool edit) {
  if (!edit) {
    KstPsdDialog::globalInstance()->showNew(name);
  } else {
    KstPsdDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showVectorViewDialog(const QString& name, bool edit) {
  if (!edit) {
    KstVvDialog::globalInstance()->showNew(name);
  } else {
    KstVvDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::newVectorDialog(QWidget *parent, const char *createdSlot, const char *selectedSlot, const char *updateSlot) {
  KstVectorDialog *ad = new KstVectorDialog(parent, "vector dialog");
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


void KstGuiDialogs::showVectorDialog(const QString& name, bool edit) {
  if (!edit) {
    KstVectorDialog::globalInstance()->showNew(name);
  } else {
    KstVectorDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::newMatrixDialog(QWidget *parent, const char *createdSlot, const char *selectedSlot, const char *updateSlot) {
  KstMatrixDialog *ad = new KstMatrixDialog(parent, "matrix dialog");
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


void KstGuiDialogs::showMatrixDialog(const QString& name, bool edit) {
  if (!edit) {
    KstMatrixDialog::globalInstance()->showNew(name);
  } else {
    KstMatrixDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showImageDialog(const QString& name, bool edit) {
  if (!edit) {
    KstImageDialog::globalInstance()->showNew(name);
  } else {
    KstImageDialog::globalInstance()->showEdit(name);
  }
}


void KstGuiDialogs::showCurveDialog(const QString& name, bool edit) {
  if (!edit) {
    KstCurveDialog::globalInstance()->showNew(name);
  } else {
    KstCurveDialog::globalInstance()->showEdit(name);
  }
}
