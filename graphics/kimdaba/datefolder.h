/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef DATEFOLDER_H
#define DATEFOLDER_H

#include "folder.h"
#include <kdialogbase.h>
#include <qdatetime.h>
class KDatePicker;

class DateFolder :public Folder {
public:
    DateFolder( const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;
    virtual QString countLabel() const;
};

class DateSearchDialog :public KDialogBase
{
    Q_OBJECT
public:
    DateSearchDialog( QWidget* parent, const char* name = 0 );
    QDate fromDate() const;
    QDate toDate() const;

protected slots:
    void fromDateChanged( QDate );
    void toDateChanged();
    void slotCopy();

protected:
    void disableDefaultButtons();
    void increaseFont( QWidget* widget, int factor );

private:
    void highlightPossibleDates( KDatePicker* picker);

    QDate  _prevFrom;
    QDate  _prevTo;
    KDatePicker* _from;
    KDatePicker* _to;
    bool _toChanged;
};


#endif /* DATEFOLDER_H */

