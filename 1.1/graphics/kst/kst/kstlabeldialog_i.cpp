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

// include files for Qt
#include <qcheckbox.h>
#include <qradiobutton.h>

// include files for KDE
#include <kcolorbutton.h>
#include <kfontcombo.h>
#include <klineedit.h>
#include <knuminput.h>

// application specific includes
#include "kst.h"
#include "kstlabeldialog_i.h"
#include "kstviewwindow.h"
#include "scalarselector.h"

KstLabelDialogI::KstLabelDialogI(QWidget *parent, const char* name, bool modal,
                                 WFlags fl)
: KstLabelDialog(parent, name, modal, fl) {
  _x = _y = 0.0;
  _i_plot = 0;

  FontComboBox->setCurrentFont(this->font().family());
  FontComboBox->setEditable(false);
  connect(Apply, SIGNAL(clicked()), this, SLOT(apply()));
  connect(OK, SIGNAL(clicked()), this, SLOT(ok()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(deleteL()));
  connect(checkBoxPlotColor, SIGNAL(clicked()), this, SLOT(usePlotColorChange()));
  setFixedHeight(height());  
}


KstLabelDialogI::~KstLabelDialogI() {
}


void KstLabelDialogI::showI(Kst2DPlotPtr plot, int i_label, double in_x, double in_y) {
  _x = in_x;
  _y = in_y;
  _i_plot = plot;
  if (i_label < 0) {
    _editing = false;
    _i_label = 0;
    LabelText->setText(QString::null);
    checkBoxPlotColor->setChecked(true);
    kColorButton->setColor(QColor(0,0,0));
    kColorButton->setEnabled(false);
  } else if (plot->_labelList.count() > (unsigned)i_label) {
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

  if (_editing && _i_plot) {
    if ((int)_i_plot->_labelList.count() <= _i_label) {
      _editing = false;
      _i_label = 0;
    } else {
      KstLabelPtr label = _i_plot->_labelList[_i_label];
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
          break;
      }
      FontComboBox->setCurrentFont(label->fontName());
      Rotation->setValue((int)label->rotation());
      checkBoxPlotColor->setChecked(label->usePlotColor());
      kColorButton->setEnabled(!label->usePlotColor());
      kColorButton->setColor(label->color());
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

  KMdiChildView *c = KstApp::inst()->activeWindow();
  if (c) {
    static_cast<KstViewWindow*>(c)->view()->paint(P_PLOT);
  }

  emit applied();
}


void KstLabelDialogI::applyAsNew() {
  KstJustifyType J;

  if (LeftJustify->isChecked()) {
    J = LxBy;
  } else if (CenterJustify->isChecked()) {
    J = CxBy;
  } else {
    J = RxBy;
  }

  KstLabelPtr new_label = new KstLabel(LabelText->text(), J,
                           (float) Rotation->value(), _x, _y, false);
  new_label->setFontName(FontComboBox->currentText());
  new_label->setSize(FontSize->value());
  new_label->setUsePlotColor(checkBoxPlotColor->isChecked());
  new_label->setColor(kColorButton->color());

  _i_plot->_labelList.append(new_label);
  _i_label = _i_plot->_labelList.count() - 1;
  _editing = true;
  _i_plot->setDirty();
}


void KstLabelDialogI::applyEdits() {
  if (_i_plot) {
    if ((int)_i_plot->_labelList.count() <= _i_label) {
    // some surprising badness...  shouldn't happen - so just bail
      _editing = false;
      _i_label = 0;
      return;
    } else {
      KstLabelPtr label = _i_plot->_labelList[_i_label];
      KstJustifyType J;

      if (LeftJustify->isChecked()) {
        J = LxBy;
      } else if (CenterJustify->isChecked()) {
        J = CxBy;
      } else {
        J = RxBy;
      }

      label->setText(LabelText->text());
      label->setJustification(J);
      label->setSize(FontSize->value());
      label->setFontName(FontComboBox->currentText());
      label->setRotation((float)Rotation->value());
      label->setUsePlotColor(checkBoxPlotColor->isChecked());
      label->setColor(kColorButton->color());
    }
    _i_plot->setDirty();
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

  if (_i_plot) {
    if ((int)_i_plot->_labelList.count() <= _i_label) {
      // some surprising badness...  shouldn't happen - so just bail
      _editing = false;
      _i_label = 0;
      close();
      return;
    } else {
      _i_plot->_labelList.remove(_i_plot->_labelList[_i_label]);
      emit applied();
    }
    _i_plot->setDirty();
  }
  _editing = false;
  close();
}


void KstLabelDialogI::usePlotColorChange() {
    kColorButton->setEnabled(!checkBoxPlotColor->isChecked());  
}
#include "kstlabeldialog_i.moc"
// vim: ts=2 sw=2 et
