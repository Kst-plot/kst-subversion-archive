/***************************************************************************
                          displayCompare.cpp  -  description
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

#include "displaycompare.h"

// Qt
#include <qlayout.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>

// KDE
#include <ksqueezedtextlabel.h>
#include <kapplication.h>
#include <kglobal.h>

DisplayCompare::DisplayCompare(QWidget* parent, QDict < QPtrVector < QFile > >* cmp )
:KDialog(parent,"DisplayCompare",true)
{
	this->cmp=cmp;

    CompareLayout = new QVBoxLayout( this, 11, 6, "CompareLayout");

    layout3 = new QHBoxLayout( 0, 0, 6, "layout3");

    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, GroupBox2->sizePolicy().hasHeightForWidth() ) );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 6 );
    GroupBox2->layout()->setMargin( 11 );
    GroupBox2Layout = new QVBoxLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );

    OriginalNameLabel = new KSqueezedTextLabel( GroupBox2, "OriginalNameLabel" );
    OriginalNameLabel->setFrameShape( QLabel::Box );
    OriginalNameLabel->setAlignment( int( QLabel::AlignCenter ) );
    GroupBox2Layout->addWidget( OriginalNameLabel );


    layout3_2 = new QHBoxLayout( 0, 0, 6, "layout3_2");
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3_2->addItem( spacer );

    preview1 = new QLabel( GroupBox2, "preview1" );
    preview1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, preview1->sizePolicy().hasHeightForWidth() ) );
    preview1->setMinimumSize( QSize( 120, 120 ) );
    preview1->setScaledContents( true );
    layout3_2->addWidget( preview1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 31, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3_2->addItem( spacer_2 );
    GroupBox2Layout->addLayout( layout3_2 );
    layout3->addWidget( GroupBox2 );

    originalInfoLabel = new KSqueezedTextLabel( GroupBox2, "originalInfoLabel" );
    originalInfoLabel->setAlignment( int( QLabel::AlignCenter ) );
    GroupBox2Layout->addWidget( originalInfoLabel );

    GroupBox2_2 = new QGroupBox( this, "GroupBox2_2" );
    GroupBox2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)3, 0, 0, GroupBox2_2->sizePolicy().hasHeightForWidth() ) );
    GroupBox2_2->setFrameShape( QGroupBox::GroupBoxPanel );
    GroupBox2_2->setFrameShadow( QGroupBox::Sunken );
    GroupBox2_2->setColumnLayout(0, Qt::Vertical );
    GroupBox2_2->layout()->setSpacing( 6 );
    GroupBox2_2->layout()->setMargin( 11 );
    GroupBox2_2Layout = new QVBoxLayout( GroupBox2_2->layout() );
    GroupBox2_2Layout->setAlignment( Qt::AlignTop );

    similarNameLabel = new KSqueezedTextLabel( GroupBox2_2, "similarNameLabel" );
    similarNameLabel->setFrameShape( QLabel::Box );
    similarNameLabel->setAlignment( int( QLabel::AlignCenter ) );
    GroupBox2_2Layout->addWidget( similarNameLabel );


    layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    QSpacerItem* spacer_3 = new QSpacerItem( 21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout4->addItem( spacer_3 );

    preview2 = new QLabel( GroupBox2_2, "preview2" );
    preview2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, preview2->sizePolicy().hasHeightForWidth() ) );
    preview2->setMinimumSize( QSize( 120, 120 ) );
    preview2->setScaledContents( true );
    layout4->addWidget( preview2 );
    QSpacerItem* spacer_4 = new QSpacerItem( 31, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout4->addItem( spacer_4 );
    GroupBox2_2Layout->addLayout( layout4 );
    layout3->addWidget( GroupBox2_2 );
    CompareLayout->addLayout( layout3 );

    similarInfoLabel = new KSqueezedTextLabel( GroupBox2_2, "similarInfoLabel" );
    similarInfoLabel->setAlignment( int( QLabel::AlignCenter ) );
    GroupBox2_2Layout->addWidget( similarInfoLabel );


    GroupBox8 = new QGroupBox( this, "GroupBox8" );
    GroupBox8->setColumnLayout(0, Qt::Vertical );
    GroupBox8->layout()->setSpacing( 6 );
    GroupBox8->layout()->setMargin( 11 );
    GroupBox8Layout = new QHBoxLayout( GroupBox8->layout() );
    GroupBox8Layout->setAlignment( Qt::AlignTop );

    listName = new QListView( GroupBox8, "listName" );
    listName->addColumn( i18n( "Files" ) );
    GroupBox8Layout->addWidget( listName );

    listEq = new QListView( GroupBox8, "listEq" );
    listEq->addColumn( i18n( "Identical To" ) );
    GroupBox8Layout->addWidget( listEq );
    CompareLayout->addWidget( GroupBox8 );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2");
    QSpacerItem* spacer_5 = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer_5 );

    delettePushButton = new QPushButton( this, "delettePushButton" );
    layout2->addWidget( delettePushButton );

    closePushButton = new QPushButton( this, "closePushButton" );
    layout2->addWidget( closePushButton );
    CompareLayout->addLayout( layout2 );
    languageChange();

    // signals and slots connections
    connect( closePushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

    // tab order
    setTabOrder( listName, listEq );
    setTabOrder( listEq, delettePushButton );
    setTabOrder( delettePushButton, closePushButton );

	///////////////////////
	///////////////////////
	///////////////////////
	connect( delettePushButton, SIGNAL( clicked() ), this, SLOT( suppression() ) );
	////
	QDictIterator < QPtrVector < QFile > >itres (*cmp);	// iterator for res
 	cmp->setAutoDelete(true);
	int n_id=0;
	while (itres.current ())
	{
		listName->insertItem ( new QCheckListItem(listName, itres.currentKey (), QCheckListItem::CheckBox));
		++itres;
		n_id++;
	}
	GroupBox8->setTitle(i18n("Found %n Image","Found %n Images",n_id));
	connect(listName, SIGNAL(selectionChanged ( QListViewItem * )), this, SLOT(slotDisplayLeft(QListViewItem *)));
	connect(listEq, SIGNAL(selectionChanged ( QListViewItem * )), this, SLOT(slotDisplayRight(QListViewItem *)));

    	resize( QSize(445, 466).expandedTo(minimumSizeHint()) );
	listName->setSelected(listName->firstChild(), true);
}


DisplayCompare::~DisplayCompare()
{
}

void
DisplayCompare::accept()
{
	done(true);
}

void
DisplayCompare::reject()
{
	done(false);
}

void
DisplayCompare::suppression()
{
	QCheckListItem* item=(QCheckListItem*)listEq->firstChild ();
	QCheckListItem* itemTmp;
	while(item)
	{
		if(item->isOn ())
		{
			itemTmp=(QCheckListItem*)item->nextSibling();
			QFile::remove (item->text(0) );
			listEq->takeItem (item);

			item=itemTmp;
		}
		else
			item=(QCheckListItem*)item->nextSibling();
	}

	///
	for (QCheckListItem* item = (QCheckListItem*)listName->firstChild (); item; item=(QCheckListItem*)item->nextSibling())
	{
		if(item->isOn ())
		{
			QFile::remove (item->text(0) );

			item-> setOn ( false );
		}
	}
}


void
DisplayCompare::slotDisplayLeft(QListViewItem * item)
{
	KApplication::setOverrideCursor( waitCursor );

	listEq->clear();
	QPtrVector < QFile > *list = (QPtrVector < QFile > *)cmp->find(item->text(0));
	QImage im = QImage(item->text(0));
	if(!im.isNull())
	{
		OriginalNameLabel->setText(item->text(0));
		originalInfoLabel->setText(i18n("%1x%2, %3 b, %4")
						.arg(im.width())
						.arg(im.height())
						.arg(QFileInfo(item->text(0)).size())
						.arg(KGlobal::locale()->formatDateTime(QFileInfo(item->text(0)).lastModified()))
					);

		im = im.smoothScale(preview1->width(), preview1->height());
		QPixmap pix;
		pix.convertFromImage(im);
		preview1->setPixmap(pix);
	}
	else
		preview1->setPixmap(NULL);

	QCheckListItem *it, *last=NULL;
	QFile *f=NULL;
	QFileInfo *fi=new QFileInfo();
	QString fn;

	for (unsigned int i = 0; i < list->size (); i++)
	{
	      f=(QFile*)list->at(i);
	      fi->setFile(*f);
	      fn =  fi->absFilePath ();
	      if(fi->exists ())
	      {
	      	it=new QCheckListItem(listEq,fn, QCheckListItem::CheckBox);
	      	listEq->insertItem (it);
	      	if(!last)
			  last=it;
	      }
	}
	preview2->setPixmap(QPixmap());
	listEq->setSelected(last, true);

	KApplication::restoreOverrideCursor();
}


void
DisplayCompare::slotDisplayRight(QListViewItem * item)
{
	KApplication::setOverrideCursor( waitCursor );

	QImage im = QImage(item->text(0));
	if(!im.isNull())
	{
		similarNameLabel->setText(item->text(0));
		similarInfoLabel->setText(i18n("%1x%2, %3 b, %4")
						.arg(im.width())
						.arg(im.height())
						.arg(QFileInfo(item->text(0)).size())
						.arg(KGlobal::locale()->formatDateTime(QFileInfo(item->text(0)).lastModified()))
					);


		im = im.smoothScale(preview2->width(), preview2->height());
		QPixmap pix;
		pix.convertFromImage(im);
		preview2->setPixmap(pix);
	}
	else
		preview2->setPixmap(NULL);

	KApplication::restoreOverrideCursor();
    }




/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void
DisplayCompare::languageChange()
{
    setCaption( i18n( "Comparison" ) );
    GroupBox2->setTitle( i18n( "Preview" ) );
    //OriginalNameLabel->setText( i18n( "textLabel1" ) );
    //originalInfoLabel->setText( i18n( "textLabel2" ) );
    GroupBox2_2->setTitle( i18n( "Preview" ) );
    //similarNameLabel->setText( i18n( "textLabel3" ) );
    //similarInfoLabel->setText( i18n( "textLabel4" ) );
    GroupBox8->setTitle( i18n( "Identical Files" ) );
    listName->header()->setLabel( 0, i18n( "Files" ) );
    //QWhatsThis::add( listName, i18n( "Contient la liste des fihiers\n"
//"ayant plusieurs occurences" ) );
    listEq->header()->setLabel( 0, i18n( "Identical to" ) );
    //QWhatsThis::add( listEq, i18n( "Fichiers identiques a celui de droite" ) );
    delettePushButton->setText( i18n( "Delete" ) );
    closePushButton->setText( i18n( "&Close" ) );
}

QSize
DisplayCompare::sizeHint () const
{
	return QSize(445, 466);
}

#include "displaycompare.moc"
