/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-01-02
 * Description : collection setup tab.
 * 
 * Copyright 2004-2006 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef SETUPCOLLECTIONS_H
#define SETUPCOLLECTIONS_H

// Qt includes.

#include <qwidget.h>

class QPushButton;
class QListBox;
class QListBoxItem;

namespace Digikam
{

class SetupCollections : public QWidget
{
    Q_OBJECT
    
public:

    SetupCollections(QWidget* parent = 0);
    ~SetupCollections();

    void applySettings();

private:

    void readSettings();
    
    QListBox     *albumCollectionBox_;
    QPushButton  *addCollectionButton_;
    QPushButton  *delCollectionButton_;
    
private slots:

    void slotCollectionSelectionChanged();
    void slotAddCollection();
    void slotDelCollection();
};

}  // namespace Digikam

#endif // SETUPCOLLECTIONS_H 
