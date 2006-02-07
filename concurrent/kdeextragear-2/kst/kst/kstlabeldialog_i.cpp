/**************************************************************************
        kstlabeldialog_i.h - source file: inherits designer dialog
                             -------------------
    begin                :  2003
    copyright            : (C) 2003 by Barth Netterfield
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
#include <qfontdatabase.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <klineedit.h>
#include <kfontcombo.h>
#include <knuminput.h>

#include "kstlabeldialog_i.h"
#include "kstdatacollection.h"
#include "kstplotlist.h"
#include "scalarselector.h"

KstLabelDialogI::KstLabelDialogI(QWidget *parent,
                                 const char* name,
                                 bool modal,
                                 WFlags fl) :
  KstLabelDialog(parent, name, modal, fl) {
    _x = _y = 0.0;
    _i_plot = 0;

    FontComboBox->setCurrentFont(this->font().family());

    connect(Apply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(OK, SIGNAL(clicked()),
            this, SLOT(ok()));
    connect(Delete, SIGNAL(clicked()),
            this, SLOT(deleteL()));
}

KstLabelDialogI::~KstLabelDialogI() {
}

void KstLabelDialogI::showI(int i_plot, int i_label,
                            double in_x, double in_y) {
  _x = in_x;
  _y = in_y;
  _i_plot = i_plot;
  if (i_label < 0) {
    _editing = false;
    _i_label = 0;
    LabelText->setText(QString::null);
  } else if ((int)KST::plotList.at(i_plot)->labelList.count() > i_label) {
    _editing = true;
    _i_label = i_label;
  } else {
    _editing = false;
    _i_label = 0;
  }

  updateI();
  show();
}

void KstLabelDialogI::updateI() {
  _scalars->update();
  if (_i_plot >= (int)KST::plotList.count())
    _i_plot = (int)KST::plotList.count() - 1;

  if (_editing && _i_plot >= 0) {
    KstPlot *plot = KST::plotList.at(_i_plot);

    if ((int)plot->labelList.count()<=_i_label) {
      _editing = false;
      _i_label = 0;
    } else {
      KstLabel *label = plot->labelList.at(_i_label);
      LabelText->setText(label->text());
      FontSize->setValue(label->size());
      switch (label->justification()) {
          case LxBy:
            LeftJustify->setChecked(true);
            break;
          case CxBy:
            CenterJustify->setChecked(true);
            break;
          case RxBy:
            RightJustify->setChecked(true);
            break;
          default:
            LeftJustify->setChecked(true);
      }
      FontComboBox->setCurrentFont(label->fontName());
      Rotation->setValue((int)label->rotation());
    }
  } else {
    _editing = false;
  }
}

void KstLabelDialogI::apply() {
  //updateI();

  if (_editing) {
    applyEdits();
  } else{
    applyAsNew();
  }

  emit applied();
}

void KstLabelDialogI::applyAsNew() {

  if (_i_plot >= (int)KST::plotList.count()) {
    // some surprising badness...  shouldn't happen - so just bail
    _i_plot = (int)KST::plotList.count()-1;
    _editing = false;
    return;
  }

  KstLabel *new_label;
  KstJustifyType J;

  if (LeftJustify->isChecked()) {
    J = LxBy;
  } else if (CenterJustify->isChecked()) {
    J = CxBy;
  } else {
    J = RxBy;
  }

  new_label = new KstLabel(LabelText->text(), J,
                           (float) Rotation->value(), _x, _y, false);
  new_label->setFontName(FontComboBox->currentText());
  new_label->setSize(FontSize->value()); // FIXME: spin box name is wrong

  KST::plotList.at(_i_plot)->labelList.append(new_label);
  _i_label = KST::plotList.at(_i_plot)->labelList.count()-1;
  _editing = true;
}

void KstLabelDialogI::applyEdits() {

  if (_i_plot >= (int)KST::plotList.count()) {
    // some surprising badness...  shouldn't happen - so just bail
    _i_plot = (int)KST::plotList.count()-1;
    _editing = false;
    return;
  }

  if (_i_plot>=0) {
    KstPlot *plot = KST::plotList.at(_i_plot);

    if ((int)plot->labelList.count()<=_i_label) {
    // some surprising badness...  shouldn't happen - so just bail
      _editing = false;
      _i_label = 0;
      return;
    } else {

      KstLabel *label = plot->labelList.at(_i_label);
      KstJustifyType J;

      if (LeftJustify->isChecked()) J = LxBy;
      else if (CenterJustify->isChecked()) J = CxBy;
      else J = RxBy;

      label->setText(LabelText->text());
      label->setJustification(J);
      label->setSize(FontSize->value());
      label->setFontName(FontComboBox->currentText());
      label->setRotation((float)Rotation->value());
    }
  }
}

void KstLabelDialogI::ok() {
  apply();
  close();
}

void KstLabelDialogI::deleteL() {
  if (!_editing) {
    close();
    return;
  }

  if (_i_plot >= (int)KST::plotList.count()) {
    // some surprising badness...  shouldn't happen - so just bail
    _i_plot = (int)KST::plotList.count()-1;
    _editing = false;
    close();
    return;
  }
  if (_i_plot>=0) {
    KstPlot *plot = KST::plotList.at(_i_plot);

    if ((int)plot->labelList.count()<=_i_label) {
    // some surprising badness...  shouldn't happen - so just bail
      _editing = false;
      _i_label = 0;
      close();
      return;
    } else {
      plot->labelList.remove(_i_label);
      emit applied();
    }
  }
  _editing = false;
  close();
}


#include "kstlabeldialog_i.moc"
