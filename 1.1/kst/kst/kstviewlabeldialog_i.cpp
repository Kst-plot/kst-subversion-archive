/**************************************************************************
        kstviewlabeldialog_i.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2004
    copyright            : (C) 2004 by The University of British Columbia
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

#include <qpushbutton.h>
#include <qradiobutton.h>

#include <kfontcombo.h>
#include <klineedit.h>
#include <knuminput.h>

#include "kstviewlabeldialog_i.h"
#include "scalarselector.h"

KstViewLabelDialogI::KstViewLabelDialogI(QWidget *parent, const char* name, bool modal,
                                 WFlags fl)
: KstViewLabelDialog(parent, name, modal, fl) {
  FontComboBox->setCurrentFont(this->font().family());

  _pViewLabel = NULL;

  connect(Apply, SIGNAL(clicked()), this, SLOT(apply()));
  connect(OK, SIGNAL(clicked()), this, SLOT(ok()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(deleteL()));
}

KstViewLabelDialogI::~KstViewLabelDialogI() {
}

void KstViewLabelDialogI::showI(KstViewLabel* pViewLabel) {
  _pViewLabel = pViewLabel;

  updateI();
  show();
}

void KstViewLabelDialogI::updateI() {
  KstVLJustifyType  justify;

  _scalars->update();

  if( _pViewLabel) {
    justify = _pViewLabel->justification();
    LabelText->setText(_pViewLabel->text());

    switch (KST_JUSTIFY_H(justify)) {
      case KST_JUSTIFY_H_RIGHT:
        RightJustify->setChecked( true );
        break;
      case KST_JUSTIFY_H_CENTER:
        CenterJustify->setChecked( true );
        break;
      case KST_JUSTIFY_H_NONE:
      case KST_JUSTIFY_H_LEFT:
      default:
        LeftJustify->setChecked( true );
        break;
    }

    switch (KST_JUSTIFY_V(justify)) {
      case KST_JUSTIFY_V_BOTTOM:
        BottomJustify->setChecked( true );
        break;
      case KST_JUSTIFY_V_CENTER:
        MiddleJustify->setChecked( true );
        break;
      case KST_JUSTIFY_V_NONE:
      case KST_JUSTIFY_V_TOP:
      default:
        TopJustify->setChecked( true );
        break;
    }

    FontComboBox->setCurrentFont(_pViewLabel->fontName());
    Rotation->setValue((int)_pViewLabel->rotation());
  } else {
    _editing = false;
  }
}

void KstViewLabelDialogI::apply() {
  if (_editing) {
    applyEdits();
  } else{
    applyAsNew();
  }

  emit applied();
}

void KstViewLabelDialogI::applyAsNew() {
  KstVLJustifyType vjustify;
  KstVLJustifyType hjustify;

  if (LeftJustify->isChecked()) {
    hjustify = KST_JUSTIFY_H_LEFT;
  } else if (CenterJustify->isChecked()) {
    hjustify = KST_JUSTIFY_H_CENTER;
  } else {
    hjustify = KST_JUSTIFY_H_RIGHT;
  }

  if (TopJustify->isChecked()) {
    vjustify = KST_JUSTIFY_V_TOP;
  } else if (MiddleJustify->isChecked()) {
    vjustify = KST_JUSTIFY_V_CENTER;
  } else {
    vjustify = KST_JUSTIFY_V_BOTTOM;
  }

  _pViewLabel->setFontName(FontComboBox->currentText());
  _pViewLabel->setJustification(SET_KST_JUSTIFY(hjustify,vjustify));

  _editing = true;
}

void KstViewLabelDialogI::applyEdits() {
  KstVLJustifyType vjustify;
  KstVLJustifyType hjustify;

  if (LeftJustify->isChecked()) {
    hjustify = KST_JUSTIFY_H_LEFT;
  } else if (CenterJustify->isChecked()) {
    hjustify = KST_JUSTIFY_H_CENTER;
  } else {
    hjustify = KST_JUSTIFY_H_RIGHT;
  }

  if (TopJustify->isChecked()) {
    vjustify = KST_JUSTIFY_V_TOP;
  } else if (MiddleJustify->isChecked()) {
    vjustify = KST_JUSTIFY_V_CENTER;
  } else {
    vjustify = KST_JUSTIFY_V_BOTTOM;
  }

  if( _pViewLabel ) {
    _pViewLabel->setText(LabelText->text());
    _pViewLabel->setJustification(SET_KST_JUSTIFY(hjustify,vjustify));
    _pViewLabel->setFontName(FontComboBox->currentText());
    _pViewLabel->setRotation((float)Rotation->value());
    _pViewLabel->update();
  }
}

void KstViewLabelDialogI::ok() {
  apply();
  close();
}

void KstViewLabelDialogI::deleteL() {
  close();  
}


#include "kstviewlabeldialog_i.moc"
// vim: ts=2 sw=2 et
