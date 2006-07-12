/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>
   Copyright (c) 2004 The University of Toronto

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qclipboard.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qapplication.h>

#include <kcompletionbox.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <kicontheme.h>
#define protected public
#include <klineedit.h>
#undef protected
#include <klocale.h>
#include <knotifyclient.h>
#include <kpixmapprovider.h>
#include <kstdaccel.h>
#include <kurl.h>
#include <kurldrag.h>

#include "ksdebug.h"

#include "kstcombobox.h"

#include <stdlib.h> // getenv

class KstComboBox::KstComboBoxPrivate
{
public:
    KstComboBoxPrivate()
    {
        klineEdit = 0L;
    }
    ~KstComboBoxPrivate()
    {
    }

    KLineEdit *klineEdit;
};

KstComboBox::KstComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    init();
}

KstComboBox::KstComboBox( bool rw, QWidget *parent, const char *name )
    : QComboBox( rw, parent, name )
{
    init();

    if ( rw )
    {
      KLineEdit *edit = new KLineEdit( this, "combo lineedit" );
      setLineEdit( edit );
    }
}

KstComboBox::~KstComboBox()
{
    delete d;
    d = 0;
}

void KstComboBox::init()
{
    d = new KstComboBoxPrivate;

    // Permanently set some parameters in the parent object.
    QComboBox::setAutoCompletion( false );

    // Enable context menu by default if widget
    // is editable.
    setContextMenuEnabled( true );
}


bool KstComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    for (int i = 0; i < count(); i++ )
    {
        if ( text(i) == _text )
            return true;
    }
    return false;
}

void KstComboBox::setAutoCompletion( bool autocomplete )
{
    if ( d->klineEdit )
    {
        if ( autocomplete )
        {
            d->klineEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
            setCompletionMode( KGlobalSettings::CompletionAuto );
        }
        else
        {
            d->klineEdit->setCompletionMode( KGlobalSettings::completionMode() );
            setCompletionMode( KGlobalSettings::completionMode() );
        }
    }
}

void KstComboBox::setContextMenuEnabled( bool showMenu )
{
    if( d->klineEdit )
        d->klineEdit->setContextMenuEnabled( showMenu );
}


void KstComboBox::setURLDropsEnabled( bool enable )
{
    if ( d->klineEdit )
        d->klineEdit->setURLDropsEnabled( enable );
}

bool KstComboBox::isURLDropsEnabled() const
{
    return d->klineEdit && d->klineEdit->isURLDropsEnabled();
}


void KstComboBox::setCompletedText( const QString& text, bool marked )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text, marked );
}

void KstComboBox::setCompletedText( const QString& text )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text );
}

void KstComboBox::makeCompletion( const QString& text )
{
    if( d->klineEdit )
        d->klineEdit->makeCompletion( text );

    else // read-only combo completion
    {
        kstdDebug() << "make completion: [" << text << "]" << endl;
        if( text.isNull() || !listBox() )
            return;

        kstdDebug() << "                 has listbox" << endl;
        int index = listBox()->index( listBox()->findItem( text ) );
        kstdDebug() << "                 index is " << index << endl;
        if( index >= 0 )
            setCurrentItem( index );
    }
}

void KstComboBox::rotateText( KCompletionBase::KeyBindingType type )
{
    if ( d->klineEdit )
        d->klineEdit->rotateText( type );
}

// not needed anymore
bool KstComboBox::eventFilter( QObject* o, QEvent* ev )
{
    return QComboBox::eventFilter( o, ev );
}

void KstComboBox::setTrapReturnKey( bool grab )
{
    if ( d->klineEdit )
        d->klineEdit->setTrapReturnKey( grab );
    else
        qWarning("KstComboBox::setTrapReturnKey not supported with a non-KLineEdit.");
}

bool KstComboBox::trapReturnKey() const
{
    return d->klineEdit && d->klineEdit->trapReturnKey();
}


void KstComboBox::setEditURL( const KURL& url )
{
    QComboBox::setEditText( url.prettyURL() );
}

void KstComboBox::insertURL( const KURL& url, int index )
{
    QComboBox::insertItem( url.prettyURL(), index );
}

void KstComboBox::insertURL( const QPixmap& pixmap, const KURL& url, int index )
{
    QComboBox::insertItem( pixmap, url.prettyURL(), index );
}

void KstComboBox::changeURL( const KURL& url, int index )
{
    QComboBox::changeItem( url.prettyURL(), index );
}

void KstComboBox::changeURL( const QPixmap& pixmap, const KURL& url, int index )
{
    QComboBox::changeItem( pixmap, url.prettyURL(), index );
}

void KstComboBox::setCompletedItems( const QStringList& items )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedItems( items );
}

KCompletionBox * KstComboBox::completionBox( bool create )
{
    if ( d->klineEdit )
        return d->klineEdit->completionBox( create );
    return 0;
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KstComboBox::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    QComboBox::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( lineEdit(), true, true );
}

void KstComboBox::wheelEvent( QWheelEvent *ev )
{
    // Not necessary anymore
    QComboBox::wheelEvent( ev );
}

void KstComboBox::setLineEdit( QLineEdit *edit )
{
    if ( !editable() && edit &&
         qstrcmp( edit->className(), "QLineEdit" ) == 0 )
    {
        // uic generates code that creates a read-only KstComboBox and then
        // calls combo->setEditable( true ), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice KLineEdit.
        // As some KstComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        edit = new KLineEdit( this, "combo edit" );
    }

    QComboBox::setLineEdit( edit );
    d->klineEdit = dynamic_cast<KLineEdit*>( edit );
    setDelegate( d->klineEdit );

    // Connect the returnPressed signal for both Q[K]LineEdits'
    if (edit)
        connect( edit, SIGNAL( returnPressed() ), SIGNAL( returnPressed() ));

    if ( d->klineEdit )
    {
        // someone calling KstComboBox::setEditable( false ) destroys our
        // lineedit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect( edit, SIGNAL( destroyed() ), SLOT( lineEditDeleted() ));

        connect( d->klineEdit, SIGNAL( returnPressed( const QString& )),
                 SIGNAL( returnPressed( const QString& ) ));

        connect( d->klineEdit, SIGNAL( completion( const QString& )),
                 SIGNAL( completion( const QString& )) );

        connect( d->klineEdit, SIGNAL( substringCompletion( const QString& )),
                 SIGNAL( substringCompletion( const QString& )) );

        connect( d->klineEdit,
                 SIGNAL( textRotation( KCompletionBase::KeyBindingType )),
                 SIGNAL( textRotation( KCompletionBase::KeyBindingType )) );

        connect( d->klineEdit,
                 SIGNAL( completionModeChanged( KGlobalSettings::Completion )),
                 SIGNAL( completionModeChanged( KGlobalSettings::Completion)));

        connect( d->klineEdit,
                 SIGNAL( aboutToShowContextMenu( QPopupMenu * )),
                 SIGNAL( aboutToShowContextMenu( QPopupMenu * )) );

        connect( d->klineEdit,
                 SIGNAL( completionBoxActivated( const QString& )),
                 SIGNAL( activated( const QString& )) );
    }
}

void KstComboBox::setCurrentItem( const QString& item, bool insert, int index )
{
    int sel = -1;

    for (int i = 0; i < count(); ++i)
    {
        if (text(i) == item)
        {
            sel = i;
            break;
        }
    }

    if (sel == -1 && insert)
    {
        insertItem(item, index);
        if (index >= 0)
            sel = index;
        else
            sel = count() - 1;
    }
    setCurrentItem(sel);
}

void KstComboBox::lineEditDeleted()
{
    // yes, we need those ugly casts due to the multiple inheritance
    // sender() is guaranteed to be a KLineEdit (see the connect() to the
    // destroyed() signal
    const KCompletionBase *base = static_cast<const KCompletionBase*>( static_cast<const KLineEdit*>( sender() ));

    // is it our delegate, that is destroyed?
    if ( base == delegate() )
        setDelegate( 0L );
}


#include "kstcombobox.moc"
// vim: ts=4 sw=4 et
