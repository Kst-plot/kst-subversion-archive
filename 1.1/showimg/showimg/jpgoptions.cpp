
/***************************************************************************
                          jpgoptions.cpp  -  description
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

#include "jpgoptions.h"

// QT
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// KDE
#include <kapplication.h>
#include <klocale.h>

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

    samplingComboBox = new QComboBox( FALSE, page, "samplingComboBox" );
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
    resize( QSize(366, 247).expandedTo(minimumSizeHint()) );
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


#include "jpgoptions.moc"
