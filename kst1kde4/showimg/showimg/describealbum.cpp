/***************************************************************************
                          describeAlbum.cpp -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groulti
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

#include "describealbum.h"

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


DescribeAlbum::DescribeAlbum( QWidget* parent, const QString& dirname,  const char* name )
:KDialogBase( parent, name, true, "Describe" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
{
    setCaption(i18n("Describe %1").arg(dirname));
    QWidget *page = new QWidget( this );
    setMainWidget(page);


/////////////
    if ( !name )
		setName( "Describe Album" );
    DescribeAlbumLayout = new QVBoxLayout( page, 11, 6, "DescribeAlbumLayout");

    DescribeAlbumlayout1 = new QHBoxLayout( 0, 0, 6, "DescribeAlbumlayout1");

    textLabel1 = new QLabel( page, "textLabel1" );
    DescribeAlbumlayout1->addWidget( textLabel1 );

    title = new KLineEdit( page, "title" );
    DescribeAlbumlayout1->addWidget( title );
    DescribeAlbumLayout->addLayout( DescribeAlbumlayout1 );

    DescribeAlbumgroupBox1 = new QGroupBox( page, "DescribeAlbumgroupBox1" );
    DescribeAlbumgroupBox1->setColumnLayout(0, Qt::Vertical );
    DescribeAlbumgroupBox1->layout()->setSpacing( 6 );
    DescribeAlbumgroupBox1->layout()->setMargin( 11 );
    DescribeAlbumgroupBox1Layout = new QVBoxLayout( DescribeAlbumgroupBox1->layout() );
    DescribeAlbumgroupBox1Layout->setAlignment( Qt::AlignTop );

    textLabel2 = new QLabel( DescribeAlbumgroupBox1, "textLabel2" );
    DescribeAlbumgroupBox1Layout->addWidget( textLabel2 );

    shortDescr = new KLineEdit( DescribeAlbumgroupBox1, "shortDescr" );
    DescribeAlbumgroupBox1Layout->addWidget( shortDescr );

    textLabel2_2 = new QLabel( DescribeAlbumgroupBox1, "textLabel2_2" );
    DescribeAlbumgroupBox1Layout->addWidget( textLabel2_2 );

    longDescr = new KTextEdit( DescribeAlbumgroupBox1, "longDescr" );
    DescribeAlbumgroupBox1Layout->addWidget( longDescr );
    DescribeAlbumLayout->addWidget( DescribeAlbumgroupBox1 );
    clearWState( WState_Polished );


/////////////
    textLabel1->setText( i18n( "Title:" ) );
    QToolTip::add( title, i18n( "A short title for the album" ) );
    DescribeAlbumgroupBox1->setTitle( i18n( "Information" ) );
    textLabel2->setText( i18n( "Short description:" ) );
    QToolTip::add( shortDescr, i18n( "A short description of the album's contents" ) );
    textLabel2_2->setText( i18n( "Long description:" ) );
    QToolTip::add( longDescr, i18n( "A longer description of the album's contents" ) );





/////////////
	QFileInfo finfo = QFileInfo(dirname);
	QString filename= finfo.absFilePath()+'/'+"album.txt";

	iinfo = new ImageFileInfo(filename, 1);
	title->setText(iinfo->getTitle());
	shortDescr->setText(iinfo->getShortDescription());
	longDescr->setText(iinfo->getLongDescription());

	title->setFocus();

}

/*
 *  Destroys the object and frees any allocated resources
 */
DescribeAlbum::~DescribeAlbum()
{
	// no need to delete child widgets, Qt does it all for us
}

void
DescribeAlbum::slotOk()
{
	iinfo->write(title->text(),
			shortDescr->text(),longDescr->text());
	KDialogBase::accept();
}

QSize
DescribeAlbum::sizeHint () const
{
	return QSize(257, 303);
}


#include "describealbum.moc"
