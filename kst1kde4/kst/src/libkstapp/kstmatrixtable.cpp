/***************************************************************************
                          kstmatrixtable.cpp -  description
                             -------------------
    begin                : Thu Mar 24 2005
    copyright            : (C) 2005 The University of British Columbia 
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

#include "kstmatrix.h"
#include "kstmatrixtable.h"
#include "kstdatacollection.h"

KstMatrixTable::KstMatrixTable( QWidget *parent, const char *name ) : QTableWidget( parent) {
  Q_UNUSED(name)
}

void KstMatrixTable::setMatrix(QString strMatrix) {
  _strMatrix = strMatrix;
}

void KstMatrixTable::paintCell( QPainter* painter, int row, int col, const QRect& cr, bool selected, const QPalette &palette ) {
  KstMatrixPtr matrix = *KST::matrixList.findTag(_strMatrix);
  QString str;
  double value;

  painter->eraseRect( 0, 0, cr.width(), cr.height() );
  if (selected) {
    painter->fillRect( 0, 0, cr.width(), cr.height(), palette.highlight() );
    painter->setPen(palette.highlightedText().color());
  } else {
    painter->fillRect( 0, 0, cr.width(), cr.height(), palette.base() );
    painter->setPen(palette.text().color());
  }

  if (matrix) {
    bool ok;
    value = matrix->valueRaw(col, row, &ok);
    if (ok) {
      str.setNum(value, 'g', 16);
    } else {
      str = "-";
    }
  }

  painter->drawText(0, 0, cr.width(), cr.height(), Qt::AlignLeft, str);
}
