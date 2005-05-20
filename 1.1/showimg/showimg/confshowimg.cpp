/***************************************************************************
                          confshowimg.cpp -  description
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

#include "confshowimg.h"

#include <fixx11h.h>
#ifdef HAVE_KIPI
#include <libkipi/version.h>
#endif


// Qt
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <qbuttongroup.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE
#include <kcolorbutton.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpixmapeffect.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <klistview.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kfontrequester.h>

#define CONFIG_ICON_SIZE 32

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "


ConfShowImg::ConfShowImg(QWidget *parent)
:KDialogBase(IconList/*Tabbed*/, i18n("Configure showimg"),
		Help|Ok|Cancel, Ok,
		parent, "Configure showimg", true)
{
	addPage1();
	addPage2();
	addPage9();
	addPage11();
#ifdef HAVE_KIPI
	addPage8();
#endif /* HAVE_KIPI */
	addPage7();
	addPage6();
	addPage3();
	addPage10();
	addPage4();
	addPage5();

	setHelp("configure.anchor", "showimg");

	resize( QSize(0, 0).expandedTo(minimumSizeHint()) )  ;
// 	clearWState( WState_Polished );
}

ConfShowImg::~ConfShowImg()
{
}


void
ConfShowImg::initFiling(int openType, const QString& openDir, bool showSP, bool startFS)
{
	if(openType==0) openHome->setChecked( TRUE );
	else if(openType==1) openLast->setChecked( TRUE );
	else open_custom->setChecked( TRUE );
	LineEdit2->setText(openDir);
	this->showSP->setChecked(showSP);
	this->startFS->setChecked(startFS);
}


int
ConfShowImg::getOpenDirType()
{
	if(openHome->isChecked()) return 0;
	else if(openLast->isChecked()) return 1;
	else return 2;
}
QString
ConfShowImg::getOpenDir()
{
	return LineEdit2->text();
}
bool
ConfShowImg::checkshowSP()
{
	return showSP->isChecked();
}
bool
ConfShowImg::checkstartFS()
{
	return startFS->isChecked();
}



void
ConfShowImg::addPage1()
{
	page1 = addPage( i18n("Startup"),
			 i18n("Startup"),
	    		 BarIcon("fileopen", CONFIG_ICON_SIZE));


    page1Layout = new QVBoxLayout( page1, 11, 6, "page1Layout");

    ///////
    GroupBox13 = new QButtonGroup( page1, "GroupBox13" );
    GroupBox13->setColumnLayout(0, Qt::Vertical );
    GroupBox13->layout()->setSpacing( 6 );
    GroupBox13->layout()->setMargin( 11 );
    GroupBox13Layout = new QVBoxLayout( GroupBox13->layout() );
    GroupBox13Layout->setAlignment( Qt::AlignTop );

    openHome = new QRadioButton( GroupBox13, "openHome" );
    openHome->setChecked( TRUE );
    GroupBox13Layout->addWidget( openHome );

    openLast = new QRadioButton( GroupBox13, "openLast" );
    GroupBox13Layout->addWidget( openLast );

    open_custom = new QRadioButton( GroupBox13, "open_custom" );
    GroupBox13Layout->addWidget( open_custom );

    layout1_2 = new QHBoxLayout( 0, 0, 6, "layout1_2");

    LineEdit2 = new QLineEdit( GroupBox13, "LineEdit2" );
    LineEdit2->setEnabled( FALSE );
    LineEdit2->setEdited( FALSE );
    layout1_2->addWidget( LineEdit2 );

    chooseButton = new QPushButton( GroupBox13, "chooseButton" );
    chooseButton->setMaximumSize( QSize( 30, 30 ) );
    layout1_2->addWidget( chooseButton );
    GroupBox13Layout->addLayout( layout1_2 );
    page1Layout->addWidget( GroupBox13 );

    showSP = new QCheckBox( page1, "showSP" );
    showSP->setChecked( TRUE );
    page1Layout->addWidget( showSP );

    startFS = new QCheckBox( page1, "startFS" );
    page1Layout->addWidget( startFS );
    QSpacerItem* spacer = new QSpacerItem( 20, 70, QSizePolicy::Minimum, QSizePolicy::Expanding );
    page1Layout->addItem( spacer );


//////////////
//////////////
//////////////

    GroupBox13->setTitle( i18n( "On Starting Open" ) );
    openHome->setText( i18n( "&Home directory" ) );
    openLast->setText( i18n( "&Last directory" ) );
    open_custom->setText( i18n( "&Specified directory:" ) );
    //chooseButton->setText( i18n( "PushButton1" ) );
    showSP->setText( i18n( "Show s&plash screen" ) );
    startFS->setText( i18n( "Start in &fullscreen mode" ) );

    QToolTip::add( showSP, i18n( "Show the ShowImg splashscreen at startup" ) );
    QToolTip::add( startFS, i18n( "Start ShowImg in fullscreen mode when it is launched with an image file name." ) );

    chooseButton->setPixmap( QPixmap(BarIcon("folder_open",16 )));
    chooseButton->setDisabled(true);

	    // signals and slots connections
	connect( open_custom, SIGNAL( toggled(bool) ), LineEdit2, SLOT( setEnabled(bool) ) );
	connect( open_custom, SIGNAL( toggled(bool) ), chooseButton, SLOT( setEnabled(bool) ) );
	connect( chooseButton, SIGNAL( clicked()), this, SLOT( chooseDir() ));

}
void
ConfShowImg::chooseDir()
{
	QString s=KFileDialog::getExistingDirectory(LineEdit2->text(),
								this,
								i18n("Specified Directory"));
 	 if(!s.isEmpty())
 		LineEdit2->setText( s );
}


void
ConfShowImg::initMiscellaneous(bool smooth, bool loadfim,
                               bool sHDir, bool sHFile, bool sDir, bool sAll,
                               bool sprelodim)
{
	smoothCheck->setChecked(smooth);
	sHiddenDirCheck ->setChecked(sHDir);
	sHiddenFileCheck ->setChecked(sHFile);
	sDirCheck ->setChecked(sDir);
	sAllCheck ->setChecked(sAll);
	prelodimCheck ->setChecked(sprelodim);
	loadfirstimCheck->setChecked(loadfim);
}

bool
ConfShowImg::getSmooth()
{
	return smoothCheck->isChecked();
}
bool
ConfShowImg::getShowHiddenDir()
{
	return sHiddenDirCheck->isChecked();
}
bool
ConfShowImg::getShowHiddenFile()
{
	return sHiddenFileCheck->isChecked();
}
bool
ConfShowImg::getShowDir()
{
	return sDirCheck->isChecked();
}
bool
ConfShowImg::getShowAll()
{
	return sAllCheck->isChecked();
}
bool
ConfShowImg::getPreloadIm()
{
	return prelodimCheck->isChecked();
}
bool
ConfShowImg::getLoadFirstImage()
{
	return loadfirstimCheck->isChecked();
}

void
ConfShowImg::addPage2()
{
	page2 = addPage( i18n("Miscellaneous"),
			 i18n("Miscellaneous"),
	    		 BarIcon("misc", CONFIG_ICON_SIZE));

    page2Layout = new QVBoxLayout( page2, 11, 6, "page2Layout");

    layout6 = new QHBoxLayout( 0, 0, 6, "layout6");

    groupBox3 = new QGroupBox( page2, "groupBox3" );
    groupBox3->setColumnLayout(0, Qt::Vertical );
    groupBox3->layout()->setSpacing( 6 );
    groupBox3->layout()->setMargin( 11 );
    groupBox3Layout = new QGridLayout( groupBox3->layout() );
    groupBox3Layout->setAlignment( Qt::AlignTop );

    smoothCheck = new QCheckBox( groupBox3, "smoothCheck" );

    groupBox3Layout->addWidget( smoothCheck, 0, 0 );
    layout6->addWidget( groupBox3 );

    groupBox4 = new QGroupBox( page2, "groupBox4" );
    groupBox4->setColumnLayout(0, Qt::Vertical );
    groupBox4->layout()->setSpacing( 6 );
    groupBox4->layout()->setMargin( 11 );
    groupBox4Layout = new QVBoxLayout( groupBox4->layout() );
    groupBox4Layout->setAlignment( Qt::AlignTop );

    prelodimCheck = new QCheckBox( groupBox4, "prelodimCheck" );
    groupBox4Layout->addWidget( prelodimCheck );

    loadfirstimCheck = new QCheckBox( groupBox4, "loadfirstimCheck" );
    groupBox4Layout->addWidget( loadfirstimCheck );
    layout6->addWidget( groupBox4 );
    page2Layout->addLayout( layout6 );

    groupBox5 = new QGroupBox( page2, "groupBox5" );
    groupBox5->setColumnLayout(0, Qt::Vertical );
    groupBox5->layout()->setSpacing( 6 );
    groupBox5->layout()->setMargin( 11 );
    groupBox5Layout = new QGridLayout( groupBox5->layout() );
    groupBox5Layout->setAlignment( Qt::AlignTop );

    sHiddenDirCheck = new QCheckBox( groupBox5, "sHiddenDirCheck" );

    groupBox5Layout->addWidget( sHiddenDirCheck, 0, 0 );

    sHiddenFileCheck = new QCheckBox( groupBox5, "sHiddenFileCheck" );

    groupBox5Layout->addWidget( sHiddenFileCheck, 1, 0 );

    sAllCheck = new QCheckBox( groupBox5, "sAllCheck" );

    groupBox5Layout->addWidget( sAllCheck, 0, 1 );

    sDirCheck = new QCheckBox( groupBox5, "sDirCheck" );

    groupBox5Layout->addWidget( sDirCheck, 1, 1 );
    page2Layout->addWidget( groupBox5 );


    ///////
    groupBox3->setTitle( i18n( "Zoom Mode" ) );
    smoothCheck->setText( i18n( "Smooth &scale" ) );
    QToolTip::add( smoothCheck, i18n( "Better quality but slower and requires more memory" ) );
    groupBox4->setTitle( i18n( "Preloading" ) );
    prelodimCheck->setText( i18n( "Preload next image" ) );
    loadfirstimCheck->setText( i18n( "Load the first image" ) );
    QToolTip::add( loadfirstimCheck, i18n( "Load the first image of the selected directory" ) );
    groupBox5->setTitle( i18n( "Files && Directories" ) );
    sHiddenDirCheck->setText( i18n( "Show hidden &directories" ) );
    sHiddenFileCheck->setText( i18n( "Show hidden &files" ) );
    sAllCheck->setText( i18n( "Show all &files" ) );
    sDirCheck->setText( i18n( "Show &directories" ) );

    //
    page2Layout->addItem( new QSpacerItem( 20, 70, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}

void
ConfShowImg::initColor(const QColor bgcolor, int gray)
{
	color->setColor(bgcolor);
	gray = (gray<50?50:gray);
	graySlider->setValue((int)(gray));
	setGrayscale(gray);
}

QColor
ConfShowImg::getColor()
{
	return color->color();
}

int
ConfShowImg::getGrayscale()
{
	return graySlider->value();
}

void
ConfShowImg::addPage3()
{
	page3 = addPage( i18n("Colors"),
			 i18n("Colors"),
	    		 BarIcon("colorize", CONFIG_ICON_SIZE));

///
    ColorsLayout = new QVBoxLayout( page3, 11, 6, "ColorsLayout");

    colorButtonGroup2 = new QButtonGroup( page3, "colorButtonGroup2" );
    colorButtonGroup2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, colorButtonGroup2->sizePolicy().hasHeightForWidth() ) );
    colorButtonGroup2->setColumnLayout(0, Qt::Vertical );
    colorButtonGroup2->layout()->setSpacing( 6 );
    colorButtonGroup2->layout()->setMargin( 11 );
    colorButtonGroup2Layout = new QGridLayout( colorButtonGroup2->layout() );
    colorButtonGroup2Layout->setAlignment( Qt::AlignTop );

    ColorRadioButton5 = new QRadioButton( colorButtonGroup2, "ColorRadioButton5" );
    ColorRadioButton5->setEnabled( FALSE );

    colorButtonGroup2Layout->addWidget( ColorRadioButton5, 0, 2 );

    PushButton1 = new QPushButton( colorButtonGroup2, "PushButton1" );
    PushButton1->setEnabled( FALSE );

    colorButtonGroup2Layout->addWidget( PushButton1, 1, 2 );

    RadioButton4 = new QRadioButton( colorButtonGroup2, "RadioButton4" );
    RadioButton4->setChecked( TRUE );

    colorButtonGroup2Layout->addMultiCellWidget( RadioButton4, 0, 0, 0, 1 );

    color = new KColorButton( colorButtonGroup2, "color" );
    color->setFlat( TRUE );

    colorButtonGroup2Layout->addWidget( color, 1, 0 );
    colorspacer1 = new QSpacerItem( 31, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    colorButtonGroup2Layout->addItem( colorspacer1, 1, 1 );
    ColorsLayout->addWidget( colorButtonGroup2 );

    colorGroupBox6 = new QGroupBox( page3, "colorGroupBox6" );
    colorGroupBox6->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, colorGroupBox6->sizePolicy().hasHeightForWidth() ) );
    colorGroupBox6->setColumnLayout(0, Qt::Vertical );
    colorGroupBox6->layout()->setSpacing( 6 );
    colorGroupBox6->layout()->setMargin( 11 );
    colorGroupBox6Layout = new QGridLayout( colorGroupBox6->layout() );
    colorGroupBox6Layout->setAlignment( Qt::AlignTop );

    PixmapLabel1 = new QLabel( colorGroupBox6, "PixmapLabel1" );
    PixmapLabel1->setMaximumSize( QSize( 100, 100 ) );
    //PixmapLabel1->setPixmap( image0 );
    PixmapLabel1->setScaledContents( TRUE );

    colorGroupBox6Layout->addMultiCellWidget( PixmapLabel1, 0, 1, 1, 1 );

    graySlider = new QSlider( colorGroupBox6, "graySlider" );
    graySlider->setMinValue( 30 );
    graySlider->setMaxValue( 100 );
    graySlider->setLineStep( 10 );
    graySlider->setValue( 30 );
    graySlider->setTracking( FALSE );
    graySlider->setOrientation( QSlider::Horizontal );
    graySlider->setTickmarks( QSlider::Both );

    colorGroupBox6Layout->addWidget( graySlider, 1, 0 );
    spacer3 = new QSpacerItem( 20, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
    colorGroupBox6Layout->addItem( spacer3, 0, 0 );
    ColorsLayout->addWidget( colorGroupBox6 );
    colorspacer2 = new QSpacerItem( 20, 61, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ColorsLayout->addItem( colorspacer2 );
    //languageChange();
//     page3->resize( QSize(268, 261).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( RadioButton4, SIGNAL( toggled(bool) ), color, SLOT( setEnabled(bool) ) );
    connect( ColorRadioButton5, SIGNAL( toggled(bool) ), PushButton1, SLOT( setEnabled(bool) ) );
    connect( graySlider, SIGNAL( valueChanged(int) ), this, SLOT( setGrayscale(int) ) );

    // tab order
    setTabOrder( RadioButton4, color );
    setTabOrder( color, PushButton1 );
///
    colorButtonGroup2->setTitle( i18n( "Background" ) );
    ColorRadioButton5->setText( i18n( "Tiled image:" ) );
    PushButton1->setText( i18n( "Choose..." ) );
    RadioButton4->setText( i18n( "Color:" ) );
    color->setText( QString::null );
    colorGroupBox6->setTitle( i18n( "Grayscale" ) );

///
    image0 = new QPixmap(locate("appdata", "pics/gradient.png"));
    PixmapLabel1->setPixmap( *image0 );
}

void
ConfShowImg::initSlideshow(int type, int time)
{
	if(type==0)
		forward->setChecked( TRUE );
	else
	if(type==1)
		backward->setChecked( TRUE );
	else
		random->setChecked( TRUE );

	timeSlide->setValue(time);
}

int
ConfShowImg::getSlideshowType()
{
	if(forward->isChecked())
		return 0;
	else
	if(backward->isChecked())
		return 1;
	else
		return 2;
}

int
ConfShowImg::getSlideshowTime()
{
	return timeSlide->value();
}

void
ConfShowImg::addPage4()
{
	page4 = addPage( i18n("Slide Show"),
			 i18n("Slide Show"),
	    		 BarIcon("run", CONFIG_ICON_SIZE));

//
    SlideShowLayout = new QVBoxLayout( page4, 11, 6, "SlideShowLayout");

    layout9 = new QHBoxLayout( 0, 0, 6, "layout9");

    ButtonGroup3 = new QButtonGroup( page4, "ButtonGroup3" );
    ButtonGroup3->setColumnLayout(0, Qt::Vertical );
    ButtonGroup3->layout()->setSpacing( 6 );
    ButtonGroup3->layout()->setMargin( 11 );
    ButtonGroup3Layout = new QVBoxLayout( ButtonGroup3->layout() );
    ButtonGroup3Layout->setAlignment( Qt::AlignTop );

    forward = new QRadioButton( ButtonGroup3, "forward" );
    forward->setChecked( TRUE );
    ButtonGroup3->insert( forward, 0 );
    ButtonGroup3Layout->addWidget( forward );

    backward = new QRadioButton( ButtonGroup3, "backward" );
    ButtonGroup3->insert( backward, 1 );
    ButtonGroup3Layout->addWidget( backward );

    random = new QRadioButton( ButtonGroup3, "random" );
    ButtonGroup3->insert( random, 2 );
    ButtonGroup3Layout->addWidget( random );

    Line1 = new QFrame( ButtonGroup3, "Line1" );
    Line1->setFrameShape( QFrame::HLine );
    Line1->setFrameShadow( QFrame::Sunken );
    Line1->setFrameShape( QFrame::HLine );
    ButtonGroup3Layout->addWidget( Line1 );

    wraparound = new QCheckBox( ButtonGroup3, "wraparound" );
    ButtonGroup3Layout->addWidget( wraparound );
    layout9->addWidget( ButtonGroup3 );

    GroupBox2 = new QGroupBox( page4, "GroupBox2" );
    GroupBox2->setColumnLayout(0, Qt::Vertical );
    GroupBox2->layout()->setSpacing( 6 );
    GroupBox2->layout()->setMargin( 11 );
    GroupBox2Layout = new QVBoxLayout( GroupBox2->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );

    timeSlide = new QSlider( GroupBox2, "timeSlide" );
    timeSlide->setOrientation( QSlider::Horizontal );
    GroupBox2Layout->addWidget( timeSlide );

    layout1 = new QHBoxLayout( 0, 0, 6, "layout1");

    timeLabel = new QLabel( GroupBox2, "timeLabel" );
    timeLabel->setAlignment( int( QLabel::AlignCenter ) );
    layout1->addWidget( timeLabel );
    GroupBox2Layout->addLayout( layout1 );
    layout9->addWidget( GroupBox2 );
    SlideShowLayout->addLayout( layout9 );
    spacer16 = new QSpacerItem( 20, 51, QSizePolicy::Minimum, QSizePolicy::Expanding );
    SlideShowLayout->addItem( spacer16 );
    //languageChange();
//     page4->resize( QSize(553, 218).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( timeSlide, SIGNAL( valueChanged(int) ), this, SLOT( slotChangeTime(int) ) );

    // tab order
    setTabOrder( forward, wraparound );
    setTabOrder( wraparound, timeSlide );
    setTabOrder( timeSlide, backward );
///
    ButtonGroup3->setTitle( i18n( "Sequence" ) );
    forward->setText( i18n( "&Forward" ) );
    //forward->setAccel( QKeySequence( i18n( "Alt+F" ) ) );
    backward->setText( i18n( "&Backward" ) );
    //backward->setAccel( QKeySequence( i18n( "Alt+B" ) ) );
    random->setText( i18n( "&Random" ) );
    //random->setAccel( QKeySequence( i18n( "Alt+R" ) ) );
    wraparound->setText( i18n( "Wrap around" ) );
    GroupBox2->setTitle( i18n( "Timed Slide Show" ) );
}


void
ConfShowImg::slotChangeTime(int)
{
	timeLabel->setText( i18n("%n second", "%n seconds", timeSlide->value()) );
}

int
ConfShowImg::getLayout()
{
	if(radioButton_1->isChecked())
		return 1;
	else
	if(radioButton_2->isChecked())
		return 2;
	else
	if(radioButton_3->isChecked())
		return 3;
	else
	if(radioButton_4->isChecked())
		return 4;
	else
		return 5;
}

void
ConfShowImg::setLayout(int l)
{
	if(l==1)
		radioButton_1->setChecked(true);
	else
	if(l==2)
		radioButton_2->setChecked(true);
	else
	if(l==3)
		radioButton_3->setChecked(true);
	else
	if(l==4)
		radioButton_4->setChecked(true);
	else
		RadioButton5->setChecked(true);
}

void
ConfShowImg::addPage5()
{
	page5 = addPage( i18n("Layout"),
			 i18n("Layout"),
			 BarIcon("view_choose", CONFIG_ICON_SIZE));

	QPixmap image0(locate("appdata", "pics/layout1.png"));
	QPixmap image1(locate("appdata", "pics/layout2.png"));
	QPixmap image2(locate("appdata", "pics/layout3.png"));
	QPixmap image3(locate("appdata", "pics/layout4.png"));

///
    Form2Layout = new QVBoxLayout( page5, 11, 6, "Form2Layout");
    layout10 = new QHBoxLayout( 0, 0, 6, "layout10");
    ButtonGroup2 = new QButtonGroup( page5, "ButtonGroup2" );

///
    ButtonGroup2->setColumnLayout(0, Qt::Vertical );
    ButtonGroup2->layout()->setSpacing( 6 );
    ButtonGroup2->layout()->setMargin( 11 );
    ButtonGroup2Layout = new QGridLayout( ButtonGroup2->layout() );
    ButtonGroup2Layout->setAlignment( Qt::AlignTop );

    radioButton_4 = new QRadioButton( ButtonGroup2, "radioButton_4" );
    radioButton_4->setPixmap( image0 );

    ButtonGroup2Layout->addWidget( radioButton_4, 2, 1 );

    radioButton_1 = new QRadioButton( ButtonGroup2, "radioButton_1" );
    radioButton_1->setPixmap( image1 );

    ButtonGroup2Layout->addWidget( radioButton_1, 0, 0 );

    radioButton_3 = new QRadioButton( ButtonGroup2, "radioButton_3" );
    radioButton_3->setPixmap( image2 );

    ButtonGroup2Layout->addWidget( radioButton_3, 2, 0 );
    spacer11 = new QSpacerItem( 20, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ButtonGroup2Layout->addItem( spacer11, 1, 0 );

    radioButton_2 = new QRadioButton( ButtonGroup2, "radioButton_2" );
    radioButton_2->setPixmap( image3 );

    ButtonGroup2Layout->addWidget( radioButton_2, 0, 1 );
    spacer12 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ButtonGroup2Layout->addItem( spacer12, 1, 1 );
    spacer10 = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ButtonGroup2Layout->addItem( spacer10, 4, 0 );

    RadioButton5 = new QRadioButton( ButtonGroup2, "RadioButton5" );
    RadioButton5->setChecked( TRUE );

    ButtonGroup2Layout->addWidget( RadioButton5, 4, 1 );
    spacer13 = new QSpacerItem( 20, 21, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ButtonGroup2Layout->addItem( spacer13, 3, 0 );
    spacer14 = new QSpacerItem( 20, 31, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ButtonGroup2Layout->addItem( spacer14, 3, 1 );
    layout10->addWidget( ButtonGroup2 );
    spacer17 = new QSpacerItem( 51, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout10->addItem( spacer17 );
    Form2Layout->addLayout( layout10 );
    spacer15 = new QSpacerItem( 20, 61, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Form2Layout->addItem( spacer15 );
    //languageChange();
//     page5->resize( QSize(303, 292).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );
///
    ButtonGroup2->setTitle( i18n( "Layout" ) );
    radioButton_4->setText( QString::null );
    radioButton_1->setText( QString::null );
    radioButton_3->setText( QString::null );
    radioButton_2->setText( QString::null );
    RadioButton5->setText( i18n( "&Current" ) );
    //RadioButton5->setAccel( QKeySequence( i18n( "Alt+C" ) ) );
}



void
ConfShowImg::setGrayscale(int val)
{
	KPixmap pix(*image0);
	float taux=((float)val)/100;
	PixmapLabel1->setPixmap(KPixmapEffect::desaturate( pix, taux));
}

void
ConfShowImg::initFullscreen(bool showToolbar, bool showStatusbar)
{
	sToolbar->setChecked( showToolbar );
	sStatusbar->setChecked( showStatusbar );
}

bool
ConfShowImg::getShowToolbar()
{
	return sToolbar->isChecked();
}
bool
ConfShowImg::getShowStatusbar()
{
	return sStatusbar->isChecked();
}

void
ConfShowImg::initOSD(bool dispOSD, bool onTop, const QFont& font, bool sFilename, bool sFullpath, bool sDimensions, bool sComments, bool sDatetime, bool sExif)
{
	sOSDcheckBox->setChecked( dispOSD );
	if(onTop)
		osdTopRadioButton->setChecked( TRUE );
	else
		osdBottomRadioButton->setChecked( TRUE );
	osdFontRequester->setFont(font);
	osdOptFilenameCheckBox->setChecked(sFilename);
	osdOptFullpathCheckBox->setChecked(sFullpath);
	osdOptDimensionsCheckBox->setChecked(sDimensions);
	osdOptCommentsCheckBox->setChecked(sComments);
	osdOptDatetimeCheckBox->setChecked(sDatetime);
	osdOptExifCheckBox->setChecked(sExif);
}

bool
ConfShowImg::getShowOSD()
{
	return sOSDcheckBox->isChecked();
}
bool
ConfShowImg::getOSDOnTop()
{
	return osdTopRadioButton->isChecked();
}
QFont
ConfShowImg::getOSDFont()
{
	return osdFontRequester->font();
}
bool
ConfShowImg::getOSDShowFilename()
{
	return osdOptFilenameCheckBox->isChecked();
}
bool
ConfShowImg::getOSDShowFullpath()
{
	return osdOptFullpathCheckBox->isChecked();
}
bool
ConfShowImg::getOSDShowDimensions()
{
	return osdOptDimensionsCheckBox->isChecked();
}
bool
ConfShowImg::getOSDShowComments()
{
	return osdOptCommentsCheckBox->isChecked();
}
bool
ConfShowImg::getOSDShowDatetime()
{
	return osdOptDatetimeCheckBox->isChecked();
}
bool
ConfShowImg::getOSDShowExif()
{
	return osdOptExifCheckBox->isChecked();
}

void
ConfShowImg::addPage6()
{
	page6 = addPage( i18n("Full Screen"),
			 i18n("Full Screen"),
			 BarIcon("window_fullscreen", CONFIG_ICON_SIZE));
	/////////////
    FullScreenLayoutLayout = new QVBoxLayout( page6, 11, 6, "FullScreenLayoutLayout");

    fsGroupBox = new QGroupBox( page6, "fsGroupBox" );
    fsGroupBox->setColumnLayout(0, Qt::Vertical );
    fsGroupBox->layout()->setSpacing( 6 );
    fsGroupBox->layout()->setMargin( 11 );
    fsGroupBoxLayout = new QVBoxLayout( fsGroupBox->layout() );
    fsGroupBoxLayout->setAlignment( Qt::AlignTop );

    sToolbar = new QCheckBox( fsGroupBox, "sToolbar" );
    fsGroupBoxLayout->addWidget( sToolbar );

    sStatusbar = new QCheckBox( fsGroupBox, "sStatusbar" );
    fsGroupBoxLayout->addWidget( sStatusbar );
    FullScreenLayoutLayout->addWidget( fsGroupBox );

    osdGroupBox = new QGroupBox( page6, "osdGroupBox" );
    osdGroupBox->setColumnLayout(0, Qt::Vertical );
    osdGroupBox->layout()->setSpacing( 6 );
    osdGroupBox->layout()->setMargin( 11 );
    osdGroupBoxLayout = new QVBoxLayout( osdGroupBox->layout() );
    osdGroupBoxLayout->setAlignment( Qt::AlignTop );

    osdcbfclayout = new QHBoxLayout( 0, 0, 6, "osdcbfclayout");

    sOSDcheckBox = new QCheckBox( osdGroupBox, "sOSDcheckBox" );
    sOSDcheckBox->setChecked( TRUE );
    osdcbfclayout->addWidget( sOSDcheckBox );

    osdFontRequester = new KFontRequester( osdGroupBox, "osdFontRequester" );
    osdcbfclayout->addWidget( osdFontRequester );
    osdGroupBoxLayout->addLayout( osdcbfclayout );

    osdTimegroupBox = new QGroupBox( osdGroupBox, "osdTimegroupBox" );
    osdTimegroupBox->setColumnLayout(0, Qt::Vertical );
    osdTimegroupBox->layout()->setSpacing( 6 );
    osdTimegroupBox->layout()->setMargin( 11 );
    osdTimegroupBoxLayout = new QVBoxLayout( osdTimegroupBox->layout() );
    osdTimegroupBoxLayout->setAlignment( Qt::AlignTop );

    osdTimeSlider = new QSlider( osdTimegroupBox, "osdTimeSlider" );
    osdTimeSlider->setMinValue( 1 );
    osdTimeSlider->setMaxValue( 60 );
    osdTimeSlider->setLineStep( 1 );
    osdTimeSlider->setValue( 10 );
    osdTimeSlider->setOrientation( QSlider::Horizontal );
    osdTimeSlider->setTickmarks( QSlider::Below );
    osdTimeSlider->setTickInterval( 5 );
    osdTimegroupBoxLayout->addWidget( osdTimeSlider );

    osdTimeTextLabel = new QLabel( osdTimegroupBox, "osdTimeTextLabel" );
    osdTimeTextLabel->setAlignment( int( QLabel::AlignCenter ) );
    osdTimegroupBoxLayout->addWidget( osdTimeTextLabel );
    osdGroupBoxLayout->addWidget( osdTimegroupBox );

    osdPosbuttonGroup = new QButtonGroup( osdGroupBox, "osdPosbuttonGroup" );
    osdPosbuttonGroup->setColumnLayout(0, Qt::Vertical );
    osdPosbuttonGroup->layout()->setSpacing( 6 );
    osdPosbuttonGroup->layout()->setMargin( 11 );
    osdPosbuttonGroupLayout = new QHBoxLayout( osdPosbuttonGroup->layout() );
    osdPosbuttonGroupLayout->setAlignment( Qt::AlignTop );

    osdTopRadioButton = new QRadioButton( osdPosbuttonGroup, "osdTopRadioButton" );
    osdTopRadioButton->setChecked( TRUE );
    osdPosbuttonGroupLayout->addWidget( osdTopRadioButton );

    osdBottomRadioButton = new QRadioButton( osdPosbuttonGroup, "osdBottomRadioButton" );
    osdPosbuttonGroupLayout->addWidget( osdBottomRadioButton );
    osdGroupBoxLayout->addWidget( osdPosbuttonGroup );

    osdOptionsGroupBox = new QGroupBox( osdGroupBox, "osdOptionsGroupBox" );
    osdOptionsGroupBox->setColumnLayout(0, Qt::Vertical );
    osdOptionsGroupBox->layout()->setSpacing( 6 );
    osdOptionsGroupBox->layout()->setMargin( 11 );
    osdOptionsGroupBoxLayout = new QGridLayout( osdOptionsGroupBox->layout() );
    osdOptionsGroupBoxLayout->setAlignment( Qt::AlignTop );

    osdOptFilenameCheckBox = new QCheckBox( osdOptionsGroupBox, "osdOptFilenameCheckBox" );

    osdOptionsGroupBoxLayout->addWidget( osdOptFilenameCheckBox, 0, 0 );

    osdOptFullpathCheckBox = new QCheckBox( osdOptionsGroupBox, "osdOptFullpathCheckBox" );

    osdOptionsGroupBoxLayout->addWidget( osdOptFullpathCheckBox, 1, 0 );

    osdOptDimensionsCheckBox = new QCheckBox( osdOptionsGroupBox, "osdOptDimensionsCheckBox" );

    osdOptionsGroupBoxLayout->addWidget( osdOptDimensionsCheckBox, 2, 0 );

    osdOptExifCheckBox = new QCheckBox( osdOptionsGroupBox, "osdOptExifCheckBox" );

    osdOptionsGroupBoxLayout->addWidget( osdOptExifCheckBox, 2, 1 );

    osdOptCommentsCheckBox = new QCheckBox( osdOptionsGroupBox, "osdOptCommentsCheckBox" );

    osdOptionsGroupBoxLayout->addWidget( osdOptCommentsCheckBox, 0, 1 );

    osdOptDatetimeCheckBox = new QCheckBox( osdOptionsGroupBox, "osdOptDatetimeCheckBox" );

    osdOptionsGroupBoxLayout->addWidget( osdOptDatetimeCheckBox, 1, 1 );
    osdGroupBoxLayout->addWidget( osdOptionsGroupBox );
    FullScreenLayoutLayout->addWidget( osdGroupBox );
    osdSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    FullScreenLayoutLayout->addItem( osdSpacer );
//     languageChange();
//     resize( QSize(307, 432).expandedTo(minimumSizeHint()) );
//     clearWState( WState_Polished );

    // signals and slots connections
    connect( sOSDcheckBox, SIGNAL( toggled(bool) ), osdOptionsGroupBox, SLOT( setEnabled(bool) ) );
    connect( sOSDcheckBox, SIGNAL( toggled(bool) ), osdPosbuttonGroup, SLOT( setEnabled(bool) ) );
    connect( sOSDcheckBox, SIGNAL( toggled(bool) ), osdFontRequester, SLOT( setEnabled(bool) ) );
    connect( sOSDcheckBox, SIGNAL( toggled(bool) ), osdTimegroupBox, SLOT( setEnabled(bool) ) );
    connect( osdTimeSlider, SIGNAL( sliderMoved(int) ), osdTimeTextLabel, SLOT( setNum(int) ) );

	//////////////
    fsGroupBox->setTitle( i18n ("Full Screen" ) );
    sToolbar->setText( i18n ("Show &toolbar" ) );
    sStatusbar->setText( i18n ("Show &status bar" ) );
    osdGroupBox->setTitle( i18n ("OSD" ) );
    sOSDcheckBox->setText( i18n ("Display &OSD" ) );
    osdTimegroupBox->setTitle( i18n ("Time" ) );
    osdTimeTextLabel->setText( i18n ("Number of Seconds" ) );
    osdPosbuttonGroup->setTitle( i18n ("Position" ) );
    osdTopRadioButton->setText( i18n ("To&p" ) );
    osdBottomRadioButton->setText( i18n ("Botto&m" ) );
    osdOptionsGroupBox->setTitle( i18n ("Options" ) );
    osdOptFilenameCheckBox->setText( i18n ("File &Name" ) );
    osdOptFullpathCheckBox->setText( i18n ("File &Full Path" ) );
    osdOptDimensionsCheckBox->setText( i18n ("&Dimensions" ) );
    osdOptExifCheckBox->setText( i18n ("&Exif Data" ) );
    osdOptCommentsCheckBox->setText( i18n ("&Comments" ) );
    osdOptDatetimeCheckBox->setText( i18n ("Date/&Time" ) );

}


void
ConfShowImg::initProperties(bool sMeta, bool sHexa)
{
	showMeta->setChecked(sMeta);
	showHexa->setChecked(sHexa);
}


bool
ConfShowImg::getShowMeta()
{
	return showMeta->isChecked();
}
bool
ConfShowImg::getShowHexa()
{
	return showHexa->isChecked();
}


void
ConfShowImg::addPage7()
{
	page7 = addPage( i18n("Properties"),
			 i18n("Properties"),
			 BarIcon("properties", CONFIG_ICON_SIZE));


    page7Layout = new QHBoxLayout( page7, 11, 6, "page7Layout");

    layoutTab = new QVBoxLayout( 0, 0, 6, "layoutTab");

    groupBoxTab = new QGroupBox( page7, "groupBoxTab" );
    groupBoxTab->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, groupBoxTab->sizePolicy().hasHeightForWidth() ) );
    groupBoxTab->setColumnLayout(0, Qt::Vertical );
    groupBoxTab->layout()->setSpacing( 6 );
    groupBoxTab->layout()->setMargin( 11 );
    groupBoxTabLayout = new QGridLayout( groupBoxTab->layout() );
    groupBoxTabLayout->setAlignment( Qt::AlignTop );

    layoutCheckBoxTab = new QVBoxLayout( 0, 0, 6, "layoutCheckBoxTab");

    showMeta = new QCheckBox( groupBoxTab, "showMeta" );
    layoutCheckBoxTab->addWidget( showMeta );

    showHexa = new QCheckBox( groupBoxTab, "showHexa" );
    layoutCheckBoxTab->addWidget( showHexa );

    groupBoxTabLayout->addLayout( layoutCheckBoxTab, 0, 0 );
    layoutTab->addWidget( groupBoxTab );
    QSpacerItem* spacer = new QSpacerItem( 20, 51, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layoutTab->addItem( spacer );
    page7Layout->addLayout( layoutTab );

    groupBoxTab->setTitle( i18n( "Show/Hide Tabs" ) );
    showMeta->setText( i18n( "Show &meta-data tab" ) );
    showHexa->setText( i18n( "Show &hexadecimal tab" ) );

}


void
ConfShowImg::applyPlugins()
{
#ifdef HAVE_KIPI
    m_Kipiconfig->apply();
#endif
}

void
ConfShowImg::addPage8()
{
	page8 = addPage( i18n("Plugins"),
					 i18n("Plugins Kipi - version %1").arg(kipi_version),
			 BarIcon("kipi", CONFIG_ICON_SIZE));

    Form1Layoutp8 = new QVBoxLayout( page8, 11, 6, "Form1Layout");
#ifdef HAVE_KIPI
   m_Kipiconfig = KIPI::PluginLoader::instance()->configWidget( page8 );
   QString pluginsListHelp = i18n("<p>Here you can see the list of plugins who can be "
                                  "loaded or unloaded from the current ShowImg instance.");

   QWhatsThis::add( m_Kipiconfig, pluginsListHelp);
   Form1Layoutp8->addWidget( m_Kipiconfig );
#endif
}


bool
ConfShowImg::getStoreth()
{
	return storethCheck->isChecked();
}
bool
ConfShowImg::getShowFrame()
{
	return showFrame->isChecked();
}
bool
ConfShowImg::getUseEXIF()
{
	return useEXIF->isChecked();
}
bool
ConfShowImg::getWordWrapIconText()
{
	return wrapIconText->isChecked();
}
bool
ConfShowImg::getShowMimeType()
{
	return showMimeType->isChecked();
}
bool
ConfShowImg::getShowSize()
{
	return showSize->isChecked();
}
bool
ConfShowImg::getShowDate()
{
	return showDate->isChecked();
}
bool
ConfShowImg::getShowDimension()
{
	return showDimension->isChecked();
}
bool
ConfShowImg::getShowTooltips()
{
	return showTooltip->isChecked();
}

void
ConfShowImg::initThumbnails(bool storeth,  bool showf, bool useexif, bool wwt,
				bool showMT, bool showS, bool showD, bool showDim, bool showTool)
{
	storethCheck->setChecked(storeth);
	showFrame->setChecked(showf);
	useEXIF->setChecked(useexif);
	wrapIconText->setChecked(wwt);

	showMimeType->setChecked(showMT);
	showSize->setChecked(showS);
	showDate->setChecked(showD);
	showDimension->setChecked(showDim);
	showTooltip->setChecked(showTool);
}

void
ConfShowImg::addPage9()
{
	page9 = addPage( i18n("Thumbnails"),
			 i18n("Thumbnails"),
			 BarIcon("thumbnail", CONFIG_ICON_SIZE));
//////
    thumbConfigWidgetLayout = new QVBoxLayout( page9, 11, 6, "thumbConfigWidgetLayout");

    layoutThumb = new QHBoxLayout( 0, 0, 6, "layoutThumb");

    groupboxThumbnails = new QGroupBox( page9, "groupboxThumbnails" );
    groupboxThumbnails->setColumnLayout(0, Qt::Vertical );
    groupboxThumbnails->layout()->setSpacing( 6 );
    groupboxThumbnails->layout()->setMargin( 11 );
    groupboxThumbnailsLayout = new QGridLayout( groupboxThumbnails->layout() );
    groupboxThumbnailsLayout->setAlignment( Qt::AlignTop );

    showFrame = new QCheckBox( groupboxThumbnails, "showFrame" );

    groupboxThumbnailsLayout->addWidget( showFrame, 0, 1 );

    storethCheck = new QCheckBox( groupboxThumbnails, "storethCheck" );
    storethCheck->setTristate( TRUE );

    groupboxThumbnailsLayout->addWidget( storethCheck, 0, 0 );

    useEXIF = new QCheckBox( groupboxThumbnails, "useEXIF" );

    groupboxThumbnailsLayout->addWidget( useEXIF, 1, 0 );

    wrapIconText = new QCheckBox( groupboxThumbnails, "wrapIconText" );

    groupboxThumbnailsLayout->addWidget( wrapIconText, 1, 1 );
    layoutThumb->addWidget( groupboxThumbnails );
    thumbConfigWidgetLayout->addLayout( layoutThumb );

    layoutDetails = new QHBoxLayout( 0, 0, 6, "layoutDetails");

    groupBoxDetails = new QGroupBox( page9, "groupBoxDetails" );
    groupBoxDetails->setColumnLayout(0, Qt::Vertical );
    groupBoxDetails->layout()->setSpacing( 6 );
    groupBoxDetails->layout()->setMargin( 11 );
    groupBoxDetailsLayout = new QGridLayout( groupBoxDetails->layout() );
    groupBoxDetailsLayout->setAlignment( Qt::AlignTop );

    showMimeType = new QCheckBox( groupBoxDetails, "showMimeType" );

    groupBoxDetailsLayout->addWidget( showMimeType, 0, 0 );

    showSize = new QCheckBox( groupBoxDetails, "showSize" );

    groupBoxDetailsLayout->addWidget( showSize, 1, 0 );

    showDate = new QCheckBox( groupBoxDetails, "showDate" );

    groupBoxDetailsLayout->addWidget( showDate, 0, 1 );

    showDimension = new QCheckBox( groupBoxDetails, "showDimension" );

    groupBoxDetailsLayout->addWidget( showDimension, 1, 1 );
    layoutDetails->addWidget( groupBoxDetails );
    thumbConfigWidgetLayout->addLayout( layoutDetails );

    tooltipGroupBox = new QGroupBox( page9, "tooltipGroupBox" );
    tooltipGroupBox->setColumnLayout(0, Qt::Vertical );
    tooltipGroupBox->layout()->setSpacing( 6 );
    tooltipGroupBox->layout()->setMargin( 11 );
    tooltipGroupBoxLayout = new QVBoxLayout( tooltipGroupBox->layout() );
    tooltipGroupBoxLayout->setAlignment( Qt::AlignTop );

    showTooltip = new QCheckBox( tooltipGroupBox, "showTooltip" );
    tooltipGroupBoxLayout->addWidget( showTooltip );
    thumbConfigWidgetLayout->addWidget( tooltipGroupBox );
    thumbnailConfigSpacer = new QSpacerItem( 20, 30, QSizePolicy::Minimum, QSizePolicy::Expanding );
    thumbConfigWidgetLayout->addItem( thumbnailConfigSpacer );
//     languageChange();
//     resize( QSize(353, 315).expandedTo(minimumSizeHint()) );
//     clearWState( WState_Polished );

    // tab order
    setTabOrder( storethCheck, showFrame );
    setTabOrder( showFrame, useEXIF );
    setTabOrder( useEXIF, wrapIconText );
    setTabOrder( wrapIconText, showMimeType );
    setTabOrder( showMimeType, showDate );
    setTabOrder( showDate, showSize );
    setTabOrder( showSize, showDimension );

    groupboxThumbnails->setTitle( i18n( "Thumbnails" ) );
    showFrame->setText( i18n( "Show &frame" ) );
    storethCheck->setText( i18n( "Store &thumbnails" ) );
    useEXIF->setText( i18n( "Use EXIF &header" ) );
    QToolTip::add( useEXIF, i18n( "Load quick preview for images containing EXIF header, but not take into account modifications on the image" ) );
    wrapIconText->setText( i18n( "&Wrap icon text" ) );
    groupBoxDetails->setTitle( i18n( "Show details" ) );
    showMimeType->setText( i18n( "&Mime type" ) );
    showSize->setText( i18n( "&Size" ) );
    showDate->setText( i18n( "&Date" ) );
    showDimension->setText( i18n( "D&imension" ) );
    tooltipGroupBox->setTitle( i18n( "Tooltip" ) );
    showTooltip->setText( i18n( "Show &tooltip" ) );

}

void
ConfShowImg::initPaths(const QString& cdrom, const QString& gimp, const QString& convert, const QString& jpegtran)
{
	cdromPath->setText(cdrom);
	gimpPath->setText(gimp);
	convertPath->setText(convert);
	jpegtranPath->setText(jpegtran);
}
QString
ConfShowImg::getcdromPath()
{
	return cdromPath->text().stripWhiteSpace();
}
QString
ConfShowImg::getgimpPath()
{
	return gimpPath->text().stripWhiteSpace();
}
QString
ConfShowImg::getconvertPath()
{
	return convertPath->text().stripWhiteSpace();
}
QString
ConfShowImg::getjpegtranPath()
{
	return jpegtranPath->text().stripWhiteSpace();
}

void
ConfShowImg::addPage10()
{
	page10 = addPage( i18n("Paths"),
			 i18n("Paths"),
			 BarIcon("fileimport", CONFIG_ICON_SIZE));

////
    ExternalProgramsLayout = new QVBoxLayout( page10, 11, 6, "ExternalProgramsLayout");

    cdromgroupBox = new QGroupBox( page10, "cdromgroupBox" );
    cdromgroupBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, cdromgroupBox->sizePolicy().hasHeightForWidth() ) );
    cdromgroupBox->setColumnLayout(0, Qt::Vertical );
    cdromgroupBox->layout()->setSpacing( 6 );
    cdromgroupBox->layout()->setMargin( 11 );
    cdromgroupBoxLayout = new QVBoxLayout( cdromgroupBox->layout() );
    cdromgroupBoxLayout->setAlignment( Qt::AlignTop );

    cdromLayout = new QHBoxLayout( 0, 0, 6, "cdromLayout");

    cdromLabel = new QLabel( cdromgroupBox, "cdromLabel" );
    cdromLabel->setMinimumSize( QSize( 80, 0 ) );
    cdromLayout->addWidget( cdromLabel );

    cdromPath = new KLineEdit( cdromgroupBox, "cdromPath" );
    cdromPath->setMinimumSize( QSize( 150, 0 ) );
    cdromLayout->addWidget( cdromPath );

    cdromPathBrowser = new QPushButton( cdromgroupBox, "cdromPathBrowser" );
    cdromPathBrowser->setMinimumSize( QSize( 30, 30 ) );
    cdromPathBrowser->setMaximumSize( QSize( 30, 30 ) );
    cdromLayout->addWidget( cdromPathBrowser );
    cdromgroupBoxLayout->addLayout( cdromLayout );
    ExternalProgramsLayout->addWidget( cdromgroupBox );

    externalProgramsGroupBox = new QGroupBox( page10, "externalProgramsGroupBox" );
    externalProgramsGroupBox->setColumnLayout(0, Qt::Vertical );
    externalProgramsGroupBox->layout()->setSpacing( 6 );
    externalProgramsGroupBox->layout()->setMargin( 11 );
    externalProgramsGroupBoxLayout = new QGridLayout( externalProgramsGroupBox->layout() );
    externalProgramsGroupBoxLayout->setAlignment( Qt::AlignTop );

    gimpLabel = new QLabel( externalProgramsGroupBox, "gimpLabel" );
    gimpLabel->setMinimumSize( QSize( 80, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( gimpLabel, 0, 0 );

    gimpPath = new KLineEdit( externalProgramsGroupBox, "gimpPath" );
    gimpPath->setMinimumSize( QSize( 150, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( gimpPath, 0, 1 );

    gimpPathBrowser = new QPushButton( externalProgramsGroupBox, "gimpPathBrowser" );
    gimpPathBrowser->setMinimumSize( QSize( 30, 30 ) );
    gimpPathBrowser->setMaximumSize( QSize( 30, 30 ) );

    externalProgramsGroupBoxLayout->addWidget( gimpPathBrowser, 0, 2 );

    convertLabel = new QLabel( externalProgramsGroupBox, "convertLabel" );
    convertLabel->setMinimumSize( QSize( 80, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( convertLabel, 2, 0 );

    convertPath = new KLineEdit( externalProgramsGroupBox, "convertPath" );
    convertPath->setMinimumSize( QSize( 150, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( convertPath, 2, 1 );

    convertPathBrowser = new QPushButton( externalProgramsGroupBox, "convertPathBrowser" );
    convertPathBrowser->setMinimumSize( QSize( 30, 30 ) );
    convertPathBrowser->setMaximumSize( QSize( 30, 30 ) );

    externalProgramsGroupBoxLayout->addWidget( convertPathBrowser, 2, 2 );

    jpegtranLabel = new QLabel( externalProgramsGroupBox, "jpegtranLabel" );
    jpegtranLabel->setMinimumSize( QSize( 80, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( jpegtranLabel, 3, 0 );

    jpegtranPath = new KLineEdit( externalProgramsGroupBox, "jpegtranPath" );
    jpegtranPath->setMinimumSize( QSize( 150, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( jpegtranPath, 3, 1 );

    jpegtranPathBrowser = new QPushButton( externalProgramsGroupBox, "jpegtranPathBrowser" );
    jpegtranPathBrowser->setMinimumSize( QSize( 30, 30 ) );
    jpegtranPathBrowser->setMaximumSize( QSize( 30, 30 ) );

    externalProgramsGroupBoxLayout->addWidget( jpegtranPathBrowser, 3, 2 );

    externalProgramsLine = new QFrame( externalProgramsGroupBox, "externalProgramsLine" );
    externalProgramsLine->setFrameShape( QFrame::HLine );
    externalProgramsLine->setFrameShadow( QFrame::Sunken );
    externalProgramsLine->setFrameShape( QFrame::HLine );

    externalProgramsGroupBoxLayout->addMultiCellWidget( externalProgramsLine, 1, 1, 0, 2 );
    ExternalProgramsLayout->addWidget( externalProgramsGroupBox );
    //languageChange();
    //resize( QSize(417, 289).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

////
    cdromgroupBox->setTitle( i18n("CD-ROM" ) );
    cdromLabel->setText( i18n("CD-ROM path:" ) );
    cdromPathBrowser->setText( "..."  );
    externalProgramsGroupBox->setTitle( i18n("External Programs" ) );
    gimpLabel->setText( i18n( "The GIMP: " ) );
    gimpPathBrowser->setText( ("..." ) );
    convertLabel->setText( ("<tt>convert</tt>: " ) );
    convertPathBrowser->setText( ("..." ) );
    jpegtranLabel->setText( ("<tt>jpegtran</tt>: " ) );
    jpegtranPathBrowser->setText( ("..." ) );

////
    QToolTip::add(gimpPathBrowser , i18n( "The default value is <tt>gimp-remote -n</tt>." ) );
    cdromPathBrowser->setPixmap( QPixmap(BarIcon("folder_open",16 )));
    gimpPathBrowser->setPixmap( QPixmap(BarIcon("folder_open",16 )));
    convertPathBrowser->setPixmap( QPixmap(BarIcon("folder_open",16 )));
    jpegtranPathBrowser->setPixmap( QPixmap(BarIcon("folder_open",16 )));


    //
    connect( cdromPathBrowser, SIGNAL( clicked()), this, SLOT( chooseDir_cdrom() ));
    connect( gimpPathBrowser, SIGNAL( clicked()), this, SLOT( chooseDir_gimp() ));
    connect( convertPathBrowser, SIGNAL( clicked()), this, SLOT( chooseDir_convert() ));
    connect( jpegtranPathBrowser, SIGNAL( clicked()), this, SLOT( chooseDir_jpegtran() ));

    //
//     page10->resize( QSize(318, 241).expandedTo(minimumSizeHint()) );
    ExternalProgramsLayout->addItem( new QSpacerItem( 20, 70, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}



void
ConfShowImg::chooseDir_cdrom()
{
	QString s = KFileDialog::getExistingDirectory(cdromPath->text(),
								this,
								i18n("Specified Directory"));
 	 if(!s.isEmpty())
 		cdromPath->setText( s );
}
void
ConfShowImg::chooseDir_gimp()
{
	QString s = KFileDialog::getOpenFileName(gimpPath->text(),
								QString::null,
								this,
								i18n("Specified Directory"));
 	 if(!s.isEmpty())
 		gimpPath->setText( s );
}
void
ConfShowImg::chooseDir_convert()
{
	QString s = KFileDialog::getOpenFileName(convertPath->text(),
								QString::null,
								this,
								i18n("Specified Directory"));
 	 if(!s.isEmpty())
 		convertPath->setText( s );
}
void
ConfShowImg::chooseDir_jpegtran()
{
	QString s = KFileDialog::getOpenFileName(jpegtranPath->text(),
								QString::null,
								this,
								i18n("Specified Directory"));
 	 if(!s.isEmpty())
 		jpegtranPath->setText( s );
}


void
ConfShowImg::initImagePosition(ImageViewer::ImagePosition pos)
{
	otherRadioButton->setChecked( TRUE );
	switch(pos)
	{
		case ImageViewer::Centered : centeredRadioButton->setChecked( TRUE ); break;
		case ImageViewer::TopLeft : topLeftBut->setChecked( TRUE ); break;
		case ImageViewer::TopCentered : topBut->setChecked( TRUE ); break;
		case ImageViewer::TopRight: topRightBut->setChecked( TRUE ); break;
		case ImageViewer::LeftCentered : leftBut->setChecked( TRUE ); break;
		case ImageViewer::RightCentered : rightBut->setChecked( TRUE ); break;
		case ImageViewer::BottomLeft : bottomLeftBut->setChecked( TRUE ); break;
		case ImageViewer::BottomCentered: bottomBut->setChecked( TRUE ); break;
		case ImageViewer::BottomRight: bottomRightBut->setChecked( TRUE ); break;
	}
}

ImageViewer::ImagePosition
ConfShowImg::getImagePosition()
{
	if(topLeftBut->isChecked()) return ImageViewer::TopLeft;
	else if(topBut->isChecked()) return ImageViewer::TopCentered;
	else if(topRightBut->isChecked()) return ImageViewer::TopRight;
	else if(leftBut->isChecked()) return ImageViewer::LeftCentered;
	else if(centeredRadioButton->isChecked()) return ImageViewer::Centered;
	else if(rightBut->isChecked()) return ImageViewer::RightCentered;
	else if(bottomLeftBut->isChecked()) return ImageViewer::BottomLeft;
	else if(bottomBut->isChecked()) return ImageViewer::BottomCentered;
	else if(bottomRightBut->isChecked()) return ImageViewer::BottomRight;
	else  return ImageViewer::LeftCentered;
}

void
ConfShowImg::addPage11()
{
	page11 = addPage( i18n("Image Position"),
			 i18n("Image Position"),
			 BarIcon("image", CONFIG_ICON_SIZE));

    ImagePositionLayout = new QVBoxLayout( page11, 11, 6, "ImagePositionLayout");

    ImagePositionGroup = new QButtonGroup( page11, "ImagePositionGroup" );
    ImagePositionGroup->setColumnLayout(0, Qt::Vertical );
    ImagePositionGroup->layout()->setSpacing( 6 );
    ImagePositionGroup->layout()->setMargin( 11 );
    ImagePositionGroupLayout = new QVBoxLayout( ImagePositionGroup->layout() );
    ImagePositionGroupLayout->setAlignment( Qt::AlignTop );

    centeredRadioButton = new QRadioButton( ImagePositionGroup, "centeredRadioButton" );
    centeredRadioButton->setChecked( TRUE );
    ImagePositionGroupLayout->addWidget( centeredRadioButton );

    otherRadioButton = new QRadioButton( ImagePositionGroup, "otherRadioButton" );
    ImagePositionGroupLayout->addWidget( otherRadioButton );

    choosePosLayout = new QHBoxLayout( 0, 0, 6, "choosePosLayout");
    QSpacerItem* spacer = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    choosePosLayout->addItem( spacer );

    chosePosButtonGroup = new QButtonGroup( ImagePositionGroup, "chosePosButtonGroup" );
    chosePosButtonGroup->setEnabled( FALSE );
    chosePosButtonGroup->setFrameShape( QButtonGroup::NoFrame );
    chosePosButtonGroup->setFrameShadow( QButtonGroup::Raised );
    chosePosButtonGroup->setFlat( FALSE );
    chosePosButtonGroup->setColumnLayout(0, Qt::Vertical );
    chosePosButtonGroup->layout()->setSpacing( 6 );
    chosePosButtonGroup->layout()->setMargin( 11 );
    chosePosButtonGroupLayout = new QGridLayout( chosePosButtonGroup->layout() );
    chosePosButtonGroupLayout->setAlignment( Qt::AlignTop );

    leftBut = new QRadioButton( chosePosButtonGroup, "leftBut" );
    leftBut->setMinimumSize( QSize( 16, 16 ) );
    leftBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( leftBut, 1, 0 );

    bottomLeftBut = new QRadioButton( chosePosButtonGroup, "bottomLeftBut" );
    bottomLeftBut->setMinimumSize( QSize( 16, 16 ) );
    bottomLeftBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( bottomLeftBut, 2, 0 );

    topLeftBut = new QRadioButton( chosePosButtonGroup, "topLeftBut" );
    topLeftBut->setMinimumSize( QSize( 16, 16 ) );
    topLeftBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( topLeftBut, 0, 0 );

    topRightBut = new QRadioButton( chosePosButtonGroup, "topRightBut" );
    topRightBut->setMinimumSize( QSize( 16, 16 ) );
    topRightBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( topRightBut, 0, 6 );

    rightBut = new QRadioButton( chosePosButtonGroup, "rightBut" );
    rightBut->setMinimumSize( QSize( 16, 16 ) );
    rightBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( rightBut, 1, 6 );

    bottomRightBut = new QRadioButton( chosePosButtonGroup, "bottomRightBut" );
    bottomRightBut->setMinimumSize( QSize( 16, 16 ) );
    bottomRightBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( bottomRightBut, 2, 6 );

    imagePreviewPosLabel = new QLabel( chosePosButtonGroup, "imagePreviewPosLabel" );
    imagePreviewPosLabel->setMinimumSize( QSize( 64, 64 ) );
    imagePreviewPosLabel->setMaximumSize( QSize( 64, 64 ) );
    //imagePreviewPosLabel->setPixmap( image0 );

    chosePosButtonGroupLayout->addMultiCellWidget( imagePreviewPosLabel, 1, 1, 2, 4 );
    QSpacerItem* spacer_2 = new QSpacerItem( 1, 1, QSizePolicy::Fixed, QSizePolicy::Minimum );
    chosePosButtonGroupLayout->addItem( spacer_2, 1, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 1, 1, QSizePolicy::Fixed, QSizePolicy::Minimum );
    chosePosButtonGroupLayout->addItem( spacer_3, 1, 5 );
    QSpacerItem* spacer_4 = new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
    chosePosButtonGroupLayout->addMultiCell( spacer_4, 0, 0, 1, 2 );
    QSpacerItem* spacer_5 = new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
    chosePosButtonGroupLayout->addMultiCell( spacer_5, 2, 2, 1, 2 );

    topBut = new QRadioButton( chosePosButtonGroup, "topBut" );
    topBut->setMinimumSize( QSize( 16, 16 ) );
    topBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( topBut, 0, 3 );

    bottomBut = new QRadioButton( chosePosButtonGroup, "bottomBut" );
    bottomBut->setMinimumSize( QSize( 16, 16 ) );
    bottomBut->setMaximumSize( QSize( 16, 16 ) );

    chosePosButtonGroupLayout->addWidget( bottomBut, 2, 3 );
    QSpacerItem* spacer_6 = new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
    chosePosButtonGroupLayout->addMultiCell( spacer_6, 0, 0, 4, 5 );
    QSpacerItem* spacer_7 = new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
    chosePosButtonGroupLayout->addMultiCell( spacer_7, 2, 2, 4, 5 );
    choosePosLayout->addWidget( chosePosButtonGroup );
    QSpacerItem* spacer_8 = new QSpacerItem( 51, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    choosePosLayout->addItem( spacer_8 );
    ImagePositionGroupLayout->addLayout( choosePosLayout );
    ImagePositionLayout->addWidget( ImagePositionGroup );
    //languageChange();
//     page11->resize( QSize(361, 249).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( otherRadioButton, SIGNAL( toggled(bool) ), chosePosButtonGroup, SLOT( setEnabled(bool) ) );
    connect( otherRadioButton, SIGNAL( toggled(bool) ), topBut, SLOT( setChecked(bool) ) );



    //
        ImagePositionGroup->setTitle( i18n("Image Position" ) );
    centeredRadioButton->setText( i18n("Centered" ) );
    otherRadioButton->setText( i18n("Other..." ) );
    chosePosButtonGroup->setTitle( QString::null );
    rightBut->setText( QString::null );
    QToolTip::add( rightBut, i18n("Right centered" ) );
    bottomLeftBut->setText( QString::null );
    QToolTip::add( bottomLeftBut, i18n("Bottom left" ) );
    topLeftBut->setText( QString::null );
    QToolTip::add( topLeftBut, i18n("Top left" ) );
    topRightBut->setText( QString::null );
    QToolTip::add( topRightBut, i18n("Top right" ) );
    leftBut->setText( QString::null );
    QToolTip::add( leftBut, i18n("Left centered" ) );
    bottomRightBut->setText( QString::null );
    QToolTip::add( bottomRightBut, i18n("Bottom right" ) );
    imagePreviewPosLabel->setText( QString::null );
    topBut->setText( QString::null );
    QToolTip::add( topBut, i18n("Top centered" ) );
    bottomBut->setText( QString::null );
    QToolTip::add( bottomBut, i18n("Bottom centered" ) );

    ///
    imagePreviewPosLabel->setPixmap( BarIcon("showimg", 64) );
    ImagePositionLayout->addItem( new QSpacerItem( 20, 70, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}






















#include "confshowimg.moc"
