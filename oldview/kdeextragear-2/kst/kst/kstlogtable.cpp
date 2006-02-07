/***************************************************************************
                          kstlogtable.cpp -  description
                             -------------------
    begin                : Thu Mar 4 2004
    copyright            : (C) 2004 Andrew Walker 
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
#include <qpalette.h>
#include <qtable.h>
#include <qcolor.h>
#include <klocale.h>
#include "kstlogtable.h"
#include "kstdebug.h"

KstLogTable::KstLogTable( QWidget * parent, const char * name ) : QTable( parent, name ) {
  _bMoveToLast = FALSE;
  _iLast = 0;
}

void	KstLogTable::setDebug( KstDebug* debug ) {
  _debug = debug;
}

void KstLogTable::clear( ) {
  setNumRows( 0 );
  _iLast = 0;
}

void KstLogTable::hideRows( KstDebug::LogLevel level, bool bHide, int iStart ) {
  QValueList<KstDebug::LogMessage>* pMessages;
  int iNumRows;
  int i;

  pMessages = KstDebug::self()->messages();
  if( pMessages ) {
    iNumRows = pMessages->count();
    
    setUpdatesEnabled( FALSE );
    
    for( i=iStart; i<iNumRows; i++ ) {
      if( (*pMessages)[i].level == level ) {
        if( bHide ) {
          hideRow( i );
        } else {
          showRow( i );
        }
      }
    }
    
    hide( );
    show( );
    
    setUpdatesEnabled( TRUE );
  }
}

void KstLogTable::paintEvent( QPaintEvent* pPaintEvent ) {
  int iNumRows;
  
  iNumRows = KstDebug::self()->messages()->count();
  _iLast = iNumRows;
  
  if( iNumRows != numRows() ) {
    setNumRows( iNumRows );
  }

  if( _bMoveToLast ) {
    ensureCellVisible( iNumRows-1, 0 );
    _bMoveToLast = FALSE;
  }
  
  QTable::paintEvent( pPaintEvent );
}

void KstLogTable::paintCell( QPainter* painter, int row, int col, const QRect& cr, bool selected, const QColorGroup& cg ) {
  QPointArray pointArray;
  QColor color;
  QString str;
  int		  iMargin;
  int     iHeight;
  int			iStep;
  
  if (selected) {
    painter->eraseRect( 0, 0, cr.width(), cr.height() );
    painter->fillRect( 0, 0, cr.width(), cr.height(), cg.highlight() );
    painter->setPen(cg.highlightedText());
  } else {
    painter->eraseRect( 0, 0, cr.width(), cr.height() );
    painter->fillRect( 0, 0, cr.width(), cr.height(), cg.base() );
    painter->setPen(cg.text());
  }

  if( _debug->messages()->count() > (unsigned int)row ) {
    iMargin = cr.height()/5;
    if( iMargin == 0 ) {
      iMargin = 1;
    }
    iHeight = cr.height() - ( 2 * iMargin );
    iStep   = iHeight / 4;
    if( iStep == 0 ) {
      iStep = 1;
    }
    if( col == 0 ) {
      switch( (*(_debug->messages()))[row].level ) {
      case KstDebug::Notice:
        color.setNamedColor( I18N_NOOP("LightSeaGreen") );
        painter->setBrush( color );
        painter->drawEllipse( iMargin, iMargin, iHeight, iHeight );
        break;
      case KstDebug::Warning:
        pointArray.putPoints( 0, 3, iMargin, iMargin, 
                              iMargin + iHeight, iMargin, 
                              iMargin + ( iHeight / 2 ), iMargin + iHeight );
        color.setNamedColor( I18N_NOOP("DarkOrange") );
        painter->setBrush( color );
        painter->drawPolygon( pointArray );
        break;
      case KstDebug::Error:
        color.setNamedColor( I18N_NOOP("Red") );
        painter->setBrush( color );
        pointArray.putPoints( 0, 8, 
                              iMargin + ( 0 * iStep ), iMargin + ( 1 * iStep ), 
                              iMargin + ( 0 * iStep ), iMargin + ( 3 * iStep ), 
                              iMargin + ( 1 * iStep ), iMargin + ( 4 * iStep ), 
                              iMargin + ( 3 * iStep ), iMargin + ( 4 * iStep ), 
                              iMargin + ( 4 * iStep ), iMargin + ( 3 * iStep ), 
                              iMargin + ( 4 * iStep ), iMargin + ( 1 * iStep ), 
                              iMargin + ( 3 * iStep ), iMargin + ( 0 * iStep ), 
                              iMargin + ( 1 * iStep ), iMargin + ( 0 * iStep ) ); 
        painter->drawPolygon( pointArray );
        break;
      case KstDebug::Debug:
        color.setNamedColor( I18N_NOOP("DeepSkyBlue") );
        painter->setBrush( color );
        painter->drawRoundRect( iMargin, iMargin, 
                                iHeight, iHeight, 
                                iHeight / 3, iHeight / 3 );
        break;
      default:
        break;
      }
      painter->drawText( iHeight+2*iMargin, 0, 
                         cr.width()-iHeight-2*iMargin, cr.height(), 
                         AlignLeft, 
                         _debug->getLabel((*(_debug->messages()))[row].level));
    }
    else if( col == 1 ) {    
      str = (*(_debug->messages()))[row].date.toString();
      painter->drawText(0, 0, cr.width(), cr.height(), AlignLeft, str);
    }
    else if( col == 2 ) {
      str = (*(_debug->messages()))[row].msg;
      painter->drawText(0, 0, cr.width(), cr.height(), AlignLeft, str);
    }
  }
}
