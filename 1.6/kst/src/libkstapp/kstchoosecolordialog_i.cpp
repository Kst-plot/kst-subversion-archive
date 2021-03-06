/**************************************************************************
        kstchoosecolordialog_i.cpp - source file: inherits designer dialog
                             -------------------
    begin                :  2001
    copyright            : (C) 2000-2003 by Barth Netterfield
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
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qvaluelist.h>
#include <qradiobutton.h>

#include <kcombobox.h>
#include <kcolorcombo.h>

#include "kst.h"
#include "kstchoosecolordialog_i.h"
#include "kstcolorsequence.h"
#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstvcurve.h"
#include "kstrvector.h"

KstChooseColorDialogI::KstChooseColorDialogI(QWidget* parent,
              const char* name, bool modal, WFlags fl)
  : KstChooseColorDialog(parent, name, modal, fl) {
  xVector->setChecked(true);
  connect(OK, SIGNAL(clicked()), this, SLOT(applyColors()));  
  _xSelected = true;
  _grid = 0L;
  _override = false;
}


KstChooseColorDialogI::~KstChooseColorDialogI() {
  delete _grid;
}


bool KstChooseColorDialogI::override() {
  return _override;
}


void KstChooseColorDialogI::updateChooseColorDialog() {
  // cannot use dataSourceList.fileNames() as it contains datasources that
  // are not used by any curves or vectors
  KstRVectorList vcList = kstObjectSubList<KstVector, KstRVector>(KST::vectorList);

  // buildup a list of filenames
  QStringList fileNameList;
  for (KstRVectorList::Iterator vc_iter = vcList.begin(); 
        vc_iter != vcList.end(); ++vc_iter) {
    if (fileNameList.contains((*vc_iter)->filename()) == 0) {
      fileNameList.push_back((*vc_iter)->filename());
    }
  }

  cleanColorGroup();

  _grid = new QGridLayout(colorFrame, fileNameList.count(), 2, 0, 8);
  _grid->setColStretch(1, 0);

  int i = fileNameList.count();

  for (QStringList::Iterator it = fileNameList.begin();  it != fileNameList.end(); ++it) {
    //
    // create the filename and color selector for each file...
    //
    QLineEdit* dsName = new QLineEdit(colorFrame, "dsName"+i);
    KColorCombo* dsColor = new KColorCombo(colorFrame, "dsColor"+i);
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


void KstChooseColorDialogI::cleanColorGroup() {
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


void KstChooseColorDialogI::showChooseColorDialog() {
  updateChooseColorDialog();
  OK->setEnabled(true);
  Cancel->setEnabled(true);
  show();
  raise();
}


void KstChooseColorDialogI::applyColors() {
  OK->setEnabled(false);
  Cancel->setEnabled(false);

  _fileColors.clear( );
  _xSelected = xVector->isChecked();
  _override = _applyToNewCurves->isChecked();

  KstVCurveList cvList = kstObjectSubList<KstDataObject, KstVCurve>(KST::dataObjectList);

  for (KstVCurveList::iterator cv_iter = cvList.begin();
        cv_iter != cvList.end(); ++cv_iter) {
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

  // force an update in case we're in paused mode
  KstApp::inst()->forceUpdate();

  close();
}


QColor KstChooseColorDialogI::getColorForFile(const QString &fileName) {
  QValueList<KColorCombo*>::Iterator kc_iter = _colorCombos.begin();

  for (QValueList<QLineEdit*>::Iterator fn_iter = _lineEdits.begin();
        fn_iter != _lineEdits.end(); ++fn_iter) {
    if (fileName == (*fn_iter)->text()) {
      return (*kc_iter)->color();
    }
    ++kc_iter;
  }

  return QColor();
}


QColor KstChooseColorDialogI::getColorForCurve(const KstVCurvePtr curve) {
  return getColorForCurve(curve->xVector(), curve->yVector());
}


QColor KstChooseColorDialogI::getColorForCurve(const KstVectorPtr xVector, const KstVectorPtr yVector) {
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

#include "kstchoosecolordialog_i.moc"
