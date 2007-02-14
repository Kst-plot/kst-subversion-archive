/***************************************************************************
                         describe.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#include "describe.h"

// Qt
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfileinfo.h>

// KDE
#include <klocale.h>
#include <klineedit.h>
#include <ktextedit.h>


Describe::Describe( QWidget* parent,  const QString& imagefile, const char* name )
:KDialogBase( parent, name, false, "Describe" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::Apply, KDialogBase::Ok, true )
{

    setCaption( i18n( "Describe %1").arg(imagefile));
    QWidget *page = new QWidget( this );
    setMainWidget(page);

///
    DescribeLayout = new QVBoxLayout( page, 11, 6, "DescribeLayout");

    layout1 = new QHBoxLayout( 0, 0, 6, "layout1");

    textLabel5 = new QLabel( page, "textLabel5" );
    layout1->addWidget( textLabel5 );

    title = new KLineEdit( page, "title" );
    layout1->addWidget( title );
    DescribeLayout->addLayout( layout1 );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2");

    groupBox2 = new QGroupBox( page, "groupBox2" );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    groupBox2Layout = new QVBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    layout3 = new QGridLayout( 0, 1, 1, 0, 6, "layout3");

    textLabel3 = new QLabel( groupBox2, "textLabel3" );

    layout3->addWidget( textLabel3, 2, 0 );

    event = new KLineEdit( groupBox2, "event" );
    event->setMinimumSize( QSize( 100, 0 ) );

    layout3->addWidget( event, 0, 1 );

    people = new KLineEdit( groupBox2, "people" );
    people->setMinimumSize( QSize( 100, 0 ) );

    layout3->addWidget( people, 2, 1 );

    textLabel4 = new QLabel( groupBox2, "textLabel4" );

    layout3->addWidget( textLabel4, 3, 0 );

    textLabel1 = new QLabel( groupBox2, "textLabel1" );

    layout3->addWidget( textLabel1, 0, 0 );

    location = new KLineEdit( groupBox2, "location" );
    location->setMinimumSize( QSize( 100, 0 ) );

    layout3->addWidget( location, 1, 1 );

    date = new KLineEdit( groupBox2, "date" );
    date->setMinimumSize( QSize( 100, 0 ) );

    layout3->addWidget( date, 3, 1 );

    textLabel2 = new QLabel( groupBox2, "textLabel2" );

    layout3->addWidget( textLabel2, 1, 0 );
    groupBox2Layout->addLayout( layout3 );
    layout2->addWidget( groupBox2 );

    groupBox3 = new QGroupBox( page, "groupBox3" );
    groupBox3->setColumnLayout(0, Qt::Vertical );
    groupBox3->layout()->setSpacing( 6 );
    groupBox3->layout()->setMargin( 11 );
    groupBox3Layout = new QHBoxLayout( groupBox3->layout() );
    groupBox3Layout->setAlignment( Qt::AlignTop );

    longDescr = new KTextEdit( groupBox3, "longDescr" );
    groupBox3Layout->addWidget( longDescr );
    layout2->addWidget( groupBox3 );
    DescribeLayout->addLayout( layout2 );
    clearWState( WState_Polished );
///
    textLabel5->setText( i18n( "Title:" ) );
    QToolTip::add( title, i18n( "Type a short title for the picture" ) );
    groupBox2->setTitle( i18n( "Information" ) );
    textLabel3->setText( i18n( "People:" ) );
    QToolTip::add( event, i18n( "The event where the image was taken" ) );
    QToolTip::add( people, i18n( "The names of the people on the picture" ) );
    QWhatsThis::add( people, i18n( "Should be a comma separated list without the word \"and\" to allow for easy parsing for a future search feature. For example: Colin, Mike, Steph, Jeff, Marc" ) );
    textLabel4->setText( i18n( "Date:" ) );
    textLabel1->setText( i18n( "Event:" ) );
    QToolTip::add( location, i18n( "The location where the image was taken" ) );
    QToolTip::add( date, i18n( "The date and time the picture was taken" ) );
    textLabel2->setText( i18n( "Location:" ) );
    groupBox3->setTitle( i18n( "Describe" ) );
    QToolTip::add( longDescr, i18n( "A description of the picture and any other information" ) );


///
	setImageFile(imagefile);
	title->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
Describe::~Describe()
{
	// no need to delete child widgets, Qt does it all for us
}

void
Describe::setImageFile(const QString& imagefile)
{
	setCaption(imagefile);
	iinfo = new ImageFileInfo(imagefile, 0);
	title->setText(iinfo->getTitle());
	event->setText(iinfo->getEvent());
	location->setText(iinfo->getLocation());
	people->setText(iinfo->getPeople());
	date->setText(iinfo->getDate());
	longDescr->setText(iinfo->getDescription());
}


void
Describe::slotOk()
{
	iinfo->write(title->text(),event->text(),
			location->text(),people->text(),
			date->text(), longDescr->text()  );
	KDialogBase::slotOk();
	emit close();
}

void
Describe::slotCancel()
{
	KDialogBase::slotCancel();
	emit close();
}

void
Describe::slotApply()
{
	iinfo->write(title->text(),event->text(),
			location->text(),people->text(),
			date->text(), longDescr->text()  );
	KDialogBase::slotApply();
}

QSize
Describe::sizeHint () const
{
	return QSize(461, 224);
}



#include "describe.moc"
