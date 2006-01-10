/***************************************************************************
                         categoryproperties.cpp  -  description
                             -------------------
    begin                : fri 10 jun 2005
    copyright            : (C) 2005 by Richard Groult
    email                : rgroult@jalix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include "categoryproperties.h"

#ifdef WANT_LIBKEXIDB

#include "categorylistitem.h"

#include <kicondialog.h>
#include <ktextedit.h>
#include <klineedit.h>
#include <klocale.h>

#include <qvariant.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "
/*
 *  Constructs a CategoryProperties as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CategoryProperties::CategoryProperties( QWidget* parent, CategoryListItemTag* categoryListItemTag)
    : KDialogBase( parent, "CategoryProperties", true, "Describe" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
{
    //setCaption(tr("CategoryProperties %1").arg(dirname));
    QWidget *page = new QWidget( this );
    setMainWidget(page);

    m_categoryListItemTag = categoryListItemTag;


/////////////
//    if ( !name )
	setName( "Category Properties" );
    CategoryPropertiesLayout = new QGridLayout( page, 1, 1, 11, 6, "CategoryPropertiesLayout");

    iconButton = new KIconButton( page, "iconButton" );
    iconButton->setMinimumSize( QSize( 60, 60 ) );
    iconButton->setMaximumSize( QSize( 50, 50 ) );
    iconButton->setOn( false );
    iconButton->setIconSize( 48 );

    CategoryPropertiesLayout->addMultiCellWidget( iconButton, 0, 1, 0, 0 );
    spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
    CategoryPropertiesLayout->addMultiCell( spacer2, 0, 1, 1, 1 );

    commentTextEdit = new KTextEdit( page, "commentTextEdit" );

    CategoryPropertiesLayout->addMultiCellWidget( commentTextEdit, 4, 4, 0, 2 );

    line1 = new QFrame( page, "line1" );
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::HLine );

    CategoryPropertiesLayout->addMultiCellWidget( line1, 2, 2, 0, 2 );

    nameLineEdit = new KLineEdit( page, "nameLineEdit" );

    CategoryPropertiesLayout->addWidget( nameLineEdit, 1, 2 );

    nametextLabel = new QLabel( page, "nametextLabel" );

    CategoryPropertiesLayout->addWidget( nametextLabel, 0, 2 );

    describeTextLabel = new QLabel( page, "describeTextLabel" );

    CategoryPropertiesLayout->addMultiCellWidget( describeTextLabel, 3, 3, 0, 2 );
    languageChange();
    clearWState( WState_Polished );


    //////////////////////////////////////
    setCaption( i18n( "Category properties: %1" ).arg(m_categoryListItemTag->name()) );
    nameLineEdit->setText(m_categoryListItemTag->name());
    commentTextEdit->setText(m_categoryListItemTag->getDescription());
    iconButton->setIcon(m_categoryListItemTag->getIcon());
}

/*
 *  Destroys the object and frees any allocated resources
 */
CategoryProperties::~CategoryProperties()
{
    // no need to delete child widgets, Qt does it all for us
}

const QString
CategoryProperties::getName()const
{
	return nameLineEdit->text();
}
const QString
CategoryProperties::getDescription()const
{
	return commentTextEdit->text();
}

const QString
CategoryProperties::getIcon()const
{
	return iconButton->icon();
}



/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CategoryProperties::languageChange()
{
//    setCaption( i18n( "Form2" ) );
    iconButton->setText( QString::null );
    nametextLabel->setText( i18n( "Name:" ) );
    describeTextLabel->setText( i18n( "Describe:" ) );
}

QSize
CategoryProperties::sizeHint () const
{
	return QSize(262, 276);
}

#include "categoryproperties.moc"
#endif /* #ifdef WANT_LIBKEXIDB */

