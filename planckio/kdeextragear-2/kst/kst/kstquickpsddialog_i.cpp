/***************************************************************************
                      kstquickpsddialog_i.cpp  -  Part of KST
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
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kcolordialog.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

#include "kstplotlist.h"
#include "kstrvector.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"

#include "kstquickpsddialog_i.h"

KstQuickPSDDialogI::KstQuickPSDDialogI(QWidget* parent,
                                           const char* name,
                                           bool modal,
                                           WFlags fl)
: KstQuickPSDDialog(parent, name, modal, fl) {


    connect((ColorChange), SIGNAL(clicked()),
            this, SLOT(changeColor()));
    connect((Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect((ApplyPlotCols), SIGNAL(clicked()),
            this, SLOT(addPlot()));
    connect((SourceVector), SIGNAL(stateChanged(int)),
            this, SLOT(updateActiveEntry(int)));

    FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly
                      | KFile::LocalOnly);

}

KstQuickPSDDialogI::~KstQuickPSDDialogI() {
}

void KstQuickPSDDialogI::showQuickPSDDialog() {
  update();
  show();
  raise();
}

void KstQuickPSDDialogI::update() {
  unsigned int i, j_v;
  KstRVectorPtr v0;
  int j;

  KstPSDCurveList curves = kstObjectSubList<KstDataObject,KstPSDCurve>(KST::dataObjectList);
  j_v = KST::vectorList.count();
  if (j_v > 0) {
    SourceVector->setEnabled(true);
    j_v--;

    /* fill the vectors list */
    Vectors->clear();
    for (i = 0; i <= j_v; i++) {
      Vectors->insertItem(KST::vectorList[i]->tagName());
    }
    Vectors->setCurrentItem(j_v);

    /* update filename list */
    j = KST::fileList.count()-1;
    if (j >= 0) {
      FileName->setURL(KST::fileList[j]->fileName());
    }

    /* clear the field entry */
    Field->clear();
  }

  KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
  j_v = rvl.count();
  if (j_v > 0) {
    j_v--;
    /* set the file range defaults to those of the last RVector */
    v0 = rvl[j_v];

    CountFromEnd->setChecked(v0->countFromEOF());
    ReadToEnd->setChecked(v0->readToEOF());

    F0->setValue(v0->reqStartFrame());
    N->setValue(v0->reqNumFrames());
    Skip->setValue(v0->skip());
    DoSkip->setChecked(v0->doSkip());
    DoFilter->setChecked(v0->doAve());
    DataFileEntry->setEnabled(false);
  } else {
    SourceVector->setChecked(false);
    SourceVector->setEnabled(false);
    SourceDataFile->setChecked(true);
    VectorEntry->setEnabled(false);
    DataFileEntry->setEnabled(true);
  }

  /* Fill the plot list with options */
  PlotList->clear();
  if (KST::plotList.count()>0) {
    for (i = 0; i < KST::plotList.count(); i++) {
      PlotList->insertItem(KST::plotList.at(i)->tagName());
    }
    PlotList->setCurrentItem(0);
  }

  PlotCols->setValue(KST::plotList.getPlotCols());

  /* set the color and draw the line */
  setCurveColor(KstColorSequence::next());
  drawCurveLine();
}

void KstQuickPSDDialogI::setCurveColor(const QColor &c) {
  CurveColor = c;
}


QColor KstQuickPSDDialogI::curveColor() {
  return CurveColor;
}


void KstQuickPSDDialogI::drawCurveLine() {
  QPixmap pix(CurveAppearanceLabel->size());
  QPainter p(&pix);
  KstPoint tmppoint;

  p.fillRect(p.window(), QColor("white"));
  p.setPen(curveColor());

  p.drawLine(1,pix.height()/2,pix.width()-1, pix.height()/2);
  CurveAppearanceLabel->setPixmap(pix);
}


void KstQuickPSDDialogI::changeColor() {
  QColor c;
  c = curveColor();
  if ((int)KColorDialog::getColor(c) == (int)KColorDialog::Accepted ) {
    setCurveColor(c);
  }
  drawCurveLine();
}

void KstQuickPSDDialogI::apply(bool autolabel) {
  KstFilePtr file;
  KstVectorPtr vx;
  KstRVectorPtr trv;
  KstPlot *plot;
  int i_v;
  QString v_name, c_name;
  bool x_is_new;
  KstPSDCurvePtr curve;
  double new_freq;
  int new_len;

  if (KST::plotList.count() < 1) {
    addPlot();
    return;
  }

  if (SourceVector->isChecked()) { // set vx from existing vectors
    i_v = Vectors->currentItem();
    if (i_v >= (int)KST::vectorList.count()) {
      return;
    }
    vx = KST::vectorList[i_v];
  } else { // set vx from data file specification

    /* generate or find the kstfile */
    KstFileList::Iterator it = KST::fileList.findFileName(FileName->url());

    if (it == KST::fileList.end()) {
      file = new KstFile(FileName->url());
      if (file->numFrames() < 1) { // No data in file
        KMessageBox::sorry(0L, i18n("The requested file does not contain data."));
        return;
      }
      KST::fileList.append(file);
    } else {
      file = *it;
    }

    KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
    x_is_new = true;
    /**** Build the XVector ***/
    /* make sure there are no vectors with the current vectors props */
    for (i_v = 0; unsigned(i_v) < rvl.count(); i_v++) {
      trv = rvl[i_v];
      if ((trv->getFilename() == FileName->url()) &&
          (trv->getField() == Field->text()) &&
          (trv->reqStartFrame() == F0->value()) &&
          (trv->reqNumFrames() == N->value()) &&
          (trv->skip() == Skip->value()) &&
          (trv->doSkip() == DoSkip->isChecked()) &&
          (trv->doAve() == DoFilter->isChecked()) &&
          (trv->readToEOF() == ReadToEnd->isChecked()) &&
          (trv->countFromEOF() == CountFromEnd->isChecked())) {
        x_is_new = false;
        i_v = rvl.count();
        vx = trv;
      }
    }

    if (x_is_new) {
      /* If not, Generate a unique vector name */
      v_name = "V" + QString::number(KST::vectorList.count()+1)+"-"
               + Field->text();
      while (KST::vectorList.findTag(v_name) != KST::vectorList.end()) {
        v_name += "'";
      }
      while (KST::dataObjectList.findTag(v_name) != KST::dataObjectList.end()) {
        v_name += "'";
      }

      if (!file->isValidField(Field->text())) {
        KMessageBox::sorry(0L, i18n("The requested field is not defined for the requested file."));
        return;
      }

      /* generate and append the vector */
      trv = new KstRVector(file, Field->text(),
                          v_name,
                          (CountFromEnd->isChecked() ? -1 : F0->value()),
                          (ReadToEnd->isChecked() ? -1 : N->value()),
                          Skip->value(),
                          DoSkip->isChecked(),
                          DoFilter->isChecked());
      vx = trv;
    }
  }
  /**** Build the PSD ***/
  /* find new_freq */
  new_freq = PSDSampRate->text().toDouble();
  if (new_freq <= 0) {
      KMessageBox::sorry(0L, i18n("The sample rate must be greater than 0."));
      return;
  }

  /* find new_len */
  new_len = PSDFFTLen->text().toInt();
  if (new_len<2) {
      KMessageBox::sorry(0L, i18n("The FFT length must be greater than 2^2."));
      return;
  }

  /* create the psd curve name */
  c_name = "PSD"+QString::number(KST::dataObjectList.count()+1) + "-" + vx->tagName();
  while (KST::dataObjectList.findTag(c_name) != KST::dataObjectList.end()) {
    c_name+="'";
  }
  while (KST::vectorList.findTag(c_name) != KST::vectorList.end()) {
    c_name+="'";
  }

  /* create the psd curve */
  curve = new KstPSDCurve(c_name, vx, new_freq, new_len,
                        PSDVectorUnits->text(), PSDRateUnits->text(),
                        curveColor());

  KST::dataObjectList.append(curve.data());
  /* assign curve to plot */
  plot = KST::plotList.FindKstPlot(PlotList->currentText());
  plot->addCurve(curve);
  if (autolabel)
    plot->GenerateDefaultLabels();

  close();
  emit docChanged();
  update();
}

void KstQuickPSDDialogI::addPlot() {
  KstPlot *plot = KST::plotList.addPlot(QString::null, PlotCols->value());
  PlotList->insertItem(plot->tagName());
  PlotList->setCurrentItem(PlotList->count()-1);
  apply(true);
}

void KstQuickPSDDialogI::updateActiveEntry(int use_vector) {
  if (use_vector==0) {
    VectorEntry->setEnabled(false);
    DataFileEntry->setEnabled(true);
  } else {
    VectorEntry->setEnabled(true);
    DataFileEntry->setEnabled(false);
  }
}

#include "kstquickpsddialog_i.moc"

