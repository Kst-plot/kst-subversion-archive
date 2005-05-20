/***************************************************************************
                          formatconversion.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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

#include "formatconversion.h"

// Local
#include "jpgoptions.h"

// Qt
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>



FormatConversion::FormatConversion( QWidget* parent,  const char* name )
:KDialogBase( parent, name, true, "FormatConversion" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )

{
    QWidget *page = new QWidget( this );
    setMainWidget(page);
//////////////////////////////
    FormatConversionLayout = new QGridLayout( page, 1, 1, 11, 6, "FormatConversionLayout"); 

    GroupBox1 = new QGroupBox( page, "GroupBox1" );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 6 );
    GroupBox1->layout()->setMargin( 11 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );

    listViewFormat = new QListView( GroupBox1, "listViewFormat" );
    listViewFormat->addColumn( i18n( "Extension" ) );
    listViewFormat->header()->setClickEnabled( FALSE, listViewFormat->header()->count() - 1 );
    listViewFormat->header()->setResizeEnabled( FALSE, listViewFormat->header()->count() - 1 );
    listViewFormat->addColumn( i18n( "Description" ) );
    listViewFormat->header()->setClickEnabled( FALSE, listViewFormat->header()->count() - 1 );
    listViewFormat->header()->setResizeEnabled( FALSE, listViewFormat->header()->count() - 1 );
    listViewFormat->setAllColumnsShowFocus( TRUE );

    GroupBox1Layout->addMultiCellWidget( listViewFormat, 0, 0, 0, 1 );
    spacer4 = new QSpacerItem( 180, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox1Layout->addItem( spacer4, 1, 0 );

    pushButtonSetting = new QPushButton( GroupBox1, "pushButtonSetting" );
    pushButtonSetting->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, pushButtonSetting->sizePolicy().hasHeightForWidth() ) );

    GroupBox1Layout->addWidget( pushButtonSetting, 1, 1 );

    CheckBox1 = new QCheckBox( GroupBox1, "CheckBox1" );

    GroupBox1Layout->addMultiCellWidget( CheckBox1, 3, 3, 0, 1 );

    line1 = new QFrame( GroupBox1, "line1" );
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::HLine );

    GroupBox1Layout->addMultiCellWidget( line1, 2, 2, 0, 1 );

    FormatConversionLayout->addWidget( GroupBox1, 0, 0 );
    //languageChange();
    resize( QSize(371, 295).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( listViewFormat, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( enabledDisabledSettingButton(QListViewItem*) ) );

    // tab order
    setTabOrder( listViewFormat, pushButtonSetting );

//////////////////////////////

	QListViewItem * item = new QListViewItem( listViewFormat, 0 );
	item->setText( 0,  "BMP"  );
	item->setText( 1, ( "Microsoft Windows Bitmap image file" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "GIF"  );
	item->setText( 1, ( "CompuServe Graphics Interchange format" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "JPG"  );
	item->setText( 1, ( "Joint Photographic Experts Group JFIF format" ) );
	listViewFormat->setCurrentItem (item);

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "PNG"  );
	item->setText( 1, ( "Portable Network Graphics" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "TGA"  );
	item->setText( 1, ( "Truevision Targa image file" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "TIF"  );
	item->setText( 1, ( "Tagged Image File Format" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "XPM"  );
	item->setText( 1, ( "X Windows system pixmap file" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "PS"  );
	item->setText( 1, ( "Adobe PostScript file" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "EPS"  );
	item->setText( 1, ( "Adobe Encapsulated PostScript file" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "PDF"  );
	item->setText( 1, ( "Portable Document Format" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "PCX"  );
	item->setText( 1, ( "ZSoft IBM PC Paintbrush file" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "ICO"  );
	item->setText( 1, ( "Microsoft icon" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "PNM"  );
	item->setText( 1, ( "Portable anymap" ) );

	item = new QListViewItem( listViewFormat, item );
	item->setText( 0,  "MNG"  );
	item->setText( 1, ( "Multiple-image Network Graphics" ) );

	listViewFormat->setGeometry( QRect( 70, 20, 270, 160 ) );
	listViewFormat->setAllColumnsShowFocus( TRUE );
	QWhatsThis::add(  listViewFormat, i18n( "Choose your format here" ) );


    QWhatsThis::add( listViewFormat, i18n( "Choose your format here" ) );
    GroupBox1->setTitle( i18n( "Destination Format" ) );
    pushButtonSetting->setText( i18n( "Format &Setting..." ) );
    QWhatsThis::add( pushButtonSetting, i18n( "Edit the setting of\n"
"the selected format" ) );
    CheckBox1->setText( i18n( "Remove/replace original" ) );
    QWhatsThis::add( CheckBox1, i18n( "Have to replace the original?" ) );

	////
	options=QString(" ");
	connect( listViewFormat, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( enabledDisabledSettingButton(QListViewItem*) ) );
	connect( pushButtonSetting, SIGNAL( clicked() ), this, SLOT( showJPGOption() ) );

	opt=NULL;
}

/*
 *  Destroys the object and frees any allocated resources
 */
FormatConversion::~FormatConversion()
{
	// no need to delete child widgets, Qt does it all for us
}
QString FormatConversion::getOptions()
{
	return options;
}

QString FormatConversion::getType()
{
	return listViewFormat->currentItem()->text(0).lower();
}

bool FormatConversion::replace()
{
	return CheckBox1->isChecked();
}

void FormatConversion::enabledDisabledSettingButton(QListViewItem* item)
{
	QString type=item->text(0);
	options="";
	if(type=="JPG")
		pushButtonSetting->setEnabled(true);
	else
		pushButtonSetting->setEnabled(false);
}
void FormatConversion::showJPGOption()
{
	if(!opt) opt = new JPGOptions(this);
	switch(opt->exec())
	{
		case KDialogBase::Accepted : options = opt->getOptions();
		default:;
	}
}

#include "formatconversion.moc"
