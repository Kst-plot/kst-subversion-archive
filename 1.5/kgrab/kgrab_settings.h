/*
 *  Copyright (C) 2006 Marcus Hufgard <marcus.Hufgard@hufgard.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KRAB_SETTINGS_H
#define KRAB_SETTINGS_H

#include <kdialog.h>

#include "ui_kgrab_settings.h"
#include "kgrab.h"

class KGrabSettingsUI : public QWidget, public Ui::SettingsDialog
{
    Q_OBJECT
    public:
        KGrabSettingsUI( QWidget *parent=0 );
};

class KGrabSettings : public KDialog
{
    Q_OBJECT
    public:
        KGrabSettings( KGrab *parent );
        ~KGrabSettings();
        void setDelay( int index );
        void setDecorations( bool b);

    public slots:
        void slotModeChanged( int index );
        
    private slots:
        void save( void );
    
    private:
        KGrabSettingsUI *ui;
        KGrab *grab_parent;
        int capturemode;
        int delay;
        bool decoration;
};

#endif // KRAB_SETTINGS_H
