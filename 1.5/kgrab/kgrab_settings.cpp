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

#include <QFrame>

#include <KVBox>

#include "kgrab_settings.h"
#include "kgrab.h"

// ----------------    KGrabSettingsUI ---------------------------------

KGrabSettingsUI::KGrabSettingsUI( QWidget *parent )
: QWidget( parent )
{
    setupUi( this );
}

// ----------------    KGrabSettings ---------------------------------

KGrabSettings::KGrabSettings( KGrab *parent)
: KDialog( parent )
{
    KVBox *vbox = new KVBox( this );
    setMainWidget( vbox );
    ui = new KGrabSettingsUI( vbox );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
   
    grab_parent = parent;
 
    connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
    connect( ui->comboMode, SIGNAL( activated(int) ), SLOT( slotModeChanged(int) ) );
}

KGrabSettings::~KGrabSettings()
{
    delete ui;
}

void
KGrabSettings::slotModeChanged(int index)
{
    switch ( index )
    {
    case 0:
        ui->cbIncludeDecorations->setEnabled(false);
        break;
    case 1:
        ui->cbIncludeDecorations->setEnabled(true);
        break;
    case 2:
        ui->cbIncludeDecorations->setEnabled(false);
        break;
    case 3:
        ui->cbIncludeDecorations->setEnabled(false);
        break;
    default:
        break;
    }
    capturemode = index;
    if (index != ui->comboMode->currentIndex())
        ui->comboMode->setCurrentIndex( index );
}

void
KGrabSettings::setDelay(int index)
{
    delay = index;
    ui->spinDelay->setValue( index );
}

void
KGrabSettings::setDecorations(bool b)
{
    decoration = b;
    ui->cbIncludeDecorations->setChecked( decoration ); 
}

void
KGrabSettings::save()
{
    grab_parent->setGrabMode( ui->comboMode->currentIndex() );
    grab_parent->setDelay( ui->spinDelay->value() );
    grab_parent->setIncludeDecorations( ui->cbIncludeDecorations->isChecked() );
}

#include "kgrab_settings.moc"

