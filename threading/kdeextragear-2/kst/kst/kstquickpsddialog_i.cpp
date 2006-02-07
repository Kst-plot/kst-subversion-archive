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
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qwidget.h>

#include <kcolordialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstsettings.h>
#include <kurlrequester.h>

#include "kstplotlist.h"
#include "kstrvector.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"

#include "kstquickpsddialog_i.h"
#include "curveappearancewidget.h"

KstQuickPSDDialogI::KstQuickPSDDialogI(QWidget* parent, const char* name,
                                           bool modal, WFlags fl)
: KstQuickPSDDialog(parent, name, modal, fl) {
    connect(Apply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ApplyPlotCols, SIGNAL(clicked()), this, SLOT(addPlot()));
    connect(SourceVector, SIGNAL(stateChanged(int)),
            this, SLOT(updateActiveEntry(int)));

    FileName->setMode(KFile::File | KFile::Directory | KFile::ExistingOnly
                      | KFile::LocalOnly);
    
    DataFileEntry->setEnabled(false);
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
  KST::vectorList.lock().readLock();
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
    KST::dataSourceList.lock().readLock();
    j = KST::dataSourceList.count() - 1;
    if (j >= 0) {
      FileName->setURL(KST::dataSourceList[j]->fileName());
    }
    KST::dataSourceList.lock().readUnlock();

    /* clear the field entry */
    Field->clear();
  }
  KST::vectorList.lock().readUnlock();

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

  /* set the color*/
  _curveAppearance->setColor(KstColorSequence::next());
  setCurveColor( _curveAppearance->color() );
}

void KstQuickPSDDialogI::setCurveColor(const QColor &c) {
  CurveColor = c;
}


QColor KstQuickPSDDialogI::curveColor() {
  return CurveColor;
}

void KstQuickPSDDialogI::changeColor() {
  QColor c;
  c = curveColor();
  if ((int)KColorDialog::getColor(c) == (int)KColorDialog::Accepted ) {
    setCurveColor(c);
  }
  _curveAppearance->setColor(c);
}

void KstQuickPSDDialogI::apply(bool autolabel) {
  KstDataSourcePtr file;
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
    KstReadLocker ml(&KST::vectorList.lock());
    if (i_v >= (int)KST::vectorList.count()) {
      return;
    }
    vx = KST::vectorList[i_v];
  } else { // set vx from data file specification
    KstReadLocker ml(&KST::dataSourceList.lock());

    /* generate or find the kstfile */
    KstDataSourceList::Iterator it = KST::dataSourceList.findFileName(FileName->url());

    if (it == KST::dataSourceList.end()) {
      file = KstDataSource::loadSource(FileName->url());
      if (!file || !file->isValid()) {
        KMessageBox::sorry(0L, i18n("The file could not be loaded."));
        return;
      }
      if (file->frameCount() < 1) {
        KMessageBox::sorry(0L, i18n("The file does not contain data."));
        return;
      }
      KST::dataSourceList.append(file);
    } else {
      file = *it;
    }

    KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);
    x_is_new = true;
    /**** Build the XVector ***/
    /* make sure there are no vectors with the current vectors props */
    for (i_v = 0; unsigned(i_v) < rvl.count(); i_v++) {
      trv = rvl[i_v];
      if ((trv->filename() == FileName->url()) &&
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
      KST::vectorList.lock().readLock();
      /* If not, Generate a unique vector name */
      v_name = "V" + QString::number(KST::vectorList.count()+1)+"-"
               + Field->text();
      while (KST::vectorList.findTag(v_name) != KST::vectorList.end()) {
        v_name += "'";
      }
      KST::vectorList.lock().readUnlock();
      KST::dataObjectList.lock().readLock();
      while (KST::dataObjectList.findTag(v_name) != KST::dataObjectList.end()) {
        v_name += "'";
      }
      KST::dataObjectList.lock().readUnlock();

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
  if (new_len < 2) {
      KMessageBox::sorry(0L, i18n("The FFT length must be greater than 2^2."));
      return;
  }

  /* create the psd curve name */
  KST::dataObjectList.lock().writeLock();
  c_name = "PSD"+QString::number(KST::dataObjectList.count()+1) + "-" + vx->tagName();
  while (KST::dataObjectList.findTag(c_name) != KST::dataObjectList.end()) {
    c_name+="'";
  }
  KST::vectorList.lock().readLock();
  while (KST::vectorList.findTag(c_name) != KST::vectorList.end()) {
    c_name+="'";
  }
  KST::vectorList.lock().readUnlock();

  /* create the psd curve */
  curve = new KstPSDCurve(c_name, vx, new_freq, new_len,
                        PSDVectorUnits->text(), PSDRateUnits->text(),
                        _curveAppearance->color());
  curve->setHasPoints(_curveAppearance->showPoints());
  curve->setHasLines(_curveAppearance->showLines());
  curve->Point.setType(_curveAppearance->pointType());

  KST::dataObjectList.append(curve.data());
  KST::dataObjectList.lock().writeUnlock();
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
  if (use_vector == 0) {
    VectorEntry->setEnabled(false);
    DataFileEntry->setEnabled(true);
  } else {
    VectorEntry->setEnabled(true);
    DataFileEntry->setEnabled(false);
  }
}

#include "kstquickpsddialog_i.moc"

