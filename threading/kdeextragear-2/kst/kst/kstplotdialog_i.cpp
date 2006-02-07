/**************************************************************************
              kstplotdialog_i.cpp - plot dialog: inherits designer dialog
                             -------------------
    begin                :  2000
    copyright            : (C) 2000-2002 by Barth Netterfield
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
#include <qdir.h>
#include <qfileinfo.h>
#include <qwidget.h>
#include <qtextstream.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfontdatabase.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <klistbox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qtabwidget.h>

// include files for KDE
#include <klocale.h>
#include <kdualcolorbutton.h>
#include <kmessagebox.h>
#include <kfontcombo.h>
#include <kdebug.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

#include "kstdatacollection.h"
#include "kstplotdialog_i.h"

/*
 *  Constructs a KstPlotDialogI which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KstPlotDialogI::KstPlotDialogI(KstDoc *in_doc, QWidget* parent,
                               const char* name, bool modal, WFlags fl)
: KstPlotDialog(parent, name, modal, fl ) {

  doc = in_doc;
  setScalarDestTopLabel();

  connect(Select, SIGNAL(activated(int)), this, SLOT(update(int)));
  connect(TabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTab()));
  connect(New, SIGNAL(clicked()), this, SLOT(new_I()));
  connect(Edit, SIGNAL(clicked()), this, SLOT(edit_I()));
  connect(Delete, SIGNAL(clicked()), this, SLOT(delete_I()));

  connect(DisplayedCurveList, SIGNAL(selectionChanged(QListBoxItem*)),
          this, SLOT(updateButtons()));
  connect(AvailableCurveList, SIGNAL(selectionChanged(QListBoxItem*)),
          this, SLOT(updateButtons()));

  /* adding/removing curves */
  connect(CurveUnplot, SIGNAL(clicked()),
          this, SLOT(removeDisplayedCurve()));
  connect(DisplayedCurveList, SIGNAL(executed(QListBoxItem*)),
          this, SLOT(removeDisplayedCurve()));
  connect(CurvePlot, SIGNAL(clicked()),
          this, SLOT(addDisplayedCurve()));
  connect(AvailableCurveList, SIGNAL(executed(QListBoxItem*)),
          this, SLOT(addDisplayedCurve()));

  /* labels */
  connect(TopLabelText, SIGNAL(textChanged(const QString &)),
          this, SLOT(updateSampTopLabel()));
  connect(XAxisText, SIGNAL(textChanged(const QString &)),
          this, SLOT(updateSampXLabel()));
  connect(YAxisText, SIGNAL(textChanged(const QString &)),
          this, SLOT(updateSampYLabel()));
  connect(TopLabelFontSize, SIGNAL(valueChanged(int)),
          this, SLOT(updateSampTopLabel()));
  connect(XLabelFontSize, SIGNAL(valueChanged(int)),
          this, SLOT(updateSampXLabel()));
  connect(YLabelFontSize, SIGNAL(valueChanged(int)),
          this, SLOT(updateSampYLabel()));
  connect(NumberFontSize, SIGNAL(valueChanged(int)),
          this, SLOT(updateSampNumLabel()));
  connect(FontComboBox, SIGNAL(activated(int)),
          this, SLOT(updateLabels()));
  connect(FontComboBox, SIGNAL(highlighted(int)),
          this, SLOT(updateLabels()));
  connect(FontComboBox, SIGNAL(textChanged(const QString &)),
          this, SLOT(updateLabels()));
  connect(AutoLabel, SIGNAL(clicked()),
          this, SLOT(applyAutoLabels()));


  connect(ScalarList, SIGNAL(activated(int)),
          this, SLOT(insertCurrentScalar()));

  connect(YAxisText, SIGNAL(selectionChanged()),
          this, SLOT(setScalarDestYLabel()));
  connect(YAxisText, SIGNAL(textChanged(const QString &)),
          this, SLOT(setScalarDestYLabel()));
  connect(XAxisText, SIGNAL(selectionChanged()),
          this, SLOT(setScalarDestXLabel()));
  connect(XAxisText, SIGNAL(textChanged(const QString &)),
          this, SLOT(setScalarDestXLabel()));
  connect(TopLabelText, SIGNAL(selectionChanged()),
          this, SLOT(setScalarDestTopLabel()));
  connect(TopLabelText, SIGNAL(textChanged(const QString &)),
          this, SLOT(setScalarDestTopLabel()));

  SampleLabel = new KstLabel("\0", CxCy, 0.0, 0.0, 0.0, true);
  QFontDatabase qfd;
  FontComboBox->setFonts(qfd.families());
}

KstPlotDialogI::~KstPlotDialogI() {
  delete SampleLabel;
  SampleLabel = 0L;
}

void KstPlotDialogI::show_I() {
  show();
  raise();
  update();
}

void KstPlotDialogI::newTab( ) {
    updateLabels();
}

void KstPlotDialogI::update(int new_index) {
  int index;
  int i_plot;
  KstPlot *plot;
  KstBaseCurvePtr curve;
  double xmin, ymin, xmax, ymax;

  if (!isShown()) {
    return;
  }

  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  /* Fill the scalar combo */
  ScalarList->clear();
  KST::scalarList.lock().readLock();
  for (index = 0; index < (int)KST::scalarList.count(); index++) {
    ScalarList->insertItem(KST::scalarList[index]->tagLabel());
  }
  KST::scalarList.lock().readUnlock();

  /* Fill the Select combo, and set it to the right item */
  if (new_index >= 0) {
    index = new_index;
  } else if (Select->count() > 0) {
    index = Select->currentItem();
  } else {
    index = Select->count()-1;
  }

  Select->clear();
  for (i_plot = 0; i_plot < (int)KST::plotList.count(); i_plot++) {
    Select->insertItem(KST::plotList.at(i_plot)->tagName());
  }

  if (index >= 0 && index < Select->count()) {
    Select->setCurrentItem(index);
  } else if (Select->count() > 0) {
    Select->setCurrentItem(Select->count()-1);
  }

  /* Fill the displayed and available curve lists */
  DisplayedCurveList->clear();
  AvailableCurveList->clear();

  if (Select->count() > 0) {
    i_plot = Select->currentItem();
    PlotPosition->setValue(i_plot);
  } else {
    i_plot = -1;
    PlotPosition->setValue(0);
  }

  if (i_plot >= 0) {
    plot = KST::plotList.at(i_plot);

    Colors->setForeground(plot->foregroundColor());
    Colors->setBackground(plot->backgroundColor());

    /* insert the current plot name in the plot name edit box */
    Name->setText(plot->tagName());

    /* fill the plotted and available curves boxes */
    for (KstBaseCurveList::iterator it = curves.begin(); it != curves.end(); it++) {
      if (plot->Curves.find(*it) == plot->Curves.end()) {
        AvailableCurveList->insertItem((*it)->tagName());
      } else {
        DisplayedCurveList->insertItem((*it)->tagName());
      }
    }
    /* file the log axis check boxes */
    XIsLog->setChecked(plot->isXLog());
    YIsLog->setChecked(plot->isYLog());

    plot->getScale(xmin, ymin, xmax, ymax);

    XFixedMin->setText(QString::number(xmin));
    XFixedMax->setText(QString::number(xmax));
    YFixedMin->setText(QString::number(ymin));
    YFixedMax->setText(QString::number(ymax));

    XACRange->setText(QString::number(xmax - xmin));
    YACRange->setText(QString::number(ymax - ymin));

    if (plot->getXScaleMode() == AUTO) {
      XAuto->setChecked(true);
    } else if (plot->getXScaleMode() == AC) {
      XAC->setChecked(true);
    } else if (plot->getXScaleMode() == FIXED) {
      XFixed->setChecked(true);
    } else if (plot->getXScaleMode() == AUTOUP) {
      XAutoUp->setChecked(true);
    } else if (plot->getXScaleMode() == NOSPIKE) {
      XNoSpikes->setChecked(true);
    } else {
      XAuto->setChecked(true);
    }

    if (plot->getYScaleMode() == AUTO) {
      YAuto->setChecked(true);
    } else if (plot->getYScaleMode() == AC) {
      YAC->setChecked(true);
    } else if (plot->getYScaleMode() == FIXED) {
      YFixed->setChecked(true);
    } else if (plot->getYScaleMode() == AUTOUP) {
      YAutoUp->setChecked(true);
    } else if (plot->getYScaleMode() == NOSPIKE) {
      YNoSpikes->setChecked(true);
    } else {
      YAuto->setChecked(true);
    }

    NumberFontSize->setValue(plot->TickLabel->size());

    XAxisText->setText(plot->XLabel->text());
    XLabelFontSize->setValue(plot->XLabel->size());

    YAxisText->setText(plot->YLabel->text());
    YLabelFontSize->setValue(plot->YLabel->size());

    TopLabelText->setText(plot->TopLabel->text());
    TopLabelFontSize->setValue(plot->TopLabel->size());

    FontComboBox->setCurrentFont(plot->TopLabel->fontName());

    updateLabels();
  } else {
    for (KstBaseCurveList::iterator it = curves.begin(); it != curves.end(); it++) {
      AvailableCurveList->insertItem((*it)->tagName());
    }
  } 
  
  PlotCols->setValue(KST::plotList.getPlotCols());

  UpdateDelay->setValue(doc->delay());

  updateButtons();
}

void KstPlotDialogI::applyLabels(KstPlot *plot) {
  int i_plot, plot_min, plot_max;

  plot->XLabel->setText((XAxisText)->text());
  plot->YLabel->setText((YAxisText)->text());
  plot->TopLabel->setText((TopLabelText)->text());

  /** Set the font info */
  if (applyFontsToAll->isChecked()) {
    i_plot = plot_min = 0;
    plot_max = KST::plotList.count() - 1;
  } else {
    plot_min = plot_max = i_plot = Select->currentItem();
  }

  for (i_plot = plot_min; i_plot <= plot_max; i_plot++) {
    plot = KST::plotList.at(i_plot);

    /* set fonts */
    /* X Axis Font */
    plot->XLabel->setFontName(FontComboBox->currentText());
    plot->XLabel->setSize(XLabelFontSize->value());

    /* Y Axis Font */
    plot->YLabel->setFontName(FontComboBox->currentText());
    plot->YLabel->setSize(YLabelFontSize->value());

    /* Top Label Font */
    plot->TopLabel->setFontName(FontComboBox->currentText());
    plot->TopLabel->setSize(TopLabelFontSize->value());

    /* TickLabel Font */
    plot->TickLabel->setFontName(FontComboBox->currentText());
    plot->TickLabel->setSize(NumberFontSize->value());
  }
}

void KstPlotDialogI::applyLimits(KstPlot *plot) {

  /* do X Scale */
  if (XAC->isChecked()) {
    plot->setXScaleMode(AC);
    plot->setXScale(0, XACRange->text().toDouble());
  } else if (XFixed->isChecked()) {
    plot->setXScaleMode(FIXED);
    plot->setXScale(XFixedMin->text().toDouble(), XFixedMax->text().toDouble());
  } else if (XAutoUp->isChecked()) {
    plot->setXScaleMode(AUTOUP);
  } else if (XAuto->isChecked()) {
    plot->setXScaleMode(AUTO);
  } else if (XNoSpikes->isChecked()) {
    plot->setXScaleMode(NOSPIKE);
  } else {
    kdDebug() << "Bug in kst: no x scale type checked (?!!)" << endl;
  }

    /* do Y Scale */
  if (YAC->isChecked()) {
    plot->setYScaleMode(AC);
    plot->setYScale(0, YACRange->text().toDouble());
  } else if (YFixed->isChecked()) {
    plot->setYScaleMode(FIXED);
    plot->setYScale(YFixedMin->text().toDouble(), YFixedMax->text().toDouble());
  } else if (YAutoUp->isChecked()) {
    plot->setYScaleMode(AUTOUP);
  } else if (YAuto->isChecked()) {
    plot->setYScaleMode(AUTO);
  } else if (YNoSpikes->isChecked()) {
    plot->setYScaleMode(NOSPIKE);
  } else {
    kdDebug() << "Bug in kst: no y scale type checked (?!!)" << endl;
  }

  plot->setLog(XIsLog->isChecked(), YIsLog->isChecked());
}

void KstPlotDialogI::new_I() {
  KstPlot *plot;

  /** verify that the plotname is unique */
  if (KST::plotList.FindKstPlot(Name->text()) != 0) {
    QString message = i18n("Could not create a new plot.\n"
                           "%1: not a unique plot name.\n"
                           "Change it to a unique name.").arg(Name->text());

    KMessageBox::sorry(NULL,message);
    return;
  }

  plot = new KstPlot(Name->text());
  KST::plotList.append(plot);

  KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
  /* add the curves */
  plot->Curves.clear();
  for (unsigned i = 0; i < DisplayedCurveList->count(); i++) {
    KstBaseCurveList::Iterator it = curves.findTag(DisplayedCurveList->text(i));
    if (it != curves.end()) {
      plot->Curves.append(*it);
    }
  }
  curves.clear();

  applyLabels(plot);
  //plot->GenerateDefaultLabels();

  applyLimits(plot);

  plot->setForegroundColor(Colors->foreground());
  plot->setBackgroundColor(Colors->background());

  update();

  changePosition(PlotPosition->value());

  KST::plotList.arrangePlots(PlotCols->value());
  doc->setDelay(UpdateDelay->value());

  update();
  emit docChanged();
}

void KstPlotDialogI::edit_I() {
  KstPlot *plot;

  int index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L, i18n("You need to select an active plot to edit."));
    return;
  }
  if (index >= (int)KST::plotList.count()) {
    new_I();
  } else {
    plot = KST::plotList.at(index);
    plot->setTagName(Name->text());
    /* add the curves */
    plot->Curves.clear();
    KstBaseCurveList curves = kstObjectSubList<KstDataObject, KstBaseCurve>(KST::dataObjectList);
    for (unsigned i = 0; i < DisplayedCurveList->count(); i++) {
	    KstBaseCurveList::Iterator it = curves.findTag(DisplayedCurveList->text(i));
	    if (it != curves.end()) {
		    plot->Curves.append(*it);
	    }
    }
    curves.clear();

    applyLabels(plot);
    applyLimits(plot);

    if (ApplyToAll->isChecked()) {
      for (unsigned i = 0; i < KST::plotList.count(); i++) {
        KST::plotList.at(i)->setForegroundColor(Colors->foreground());
        KST::plotList.at(i)->setBackgroundColor(Colors->background());
      }
    } else {
      plot->setForegroundColor(Colors->foreground());
      plot->setBackgroundColor(Colors->background());
    }
    changePosition(PlotPosition->value());

    KST::plotList.arrangePlots(PlotCols->value());
    doc->setDelay(UpdateDelay->value());

    update();
    emit docChanged();
  }
}

void KstPlotDialogI::delete_I() {
  int index = Select->currentItem();
  if (index < 0) {
    KMessageBox::sorry(0L,i18n("You need to select an active plot to delete."));
    return;
  }

  KST::plotList.remove(index);
  KST::plotList.arrangePlots(PlotCols->value());
  update();

  emit docChanged();
}

void KstPlotDialogI::updateSampNumLabel() {
  if (FontComboBox->currentText() != 0) {
    QPixmap pix(SampleNumLabel->size());
    QPainter p(&pix);

    p.fillRect(p.window(), Colors->background());
    p.setPen(Colors->foreground());

    SampleLabel->setFontName(FontComboBox->currentText());
    SampleLabel->setSize(NumberFontSize->value());
    SampleLabel->setText("1.234E-5");
    SampleLabel->draw(p,SampleNumLabel->width()/2,
                      SampleNumLabel->height()/2,true);
    
    SampleNumLabel->setPixmap(pix);

    QFont font(FontComboBox->currentText(), NumberFontSize->value()+12);
    SampleNumLabel->setFont(font);
  } else {
    QFont font;
    SampleNumLabel->setFont(font);
  }
}

void KstPlotDialogI::updateSampTopLabel() {
  if (FontComboBox->currentText() != 0) {
    QPixmap pix(SampleTopLabel->size());
    QPainter p(&pix);
        
    p.fillRect(p.window(), Colors->background());
    p.setPen(Colors->foreground());

    SampleLabel->setFontName(FontComboBox->currentText());
    SampleLabel->setSize(TopLabelFontSize->value());
    SampleLabel->setText(TopLabelText->text());
    SampleLabel->draw(p,SampleTopLabel->width()/2,
                      SampleTopLabel->height()/2,true);
    
    SampleTopLabel->setPixmap(pix);
  } else {
    QFont font;
    SampleTopLabel->setFont(font);
  }
}

void KstPlotDialogI::updateSampXLabel() {
  if (FontComboBox->currentText() != 0) {
    QPixmap pix(SampleXLabel->size());
    QPainter p(&pix);

    p.fillRect(p.window(), Colors->background());
    p.setPen(Colors->foreground());

    SampleLabel->setFontName(FontComboBox->currentText());
    SampleLabel->setSize(XLabelFontSize->value());
    SampleLabel->setText(XAxisText->text());
    SampleLabel->draw(p,SampleXLabel->width()/2,
                      SampleXLabel->height()/2,true);
    
    SampleXLabel->setPixmap(pix);
  } else {
    QFont font;
    SampleXLabel->setFont(font);
  }
}

void KstPlotDialogI::updateSampYLabel() {
  if (FontComboBox->currentText() != 0) {
    QPixmap pix(SampleYLabel->size());
    QPainter p(&pix);

    p.fillRect(p.window(), Colors->background());
    p.setPen(Colors->foreground());

    SampleLabel->setFontName(FontComboBox->currentText());
    SampleLabel->setSize(YLabelFontSize->value());
    SampleLabel->setText(YAxisText->text());
    SampleLabel->draw(p,SampleYLabel->width()/2,
                      SampleYLabel->height()/2,true);
    
    SampleYLabel->setPixmap(pix);
   } else {
    QFont font;
    SampleYLabel->setFont(font);
  }
}

void KstPlotDialogI::updateLabels() {
  updateSampYLabel();
  updateSampXLabel();
  updateSampTopLabel();
  updateSampNumLabel();
}

void KstPlotDialogI::removeDisplayedCurve(){
  int i_curve = DisplayedCurveList->currentItem();
  if (i_curve >= 0) {
    AvailableCurveList->insertItem(DisplayedCurveList->currentText());
    DisplayedCurveList->removeItem(i_curve);
  }
  updateButtons();
}

void KstPlotDialogI::addDisplayedCurve(){
  int i_curve = AvailableCurveList->currentItem();
  if (i_curve >= 0) {
    DisplayedCurveList->insertItem(AvailableCurveList->currentText());
    AvailableCurveList->removeItem(i_curve);
  }
  updateButtons();
}


void KstPlotDialogI::applyAutoLabels() {
  if (KST::plotList.count() < 1)
    return;

  KstPlot *plot = KST::plotList.at(Select->currentItem());
  plot->GenerateDefaultLabels();
  update();
}

void KstPlotDialogI::changePosition(int ni) {
  if (ni >= Select->count()) {
    ni = Select->count() - 1;
  }

  int ci = Select->currentItem();
  KST::plotList.insert(ni, KST::plotList.take(ci));
  Select->setCurrentItem(ni);
}

void KstPlotDialogI::updateButtons() {
  CurvePlot->setEnabled(AvailableCurveList->selectedItem() != 0L);
  CurveUnplot->setEnabled(DisplayedCurveList->selectedItem() != 0L);
}

#include "kstplotdialog_i.moc"
// vim: et ts=2 sw=2
