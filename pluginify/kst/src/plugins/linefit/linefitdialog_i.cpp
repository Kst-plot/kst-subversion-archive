/***************************************************************************
                     kstlinefitdialog_i.h  -  Part of KST
                             -------------------
    begin                : 09/12/06
    copyright            : (C) 2006 The University of Toronto
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

#include <assert.h>

#include "linefitdialogwidget.h"

// include files for Qt
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "linefitdialog_i.h"

// application specific includes
#include <kst.h>
#include <scalarselector.h>
#include <stringselector.h>
#include <vectorselector.h>

const QString& LineFitDialogI::defaultTag = KGlobal::staticQString("<Auto Name>");

QGuardedPtr<LineFitDialogI> LineFitDialogI::_inst;

LineFitDialogI::LineFitDialogI(QWidget* parent, const char* name, bool modal, WFlags fl)
: KstDataDialog(parent, name, modal, fl) {
  _w = new LineFitDialogWidget(_contents);
  setMultiple(false);
}

LineFitDialogI::~LineFitDialogI() {
}

void LineFitDialogI::fillFieldsForEdit() {
  LineFitPtr lf = kst_cast<LineFit>(_dp);
  if (!lf) {
    return;
  }

  lf->readLock();

  _tagName->setText(lf->tagName());
  _legendText->setText(defaultTag); //FIXME?

  _w->_xArray->setSelection( lf->xArrayTag() );
  _w->_yArray->setSelection( lf->yArrayTag() );

  _w->_xInterpolated->setText( lf->xInterpolatedTag() );
  _w->_yInterpolated->setText( lf->yInterpolatedTag() );

  _w->_a->setText( lf->aTag() );
  _w->_b->setText( lf->bTag() );
  _w->_chi2->setText( lf->chi2Tag() );

  lf->unlock();

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

void LineFitDialogI::fillFieldsForNew() {
  _tagName->setText(defaultTag);
  _legendText->setText(defaultTag);

  adjustSize();
  resize(minimumSizeHint());
  setFixedHeight(height());
}

#include "linefitdialog_i.moc"

// vim: ts=2 sw=2 et
