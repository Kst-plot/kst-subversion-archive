/***************************************************************************
                          ksteventtable.cpp -  description
                             -------------------
    begin                : Tue Mar 30 2004
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
#include "ksteventtable.h"
#include "kstrvector.h"
#include "kstdatacollection.h"

KstEventTable::KstEventTable( QWidget * parent, const char * name ) : QTable( parent, name ) {
  _eventMonitors = NULL;
}

void KstEventTable::setEventMonitors(EventMonitors* eventMonitors) {
  _eventMonitors = eventMonitors;
  if( _eventMonitors != NULL ) {
    setNumRows( _eventMonitors->size() );
  } else {
    setNumRows( 0 );    
  }
}

void KstEventTable::paintCell( QPainter* painter, int row, int col, const QRect& cr, bool selected, const QColorGroup& cg ) {
  QString str;
  
  if (selected) {
    painter->eraseRect( 0, 0, cr.width(), cr.height() );
    painter->fillRect( 0, 0, cr.width(), cr.height(), cg.highlight() );
    painter->setPen(cg.highlightedText());
  } else {
    painter->eraseRect( 0, 0, cr.width(), cr.height() );
    painter->fillRect( 0, 0, cr.width(), cr.height(), cg.base() );
    painter->setPen(cg.text());
  }

  if( _eventMonitors != NULL ) {
    if( col == 0 ) {
      str = KstDebug::self()->getLabel( (*_eventMonitors)[row].getLevel() );
    } else if( col == 1 ) {
      str = (*_eventMonitors)[row].getEvent();
    } else if( col == 2 ) {
      str = (*_eventMonitors)[row].getDescription();      
    }
  }
  
  painter->drawText(0, 0, cr.width(), cr.height(), AlignLeft, str);
}
