/***************************************************************************
                    kstchangenptsdialog_i.cpp  -  Part of KST
                             -------------------
    begin                :
    copyright            : (C) 2003 The University of Toronto
                           (C) 2003 C. Barth Netterfield
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
#include <qwidget.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>

#include "kstrvector.h"
#include "kstdatacollection.h"
#include "kstchangenptsdialog_i.h"

KstChangeNptsDialogI::KstChangeNptsDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl) :
  KstChangeNptsDialog(parent, name, modal, fl) {


    connect(Clear,     SIGNAL(clicked()),
            CurveList, SLOT(clearSelection()));
    connect(SelectAll, SIGNAL(clicked()),
            this,      SLOT(selectAll()));
    connect(Apply,     SIGNAL(clicked()),
            this,      SLOT(applyNptsChange()));
    connect(CurveList, SIGNAL(selected ( int )),
            this,      SLOT(updateDefaults( int )));
}

KstChangeNptsDialogI::~KstChangeNptsDialogI() {
}

void KstChangeNptsDialogI::selectAll() {
  CurveList->selectAll(true);
}

void KstChangeNptsDialogI::updateChangeNptsDialog() {
  unsigned int i;

  CurveList->clear();

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  /* insert vectors into ChangeNptsCurveList */
  for (i = 0; i < rvl.count(); i++) {
    CurveList->insertItem(rvl[i]->tagName(), -1);
  }
}

void KstChangeNptsDialogI::showChangeNptsDialog() {
  updateChangeNptsDialog();
  updateDefaults(0);
  CurveList->selectAll(true);

  show();
  raise();
}

void KstChangeNptsDialogI::applyNptsChange() {
  unsigned int i_vector;
  KstRVectorPtr vector;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  for (i_vector = 0; i_vector < CurveList->count(); i_vector++) {
    if (CurveList->isSelected(i_vector)) {
      vector = rvl[i_vector];

      vector->changeFrames(
        (CountFromEnd->isChecked() ?  -1 : F0->value()),
        (ReadToEnd->isChecked() ? -1 : N->value()),
        Skip->value(),
        DoSkip->isChecked(),
        DoFilter->isChecked());
    }
  }

  emit docChanged();
}

void KstChangeNptsDialogI::updateDefaults(int index) {
  KstRVectorPtr vector;

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  if (rvl.isEmpty()) {
    return;
  }

  if (index > (int)rvl.count()) {
    return;
  }

  vector = rvl[index];

    /* fill the vector range entries */
  if (vector->countFromEOF()) {
    CountFromEnd->setChecked(true);
  } else {
    CountFromEnd->setChecked(false);
  }
  F0->setValue(vector->reqStartFrame());

  /* fill number of frames entries */
  if (vector->readToEOF()) {
    ReadToEnd->setChecked(true);
  } else {
    ReadToEnd->setChecked(false);
  }
  N->setValue(vector->reqNumFrames());

  /* fill in frames to skip box */
  Skip->setValue(vector->skip());
  DoSkip->setChecked(vector->doSkip());
  DoFilter->setChecked(vector->doAve());

}

#include "kstchangenptsdialog_i.moc"
