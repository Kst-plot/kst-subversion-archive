/***************************************************************************
                    kstquickcurvesdialog_i.h  -  Part of KST
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
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kcombobox.h>

#include "kstplotlist.h"
#include "kstdatacollection.h"
#include "kstcolorsequence.h"

#include "kstquickcurvesdialog_i.h"

KstQuickCurvesDialogI::KstQuickCurvesDialogI(QWidget* parent,
                                       const char* name, bool modal, WFlags fl)
: KstQuickCurvesDialog(parent, name, modal, fl) {

    connect(ColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(drawCurveLine()));
    connect(Apply, SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(ApplyPlotCols, SIGNAL(clicked()),
            this, SLOT(addPlot()));

    FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly
                      | KFile::LocalOnly);

    _xaxisCompletion = XAxis->completionObject();
    XAxis->setAutoDeleteCompletionObject(true);
    _yaxisCompletion = YAxis->completionObject();
    YAxis->setAutoDeleteCompletionObject(true);
}

KstQuickCurvesDialogI::~KstQuickCurvesDialogI() {
}

void KstQuickCurvesDialogI::showQuickCurvesDialog() {
  update();
  show();
  raise();
}


void KstQuickCurvesDialogI::update() {
  unsigned int i;
  int j;
  KstRVectorPtr v0;

  KstVCurveList curves = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  /* put INDEX as a default in the X axis list */
  XAxis->clear();
  YAxis->clear();

  /* update filename list and fill axes combo boxes */
  j = KST::fileList.count()-1;
  if (j >= 0) {
    FileName->setURL(KST::fileList[j]->fileName());
    XAxis->insertStringList(KST::fileList[j]->fieldList());
    _xaxisCompletion->insertItems(KST::fileList[j]->fieldList());
    YAxis->insertStringList(KST::fileList[j]->fieldList());
    _yaxisCompletion->insertItems(KST::fileList[j]->fieldList());
  }

  XAxis->setCurrentText("INDEX");
  YAxis->setCurrentText("");

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  /* set the file range defaults to those of Vector 0 */
  if (rvl.count() > 0) {
    v0 = rvl[0];

    CountFromEnd->setChecked(v0->countFromEOF());
    ReadToEnd->setChecked(v0->readToEOF());

    F0->setValue(v0->reqStartFrame());
    N->setValue(v0->reqNumFrames());
    Skip->setValue(v0->skip());
    DoSkip->setChecked(v0->doSkip());
    DoFilter->setChecked(v0->doAve());
  }

  /* Fill the plot list with options */
  PlotList->clear();
  if (KST::plotList.count() > 0) {
    for (i = 0; i < KST::plotList.count(); i++) {
      PlotList->insertItem(KST::plotList.at(i)->tagName());
    }
    PlotList->setCurrentItem(0);
  }

  PlotCols->setValue(KST::plotList.getPlotCols());
  /* set the color and draw the line */
  ColorButton->setColor(KstColorSequence::next());
  drawCurveLine();
}

void KstQuickCurvesDialogI::drawCurveLine() {
  QPixmap pix(CurveAppearanceLabel->size());
  QPainter p(&pix);
  KstPoint tmppoint;

  p.fillRect(p.window(), QColor("white"));
  p.setPen(ColorButton->color());

  p.drawLine(1,pix.height()/2,pix.width()-1, pix.height()/2);
  CurveAppearanceLabel->setPixmap(pix);
}

bool KstQuickCurvesDialogI::apply(bool autolabel) {
  KstFilePtr file;
  KstRVectorPtr tv, vx, vy;
  KstVCurvePtr curve;
  KstPlot *plot;
  unsigned int i_v;
  QString v_name, c_name;
  bool x_is_new, y_is_new;

  if (KST::plotList.count() < 1) {
    return addPlot();
  }

  KstVCurveList curves = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);
  /* generate or find the kstfile */
  KstFileList::Iterator it = KST::fileList.findFileName(FileName->url());

  if (it == KST::fileList.end()) {
    file = new KstFile(FileName->url());
    if (file->numFrames() < 1) { // No data in file
      KMessageBox::sorry(0L, i18n("The requested file does not contain data."));
      return false;
    }
    KST::fileList.append(file);
  } else {
    file = *it;
  }

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  x_is_new = true;
  /**** Build the XVector ***/
  /* make sure there are no vectors with the current vectors props */
  for (i_v = 0; i_v < rvl.count(); i_v++) {
    tv = rvl[i_v];
    if ((tv->getFilename() == FileName->url()) &&
        (tv->getField() == XAxis->currentText()) &&
        (tv->reqStartFrame() == F0->value()) &&
        (tv->reqNumFrames() == N->value()) &&
        (tv->skip() == Skip->value()) &&
        (tv->doSkip() == DoSkip->isChecked()) &&
        (tv->doAve() == DoFilter->isChecked()) &&
        (tv->readToEOF() == ReadToEnd->isChecked()) &&
        (tv->countFromEOF() == CountFromEnd->isChecked())) {
      x_is_new = false;
      i_v = rvl.count();
      vx = tv;
    }
  }

  if (x_is_new) {
    /* If not, Generate a unique vector name */
    v_name = "V" + QString::number(KST::vectorList.count()+1)+"-"
             + XAxis->currentText();
    while (KST::vectorList.findTag(v_name) != KST::vectorList.end()) {
      v_name += "'";
    }
    while (curves.findTag(v_name) != curves.end()) {
      v_name += "'";
    }

    /* generate and append the vector */
    vx = new KstRVector(file, XAxis->currentText(),
                         v_name,
                         (CountFromEnd->isChecked() ? -1 : F0->value()),
                         (ReadToEnd->isChecked() ? -1 : N->value()),
                         Skip->value(),
			 DoSkip->isChecked(),
			 DoFilter->isChecked());
  }

  /**** Build the YVector ***/
  y_is_new = true;
  /* make sure there are no vectors with the current vectors props */
  for (i_v = 0; i_v < rvl.count(); i_v++) {
    tv = rvl[i_v];
    if ((tv->getFilename() == FileName->url()) &&
        (tv->getField() == YAxis->currentText()) &&
        (tv->reqStartFrame() == F0->value()) &&
        (tv->reqNumFrames() == N->value()) &&
        (tv->skip() == Skip->value()) &&
        (tv->doSkip() == DoSkip->isChecked()) &&
        (tv->doAve() == DoFilter->isChecked()) &&
        (tv->readToEOF() == ReadToEnd->isChecked()) &&
        (tv->countFromEOF() == CountFromEnd->isChecked())) {
      y_is_new = false;
      i_v = rvl.count();
      vy = tv;
    }
  }

  if (y_is_new) {
    /* If not, Generate a unique vector name */
    v_name = "V" + QString::number(KST::vectorList.count()+1)+"-"
             + YAxis->currentText();
    while (KST::vectorList.findTag(v_name) != KST::vectorList.end()) {
      v_name += "'";
    }
    while (curves.findTag(v_name) != curves.end()) {
      v_name += "'";
    }

    /* generate and append the vector */
    vy = new KstRVector(file, YAxis->currentText(),
                         v_name,
                         (CountFromEnd->isChecked() ? -1 : F0->value()),
                         (ReadToEnd->isChecked() ? -1 : N->value()),
                         Skip->value(),
			 DoSkip->isChecked(),
			 DoFilter->isChecked());
  }

  if (x_is_new) {
    if (!vx->isValid()) {
      KMessageBox::sorry(0L, i18n("The requested X vector is not in the file."));
      return false;
    }
  }
  if (y_is_new) {
    if (!vy->isValid()) {
      KMessageBox::sorry(0L, i18n("The requested Y vector is not in the file."));
      return false;
    }
  }

  /**** Build the curve ***/
  /* Generate new curve name */
  c_name = "C" + QString::number(curves.count()+1) + "-" + YAxis->currentText();
  while (curves.findTag(c_name) != curves.end()) {
    c_name += "'";
  }
  while (KST::vectorList.findTag(c_name) != KST::vectorList.end()) {
    c_name += "'";
  }

  /* Generate the new curve */
  curve = new KstVCurve(c_name, KstVectorPtr(vx), KstVectorPtr(vy), 0, 0,
                        ColorButton->color());
  KST::dataObjectList.append(curve.data());
  curves.append(curve);

  /* assign curve to plot */
  plot = KST::plotList.FindKstPlot(PlotList->currentText());
  plot->addCurve(curve);
  if (autolabel)
    plot->GenerateDefaultLabels();

  close();
  emit docChanged();
  update();
  return true;
}

bool KstQuickCurvesDialogI::addPlot() {
  KstPlot *plot = KST::plotList.addPlot(QString::null, PlotCols->value());
  PlotList->insertItem(plot->tagName());
  PlotList->setCurrentItem(PlotList->count()-1);

  if (!apply(true)) {
    PlotList->removeItem(PlotList->count()-1);
    KST::plotList.remove(plot);
    return false;
  }
  return true;
}

#include "kstquickcurvesdialog_i.moc"
