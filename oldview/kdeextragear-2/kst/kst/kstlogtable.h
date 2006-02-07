/***************************************************************************
                          kstlogtable.h  -  description
                             -------------------
    begin                : Thu Mar 25 2004
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

#ifndef KSTLOGTABLE_H
#define KSTLOGTABLE_H

#include "kstdebug.h"

class KstLogTable : public QTable
{
public:
    KstLogTable( QWidget * parent = 0, const char * name = 0 );

    void 					clear( );
    void 					hideRows( KstDebug::LogLevel level, bool bHide, int iStart );
    void					setDebug( KstDebug* debug );
    void 					paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg );
    
    void					setMoveToLast() { _bMoveToLast = TRUE; }
    void 					resizeData(int) {}
    QWidget* 			createEditor(int, int, bool) const { return 0; }
    QTableItem*		item(int, int) { return 0; }
    void 					setItem(int, int, QTableItem *) {}
    void 					clearCell(int, int) {}
    void 					insertWidget(int, int, QWidget *) {}
    QWidget*			cellWidget(int, int) const { return 0; }
    void 					clearCellWidget(int, int) {}
  
protected:
    virtual void  paintEvent( QPaintEvent* pPaintEvent ); 
      
private:            
    KstDebug*		  _debug;
    bool				  _bMoveToLast;
    int						_iLast;
};

#endif
