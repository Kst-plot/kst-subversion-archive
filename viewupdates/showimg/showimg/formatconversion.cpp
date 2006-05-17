/***************************************************************************
                          formatconversion.cpp  -  description
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

#include "formatconversion.h"

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
#include <qimage.h>
#include <qpixmap.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qwhatsthis.h>


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
    listViewFormat->header()->setClickEnabled( false, listViewFormat->header()->count() - 1 );
    listViewFormat->header()->setResizeEnabled( false, listViewFormat->header()->count() - 1 );
    listViewFormat->addColumn( i18n( "Description" ) );
    listViewFormat->header()->setClickEnabled( false, listViewFormat->header()->count() - 1 );
    listViewFormat->header()->setResizeEnabled( false, listViewFormat->header()->count() - 1 );
    listViewFormat->setAllColumnsShowFocus( true );

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
	item->setText( 1, ( "CompuServe Graphics Interchange Format" ) );

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
	listViewFormat->setAllColumnsShowFocus( true );
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
	if(type == QString::fromLatin1("JPG"))
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

QSize
FormatConversion::sizeHint () const
{
	return QSize(371, 295);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



JPGOptions::JPGOptions( QWidget* parent,  const char* name )
:KDialogBase( parent, name, true, "JPGOptions" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::Default, KDialogBase::Ok, true )
{
    //setCaption( i18n( "Describe ")+imagefile);
    QWidget *page = new QWidget( this );
    setMainWidget(page);

///
    JPGOptionsLayout = new QVBoxLayout( page, 11, 6, "JPGOptionsLayout");

    GroupBox13 = new QGroupBox( page, "GroupBox13" );
    GroupBox13->setColumnLayout(0, Qt::Vertical );
    GroupBox13->layout()->setSpacing( 6 );
    GroupBox13->layout()->setMargin( 11 );
    GroupBox13Layout = new QVBoxLayout( GroupBox13->layout() );
    GroupBox13Layout->setAlignment( Qt::AlignTop );

    layout6 = new QHBoxLayout( 0, 0, 6, "layout6");
    Spacer1 = new QSpacerItem( 79, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
    layout6->addItem( Spacer1 );

    TextLabel4 = new QLabel( GroupBox13, "TextLabel4" );
    TextLabel4->setAlignment( int( QLabel::AlignCenter ) );
    layout6->addWidget( TextLabel4 );

    qualitySslider1 = new QSlider( GroupBox13, "qualitySslider1" );
    qualitySslider1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, qualitySslider1->sizePolicy().hasHeightForWidth() ) );
    qualitySslider1->setMinimumSize( QSize( 180, 0 ) );
    qualitySslider1->setMaximumSize( QSize( 200, 32767 ) );
    qualitySslider1->setMaxValue( 100 );
    qualitySslider1->setValue( 75 );
    qualitySslider1->setOrientation( QSlider::Horizontal );
    qualitySslider1->setTickmarks( QSlider::Above );
    qualitySslider1->setTickInterval( 0 );
    layout6->addWidget( qualitySslider1 );

    TextLabel5 = new QLabel( GroupBox13, "TextLabel5" );
    TextLabel5->setAlignment( int( QLabel::AlignCenter ) );
    layout6->addWidget( TextLabel5 );
    Spacer2 = new QSpacerItem( 78, 20, QSizePolicy::Preferred, QSizePolicy::Minimum );
    layout6->addItem( Spacer2 );
    GroupBox13Layout->addLayout( layout6 );

    layout7 = new QHBoxLayout( 0, 0, 6, "layout7");
    spacer6 = new QSpacerItem( 121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout7->addItem( spacer6 );

    SpinBox1 = new QSpinBox( GroupBox13, "SpinBox1" );
    SpinBox1->setButtonSymbols( QSpinBox::UpDownArrows );
    SpinBox1->setMaxValue( 100 );
    SpinBox1->setValue( 75 );
    layout7->addWidget( SpinBox1 );
    spacer7 = new QSpacerItem( 121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout7->addItem( spacer7 );
    GroupBox13Layout->addLayout( layout7 );
    JPGOptionsLayout->addWidget( GroupBox13 );
    spacer8 = new QSpacerItem( 16, 28, QSizePolicy::Minimum, QSizePolicy::Expanding );
    JPGOptionsLayout->addItem( spacer8 );

    Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2");

    progressiveCheckBox = new QCheckBox( page, "progressiveCheckBox" );
    Layout2->addWidget( progressiveCheckBox );

    TextLabel6 = new QLabel( page, "TextLabel6" );
    TextLabel6->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    Layout2->addWidget( TextLabel6 );

    samplingComboBox = new QComboBox( false, page, "samplingComboBox" );
    Layout2->addWidget( samplingComboBox );
    JPGOptionsLayout->addLayout( Layout2 );

    layout5 = new QHBoxLayout( 0, 0, 6, "layout5");
    spacer5 = new QSpacerItem( 131, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout5->addItem( spacer5 );

    TextLabel7 = new QLabel( page, "TextLabel7" );
    TextLabel7->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout5->addWidget( TextLabel7 );

    smootingSpinBox = new QSpinBox( page, "smootingSpinBox" );
    layout5->addWidget( smootingSpinBox );
    JPGOptionsLayout->addLayout( layout5 );
    spacer9 = new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    JPGOptionsLayout->addItem( spacer9 );

    Line1 = new QFrame( page, "Line1" );
    Line1->setFrameShape( QFrame::HLine );
    Line1->setFrameShadow( QFrame::Sunken );
    Line1->setFrameShape( QFrame::HLine );
    JPGOptionsLayout->addWidget( Line1 );

    saveCheckBox = new QCheckBox( page, "saveCheckBox" );
    JPGOptionsLayout->addWidget( saveCheckBox );
    //languageChange();
    clearWState( WState_Polished );

    // signals and slots connections
    connect( qualitySslider1, SIGNAL( sliderMoved(int) ), SpinBox1, SLOT( setValue(int) ) );
    connect( SpinBox1, SIGNAL( valueChanged(int) ), qualitySslider1, SLOT( setValue(int) ) );

    // tab order
    setTabOrder( qualitySslider1, SpinBox1 );
    setTabOrder( SpinBox1, progressiveCheckBox );
    setTabOrder( progressiveCheckBox, samplingComboBox );
    setTabOrder( samplingComboBox, smootingSpinBox );
    setTabOrder( smootingSpinBox, saveCheckBox );

///
    GroupBox13->setTitle( i18n( "Image Quality" ) );
    TextLabel4->setText( i18n( "Best\n"
" compression" ) );
    QWhatsThis::add( qualitySslider1, i18n( "Choose the quality\n"
"of the converted picture" ) );
    TextLabel5->setText( i18n( "Best\n"
" quality" ) );
    progressiveCheckBox->setText( i18n( "&Progressive" ) );
    QWhatsThis::add( progressiveCheckBox, i18n( "Have to use the\n"
" progressive algorithm?" ) );
    TextLabel6->setText( i18n( "Component\n"
" sampling:" ) );
    samplingComboBox->clear();
    samplingComboBox->insertItem( i18n( "YUV 122 (default)" ) );
    samplingComboBox->insertItem( i18n( "GRAY (to grayscale)" ) );
    samplingComboBox->insertItem( i18n( "CMYK" ) );
    TextLabel7->setText( i18n( "Smoothing:" ) );
    QWhatsThis::add( TextLabel7, i18n( "Choose how to\n"
"smooth the image" ) );
    saveCheckBox->setText( i18n( "&Save the settings as the default" ) );
    //saveCheckBox->setAccel( QKeySequence( i18n( "Alt+S" ) ) );
    QWhatsThis::add( saveCheckBox, i18n( "Save these parameters\n"
"for the next times" ) );


///


	slotDefault();
}

/*
 *  Destroys the object and frees any allocated resources
 */
JPGOptions::~JPGOptions()
{
	// no need to delete child widgets, Qt does it all for us
}

void JPGOptions::slotDefault()
{
	SpinBox1->setValue( 75 );

	smootingSpinBox->setValue(1);
	samplingComboBox->setCurrentItem (0);
	progressiveCheckBox->setChecked(false);
	samplingComboBox->setCurrentItem(0);
}

QString JPGOptions::getOptions()
{
	QString opt;
	opt = QString("-quality ") + QString().setNum(SpinBox1->value());
	if (progressiveCheckBox->isChecked ())
		opt+=" -interlace Plane";
	if(smootingSpinBox->value()!=1)
		opt+=" -blur "+ QString().setNum(smootingSpinBox->value());
	if(samplingComboBox->currentItem()==0)
		opt+=" -colorspace YUV";
	else
	if(samplingComboBox->currentItem()==1)
		opt+=" -colorspace GRAY";
	else
	if(samplingComboBox->currentItem()==2)
		opt+=" -colorspace CMYK";
	return opt+" ";
}

QSize
JPGOptions::sizeHint () const
{
	return QSize(366, 247);
}



#include "formatconversion.moc"
