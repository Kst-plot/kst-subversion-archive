/***************************************************************************
                          kstvectortable.cpp -  description
                             -------------------
    begin                : Thu Mar 4 2004
    copyright            : (C) 2004 The University of British Columbia 
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

#include <qpainter.h>
#include <qtable.h>
#include "kstvectortable.h"
#include "kstdatacollection.h"

KstVectorTable::KstVectorTable( QWidget * parent, const char * name ) : QTable( parent, name ) {
}

void KstVectorTable::setVector(QString strVector) {
  _strVector = strVector;
}

void KstVectorTable::paintCell( QPainter* painter, int row, int col, const QRect& cr, bool selected, const QColorGroup& cg ) {
  QString str;

  painter->eraseRect( 0, 0, cr.width(), cr.height() );
  if (selected) {
    painter->fillRect( 0, 0, cr.width(), cr.height(), cg.highlight() );
    painter->setPen(cg.highlightedText());
  } else {
    painter->fillRect( 0, 0, cr.width(), cr.height(), cg.base() );
    painter->setPen(cg.text());
  }

  if (col == 0) {
    str.setNum(row);
    painter->drawText(0, 0, cr.width(), cr.height(), AlignLeft, str);
  } else if (col == 1) {
    KstVectorPtr vector = *KST::vectorList.findTag(_strVector);

    if (vector) {
      str.setNum(vector->value(row), 'g', 16);
      painter->drawText(0, 0, cr.width(), cr.height(), AlignLeft, str);
    }
  }
}

