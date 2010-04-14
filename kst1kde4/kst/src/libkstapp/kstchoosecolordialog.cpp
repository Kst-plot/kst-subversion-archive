/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2010 The University of British Columbia               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QList>
#include <QRadioButton>

#include "kst.h"
#include "kstchoosecolordialog.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstvcurve.h"
#include "kstrvector.h"

KstChooseColorDialog::KstChooseColorDialog(QWidget* parent,
              const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog(parent, fl) {
  setupUi(this);

  xVector->setChecked(true);

  connect(OK, SIGNAL(clicked()), this, SLOT(applyColors()));  

  _xSelected = true;
  _grid = 0L;
  _override = false;
}


KstChooseColorDialog::~KstChooseColorDialog() {
  delete _grid;
}


bool KstChooseColorDialog::override() {
  return _override;
}


void KstChooseColorDialog::updateChooseColorDialog() {
  //
  // cannot use dataSourceList.fileNames() as it contains datasources that
  // are not used by any curves or vectors...
  //

  KstRVectorList::iterator vc_iter;
  KstRVectorList vcList;
  QStringList fileNameList;
  QStringList::iterator it;
  
  vcList = kstObjectSubList<KstVector, KstRVector>(KST::vectorList);
  for (vc_iter = vcList.begin(); vc_iter != vcList.end(); ++vc_iter) {
    if (fileNameList.contains((*vc_iter)->filename()) == 0) {
      fileNameList.push_back((*vc_iter)->filename());
    }
  }

  cleanColorGroup();

  _grid = new QGridLayout(colorFrame);// xxx, fileNameList.count(), 2, 0, 8);
  _grid->setColumnStretch(1, 0);

  int i = fileNameList.count();

  for (it = fileNameList.begin();  it != fileNameList.end(); ++it) {
    //
    // create the filename and color selector for each file...
    //

    QLineEdit* dsName = new QLineEdit(colorFrame);
    KColorCombo* dsColor = new KColorCombo(colorFrame);
    QMap<QString, QColor>::const_iterator itColor;

    dsName->setReadOnly(true);
    dsName->setText(*it);
    _grid->addWidget(dsName, i, 0);
    _lineEdits.push_back(dsName);
    dsName->show();

    dsColor->setColor(KstColorSequence::next());
    _grid->addWidget(dsColor, i, 1);
    _colorCombos.push_back(dsColor);
    dsColor->show();

    itColor = _fileColors.find(*it);
    if (itColor != _fileColors.end()) {
      //
      // set the color to the existing value, if present...
      //

      dsColor->setColor(*itColor);
    } else {
      //
      // else assign a custom color...
      //

      dsColor->setColor(QColor(0x11, 0x22, 0x33));
    }

    i++;
  }

  adjustSize();
  resize(QSize(500, minimumSizeHint().height()));
  setFixedHeight(height());
}


void KstChooseColorDialog::cleanColorGroup() {
  while (!_lineEdits.isEmpty()) {
    QLineEdit* lineEdit = _lineEdits.back();
    _lineEdits.pop_back();

    delete lineEdit;
  }

  while (!_colorCombos.isEmpty()) {
    KColorCombo* colorCombo = _colorCombos.back();
    _colorCombos.pop_back();

    delete colorCombo;
  }

  delete _grid;
}


void KstChooseColorDialog::showChooseColorDialog() {
  updateChooseColorDialog();
  OK->setEnabled(true);
  Cancel->setEnabled(true);
  show();
  raise();
}


void KstChooseColorDialog::applyColors() {
  OK->setEnabled(false);
  Cancel->setEnabled(false);

  _fileColors.clear( );
  _xSelected = xVector->isChecked();
  _override = _applyToNewCurves->isChecked();

  KstVCurveList cvList;
  KstVCurveList::iterator cv_iter;

// xxx  cvList = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  for (cv_iter = cvList.begin(); cv_iter != cvList.end(); ++cv_iter) {
    KstVectorPtr vect;

    if (_xSelected) {
      vect = (*cv_iter)->xVector();
    } else {
      vect = (*cv_iter)->yVector();
    }

    if (kst_cast<KstRVector>(vect)) {
      QString strFile;
      QColor color;

      strFile = kst_cast<KstRVector>(vect)->filename();
      color = getColorForFile(strFile);

      (*cv_iter)->setColor(color);

      _fileColors.insert(strFile, color);
    }
  }

  //
  // force an update in case we're in paused mode...
  //

  KstApp::inst()->forceUpdate();

  close();
}


QColor KstChooseColorDialog::getColorForFile(const QString &fileName) {
  QList<KColorCombo*>::iterator kc_iter = _colorCombos.begin();
  QList<QLineEdit*>::iterator fn_iter;

  for (fn_iter = _lineEdits.begin(); fn_iter != _lineEdits.end(); ++fn_iter) {
    if (fileName == (*fn_iter)->text()) {
      return (*kc_iter)->color();
    }
    ++kc_iter;
  }

  return QColor();
}


QColor KstChooseColorDialog::getColorForCurve(const KstVCurvePtr curve) {
  return getColorForCurve(curve->xVector(), curve->yVector());
}


QColor KstChooseColorDialog::getColorForCurve(const KstVectorPtr xVector, const KstVectorPtr yVector) {
  QColor color;

  if (_override) {
    QMap<QString, QColor>::const_iterator it;
    KstRVector* rVect;
    KstVectorPtr vect;

    if (_xSelected) {
      vect = xVector;
    } else {
      vect = yVector;
    }

    rVect = kst_cast<KstRVector>(vect);

    if (rVect) {
      QString strFile;

      strFile = rVect->filename();
      if (!strFile.isEmpty()) {
        it = _fileColors.find(strFile);

        if (it != _fileColors.end()) {
          color = *it;
        }
      }
    }
  }

  return color;
}

#include "kstchoosecolordialog.moc"
