/***************************************************************************
                          ksteventtable.h  -  description
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

#ifndef KSTEVENTTABLE_H
#define KSTEVENTTABLE_H

#include "kstrvector.h"

class KstEventTable : public QTable
{
public:
    KstEventTable( QWidget * parent = 0, const char * name = 0 );

    void				  setEventMonitors(EventMonitors* pEventMonitors);
    void 					paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg );
    
    void 					resizeData(int) {}
    QWidget* 			createEditor(int, int, bool) const { return 0; }
    QTableItem*		item(int, int) { return 0; }
    void 					setItem(int, int, QTableItem *) {}
    void 					clearCell(int, int) {}
    void 					insertWidget(int, int, QWidget *) {}
    QWidget*			cellWidget(int, int) const { return 0; }
    void 					clearCellWidget(int, int) {}
    
private:
    EventMonitors* _eventMonitors;
};
#endif
