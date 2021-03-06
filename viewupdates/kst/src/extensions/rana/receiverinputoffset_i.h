/***************************************************************************
                       receiverinputoffset_i.h  -  Part of KST
                             -------------------
    begin                :
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

#ifndef RECEIVERINPUTOFFSETI_H
#define RECEIVERINPUTOFFSETI_H

#include "receiverinputoffset.h"

class KstRANA;

class ReceiverInputOffsetI : public ReceiverInputOffset {
  Q_OBJECT
  public:
    ReceiverInputOffsetI( KstRANA* elog,
                        QWidget* parent = 0,
                        const char* name = 0,
                        bool modal = FALSE, WFlags fl = 0 );
    virtual ~ReceiverInputOffsetI();

    void initialize();
        
  private slots:
    virtual void shut();
    virtual void vOffsetCalculate();
    virtual void tOffsetCalculate();
    virtual void newEntry();
    virtual void deleteEntry();
    virtual void clearEntries();
    virtual void rowChanged();
    virtual void rowUpdated();
    virtual void cellValueChanged(int row, int col);
        
  signals:
    
  protected:
    
  private:
    void blockTextChangeSignals(bool block);
    void save();
    void load();
        
    KstRANA* _rana;
};


#endif // RECEIVERINPUTOFFSETI_H
