/***************************************************************************
                          confshowimg.cpp -  description
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

#include "confshowimg.h"

#include <fixx11h.h>
#ifdef HAVE_KIPI
#include <libkipi/version.h>
#endif


// Qt
#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <qbuttongroup.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtabwidget.h>

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
#include <kurlrequester.h>
#include <kmessagebox.h>

#define CONFIG_ICON_SIZE 24

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "


ConfShowImg::ConfShowImg(QWidget *parent)
:KDialogBase(IconList/*Tabbed*/, i18n("Configure showimg"),
		Help|Ok|Cancel, Ok,
		parent, "Configure showimg", true)
{
	addPage1();
	addPage2();
	addPage9();
	addPage11();
	addPage12();
#ifdef HAVE_KIPI
	addPage8();
#endif /* HAVE_KIPI */
#ifdef WANT_LIBKEXIDB
	addPage13();
#endif
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
ConfShowImg::slotOk()
{
#ifdef WANT_LIBKEXIDB
	if(m_categoriesSettings != getCategoriesType()+"--"+getCategoriesSqlitePath()+"--"+getCategoriesMysqlUsername()+"--"+getCategoriesMysqlPassword()+"--"+getCategoriesMysqlHostname())
		KMessageBox::information(this, i18n("Category configuration has changed, you need to restart to take effect."));
	KDialogBase::slotOk();
#endif
}

void
ConfShowImg::initFiling(int openType, const QString& openDir, bool showSP, bool startFS)
{
	if(openType==0) openHome->setChecked( true );
	else if(openType==1) openLast->setChecked( true );
	else open_custom->setChecked( true );
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
    openHome->setChecked( true );
    GroupBox13Layout->addWidget( openHome );

    openLast = new QRadioButton( GroupBox13, "openLast" );
    GroupBox13Layout->addWidget( openLast );

    open_custom = new QRadioButton( GroupBox13, "open_custom" );
    GroupBox13Layout->addWidget( open_custom );

    layout1_2 = new QHBoxLayout( 0, 0, 6, "layout1_2");

    LineEdit2 = new QLineEdit( GroupBox13, "LineEdit2" );
    LineEdit2->setEnabled( false );
    LineEdit2->setEdited( false );
    layout1_2->addWidget( LineEdit2 );

    chooseButton = new QPushButton( GroupBox13, "chooseButton" );
    chooseButton->setMaximumSize( QSize( 30, 30 ) );
    layout1_2->addWidget( chooseButton );
    GroupBox13Layout->addLayout( layout1_2 );
    page1Layout->addWidget( GroupBox13 );

    showSP = new QCheckBox( page1, "showSP" );
    showSP->setChecked( true );
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
                               bool sprelodim,
			       bool sArchive)
{
	smoothCheck->setChecked(smooth);
	sHiddenDirCheck ->setChecked(sHDir);
	sHiddenFileCheck ->setChecked(sHFile);
	sDirCheck ->setChecked(sDir);
	sAllCheck ->setChecked(sAll);
	prelodimCheck ->setChecked(sprelodim);
	loadfirstimCheck->setChecked(loadfim);
	sArchivesCheck->setChecked(sArchive);
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
bool
ConfShowImg::getShowArchives()
{
	return sArchivesCheck->isChecked();
}

void
ConfShowImg::addPage2()
{
	page2 = addPage( i18n("Miscellaneous"),
			 i18n("Miscellaneous"),
	    		 BarIcon("misc", CONFIG_ICON_SIZE));

//////////////
    MiscellaneousLayout = new QVBoxLayout( page2, 11, 6, "MiscellaneousLayout");

    miscellaneousGroupBox = new QGroupBox( page2, "miscellaneousGroupBox" );
    miscellaneousGroupBox->setColumnLayout(0, Qt::Vertical );
    miscellaneousGroupBox->layout()->setSpacing( 6 );
    miscellaneousGroupBox->layout()->setMargin( 11 );
    miscellaneousGroupBoxLayout = new QGridLayout( miscellaneousGroupBox->layout() );
    miscellaneousGroupBoxLayout->setAlignment( Qt::AlignTop );

    zoommodeGroupBox = new QGroupBox( miscellaneousGroupBox, "zoommodeGroupBox" );
    zoommodeGroupBox->setColumnLayout(0, Qt::Vertical );
    zoommodeGroupBox->layout()->setSpacing( 6 );
    zoommodeGroupBox->layout()->setMargin( 11 );
    zoommodeGroupBoxLayout = new QVBoxLayout( zoommodeGroupBox->layout() );
    zoommodeGroupBoxLayout->setAlignment( Qt::AlignTop );

    smoothCheck = new QCheckBox( zoommodeGroupBox, "smoothCheck" );
    zoommodeGroupBoxLayout->addWidget( smoothCheck );

    miscellaneousGroupBoxLayout->addWidget( zoommodeGroupBox, 0, 0 );

    preloaddingGroupBox = new QGroupBox( miscellaneousGroupBox, "preloaddingGroupBox" );
    preloaddingGroupBox->setColumnLayout(0, Qt::Vertical );
    preloaddingGroupBox->layout()->setSpacing( 6 );
    preloaddingGroupBox->layout()->setMargin( 11 );
    preloaddingGroupBoxLayout = new QVBoxLayout( preloaddingGroupBox->layout() );
    preloaddingGroupBoxLayout->setAlignment( Qt::AlignTop );

    prelodimCheck = new QCheckBox( preloaddingGroupBox, "prelodimCheck" );
    preloaddingGroupBoxLayout->addWidget( prelodimCheck );

    loadfirstimCheck = new QCheckBox( preloaddingGroupBox, "loadfirstimCheck" );
    preloaddingGroupBoxLayout->addWidget( loadfirstimCheck );

    miscellaneousGroupBoxLayout->addWidget( preloaddingGroupBox, 0, 1 );

    filendirGroupBox = new QGroupBox( miscellaneousGroupBox, "filendirGroupBox" );
    filendirGroupBox->setColumnLayout(0, Qt::Vertical );
    filendirGroupBox->layout()->setSpacing( 6 );
    filendirGroupBox->layout()->setMargin( 11 );
    filendirGroupBoxLayout = new QGridLayout( filendirGroupBox->layout() );
    filendirGroupBoxLayout->setAlignment( Qt::AlignTop );

    sHiddenDirCheck = new QCheckBox( filendirGroupBox, "sHiddenDirCheck" );

    filendirGroupBoxLayout->addWidget( sHiddenDirCheck, 0, 0 );

    sAllCheck = new QCheckBox( filendirGroupBox, "sAllCheck" );

    filendirGroupBoxLayout->addWidget( sAllCheck, 0, 1 );

    sDirCheck = new QCheckBox( filendirGroupBox, "sDirCheck" );

    filendirGroupBoxLayout->addWidget( sDirCheck, 1, 1 );

    sHiddenFileCheck = new QCheckBox( filendirGroupBox, "sHiddenFileCheck" );

    filendirGroupBoxLayout->addWidget( sHiddenFileCheck, 1, 0 );

    sArchivesCheck = new QCheckBox( filendirGroupBox, "sArchivesCheck" );

    filendirGroupBoxLayout->addWidget( sArchivesCheck, 2, 0 );

    miscellaneousGroupBoxLayout->addMultiCellWidget( filendirGroupBox, 1, 1, 0, 1 );
    MiscellaneousLayout->addWidget( miscellaneousGroupBox );
    miscellaneousSpacer = new QSpacerItem( 20, 21, QSizePolicy::Minimum, QSizePolicy::Expanding );
    MiscellaneousLayout->addItem( miscellaneousSpacer );

    // tab order
    setTabOrder( smoothCheck, prelodimCheck );
    setTabOrder( prelodimCheck, loadfirstimCheck );
    setTabOrder( loadfirstimCheck, sHiddenDirCheck );
    setTabOrder( sHiddenDirCheck, sAllCheck );
    setTabOrder( sAllCheck, sHiddenFileCheck );
    setTabOrder( sHiddenFileCheck, sDirCheck );
    setTabOrder( sDirCheck, sArchivesCheck );

    miscellaneousGroupBox->setTitle( i18n( "Miscellaneous" ) );
    zoommodeGroupBox->setTitle( i18n( "Zoom mode" ) );
    smoothCheck->setText( i18n( "Smooth &scale" ) );
    QToolTip::add( smoothCheck, i18n( "Better quality but slower and requires more memory" ) );
    preloaddingGroupBox->setTitle( i18n( "Preloading" ) );
    prelodimCheck->setText( i18n( "Preload &next image" ) );
    loadfirstimCheck->setText( i18n( "Load the f&irst image" ) );
    QToolTip::add( loadfirstimCheck, i18n( "Load the first image of the selected directory" ) );
    filendirGroupBox->setTitle( i18n( "Files and directories" ) );
    sHiddenDirCheck->setText( i18n( "Show hidden &directories" ) );
    sAllCheck->setText( i18n( "Show all &files" ) );
    sDirCheck->setText( i18n( "Show &directories" ) );
    sHiddenFileCheck->setText( i18n( "Show hidden &files" ) );
    sArchivesCheck->setText( i18n( "Show &Archives" ) );
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
    ColorRadioButton5->setEnabled( false );

    colorButtonGroup2Layout->addWidget( ColorRadioButton5, 0, 2 );

    PushButton1 = new QPushButton( colorButtonGroup2, "PushButton1" );
    PushButton1->setEnabled( false );

    colorButtonGroup2Layout->addWidget( PushButton1, 1, 2 );

    RadioButton4 = new QRadioButton( colorButtonGroup2, "RadioButton4" );
    RadioButton4->setChecked( true );

    colorButtonGroup2Layout->addMultiCellWidget( RadioButton4, 0, 0, 0, 1 );

    color = new KColorButton( colorButtonGroup2, "color" );
    color->setFlat( true );

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
    PixmapLabel1->setScaledContents( true );

    colorGroupBox6Layout->addMultiCellWidget( PixmapLabel1, 0, 1, 1, 1 );

    graySlider = new QSlider( colorGroupBox6, "graySlider" );
    graySlider->setMinValue( 30 );
    graySlider->setMaxValue( 100 );
    graySlider->setLineStep( 10 );
    graySlider->setValue( 30 );
    graySlider->setTracking( false );
    graySlider->setOrientation( QSlider::Horizontal );
    graySlider->setTickmarks( QSlider::Both );

    colorGroupBox6Layout->addWidget( graySlider, 1, 0 );
    spacer3 = new QSpacerItem( 20, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
    colorGroupBox6Layout->addItem( spacer3, 0, 0 );
    ColorsLayout->addWidget( colorGroupBox6 );
    colorspacer2 = new QSpacerItem( 20, 61, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ColorsLayout->addItem( colorspacer2 );
    //languageChange();
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
		forward->setChecked( true );
	else
	if(type==1)
		backward->setChecked( true );
	else
		random->setChecked( true );

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
    forward->setChecked( true );
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
    backward->setText( i18n( "&Backward" ) );
    random->setText( i18n( "&Random" ) );
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
    RadioButton5->setChecked( true );

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
    //clearWState( WState_Polished );
///
    ButtonGroup2->setTitle( i18n( "Layout" ) );
    radioButton_4->setText( QString::null );
    radioButton_1->setText( QString::null );
    radioButton_3->setText( QString::null );
    radioButton_2->setText( QString::null );
    RadioButton5->setText( i18n( "&Current" ) );
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
		osdTopRadioButton->setChecked( true );
	else
		osdBottomRadioButton->setChecked( true );
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
    sOSDcheckBox->setChecked( true );
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
    osdTopRadioButton->setChecked( true );
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
    osdOptDimensionsCheckBox->setText( i18n ("D&imensions" ) );
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
			 BarIcon("info", CONFIG_ICON_SIZE));


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
#ifdef HAVE_KIPI
	page8 = addPage( i18n("Plugins"),
					 i18n("Plugins Kipi - version %1").arg(kipi_version),
			 BarIcon("kipi", CONFIG_ICON_SIZE));

    Form1Layoutp8 = new QVBoxLayout( page8, 11, 6, "Form1Layout");
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
ConfShowImg::getShowCategoryInfo()
{
	return showCategoryinfo->isChecked();
}
bool
ConfShowImg::getShowTooltips()
{
	return showTooltip->isChecked();
}

void
ConfShowImg::initThumbnails(bool storeth,  bool showf, bool useexif, bool wwt,
				bool showMT, bool showS, bool showD, bool showDim, bool showCat,
				bool showTool)
{
	storethCheck->setChecked(storeth);
	showFrame->setChecked(showf);
	useEXIF->setChecked(useexif);
	wrapIconText->setChecked(wwt);

	showMimeType->setChecked(showMT);
	showSize->setChecked(showS);
	showDate->setChecked(showD);
	showDimension->setChecked(showDim);
	showCategoryinfo->setChecked(showCat);
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
    storethCheck->setTristate( true );

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

    showCategoryinfo = new QCheckBox( groupBoxDetails, "showCategoryinfo" );

    groupBoxDetailsLayout->addWidget( showCategoryinfo, 2, 0 );
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
    showDate->setText( i18n( "&Date (slow)" ) );
    showDimension->setText( i18n( "D&imension (slow)" ) );
    showCategoryinfo->setText( i18n( "&Category Info (experimental)" ) );
    tooltipGroupBox->setTitle( i18n( "Tooltip" ) );
    showTooltip->setText( i18n( "Show &tooltip" ) );
}

void
ConfShowImg::initPaths(const QString& cdrom, const QString& gimp, const QString& convert, const QString& jpegtran, const QString& unrar)
{
	cdromPath->setURL(cdrom);
	gimpPath->setURL(gimp);
	convertPath->setURL(convert);
	jpegtranPath->setURL(jpegtran);
	unrarPath->setURL(unrar);
}
QString
ConfShowImg::getcdromPath()
{
	return cdromPath->url().stripWhiteSpace();
}
QString
ConfShowImg::getgimpPath()
{
	return gimpPath->url().stripWhiteSpace();
}
QString
ConfShowImg::getconvertPath()
{
	return convertPath->url().stripWhiteSpace();
}
QString
ConfShowImg::getjpegtranPath()
{
	return jpegtranPath->url().stripWhiteSpace();
}
QString
ConfShowImg::getunrarPath()
{
	return unrarPath->url().stripWhiteSpace();
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
    cdromgroupBoxLayout = new QGridLayout( cdromgroupBox->layout() );
    cdromgroupBoxLayout->setAlignment( Qt::AlignTop );

    cdromLabel = new QLabel( cdromgroupBox, "cdromLabel" );
    cdromLabel->setMinimumSize( QSize( 80, 0 ) );

    cdromgroupBoxLayout->addWidget( cdromLabel, 0, 0 );

    cdromPath = new KURLRequester( cdromgroupBox, "cdromPath" );

    cdromgroupBoxLayout->addWidget( cdromPath, 0, 1 );
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

    convertLabel = new QLabel( externalProgramsGroupBox, "convertLabel" );
    convertLabel->setMinimumSize( QSize( 80, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( convertLabel, 3, 0 );

    jpegtranLabel = new QLabel( externalProgramsGroupBox, "jpegtranLabel" );
    jpegtranLabel->setMinimumSize( QSize( 80, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( jpegtranLabel, 4, 0 );

    unrarLabel = new QLabel( externalProgramsGroupBox, "unrarLabel" );
    unrarLabel->setMinimumSize( QSize( 80, 0 ) );

    externalProgramsGroupBoxLayout->addWidget( unrarLabel, 7, 0 );

    externalProgramsLine = new QFrame( externalProgramsGroupBox, "externalProgramsLine" );
    externalProgramsLine->setFrameShape( QFrame::HLine );
    externalProgramsLine->setFrameShadow( QFrame::Sunken );
    externalProgramsLine->setFrameShape( QFrame::HLine );

    externalProgramsGroupBoxLayout->addMultiCellWidget( externalProgramsLine, 1, 2, 0, 1 );

    convertPath = new KURLRequester( externalProgramsGroupBox, "convertPath" );

    externalProgramsGroupBoxLayout->addMultiCellWidget( convertPath, 2, 3, 1, 1 );

    externalProgramsLine_2 = new QFrame( externalProgramsGroupBox, "externalProgramsLine_2" );
    externalProgramsLine_2->setFrameShape( QFrame::HLine );
    externalProgramsLine_2->setFrameShadow( QFrame::Sunken );
    externalProgramsLine_2->setFrameShape( QFrame::HLine );

    externalProgramsGroupBoxLayout->addMultiCellWidget( externalProgramsLine_2, 5, 6, 0, 1 );

    jpegtranPath = new KURLRequester( externalProgramsGroupBox, "jpegtranPath" );

    externalProgramsGroupBoxLayout->addWidget( jpegtranPath, 4, 1 );

    unrarPath = new KURLRequester( externalProgramsGroupBox, "unrarPath" );

    externalProgramsGroupBoxLayout->addMultiCellWidget( unrarPath, 6, 7, 1, 1 );

    gimpPath = new KURLRequester( externalProgramsGroupBox, "gimpPath" );

    externalProgramsGroupBoxLayout->addWidget( gimpPath, 0, 1 );
    ExternalProgramsLayout->addWidget( externalProgramsGroupBox );
    pathspacer10 = new QSpacerItem( 20, 31, QSizePolicy::Minimum, QSizePolicy::Expanding );
    ExternalProgramsLayout->addItem( pathspacer10 );
//    languageChange();
//    clearWState( WState_Polished );

///////////////////////////////////////////////////////////
	cdromgroupBox->setTitle( i18n( "CD-Rom" ) );
	cdromLabel->setText( i18n( "CD-Rom path:" ) );
	externalProgramsGroupBox->setTitle( i18n( "External programs" ) );
 	gimpLabel->setText( i18n( "The GIMP: " ) );
 	convertLabel->setText( i18n( "<tt>convert</tt>: " ) );
 	jpegtranLabel->setText( i18n( "<tt>jpegtran</tt>: " ) );
 	unrarLabel->setText( i18n( "<tt>unrar</tt>:" ) );
}





void
ConfShowImg::initImagePosition(ImageViewer::ImagePosition pos)
{
	otherRadioButton->setChecked( true );
	switch(pos)
	{
		case ImageViewer::Centered : centeredRadioButton->setChecked( true ); break;
		case ImageViewer::TopLeft : topLeftBut->setChecked( true ); break;
		case ImageViewer::TopCentered : topBut->setChecked( true ); break;
		case ImageViewer::TopRight: topRightBut->setChecked( true ); break;
		case ImageViewer::LeftCentered : leftBut->setChecked( true ); break;
		case ImageViewer::RightCentered : rightBut->setChecked( true ); break;
		case ImageViewer::BottomLeft : bottomLeftBut->setChecked( true ); break;
		case ImageViewer::BottomCentered: bottomBut->setChecked( true ); break;
		case ImageViewer::BottomRight: bottomRightBut->setChecked( true ); break;
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
    centeredRadioButton->setChecked( true );
    ImagePositionGroupLayout->addWidget( centeredRadioButton );

    otherRadioButton = new QRadioButton( ImagePositionGroup, "otherRadioButton" );
    ImagePositionGroupLayout->addWidget( otherRadioButton );

    choosePosLayout = new QHBoxLayout( 0, 0, 6, "choosePosLayout");
    QSpacerItem* spacer = new QSpacerItem( 30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum );
    choosePosLayout->addItem( spacer );

    chosePosButtonGroup = new QButtonGroup( ImagePositionGroup, "chosePosButtonGroup" );
    chosePosButtonGroup->setEnabled( false );
    chosePosButtonGroup->setFrameShape( QButtonGroup::NoFrame );
    chosePosButtonGroup->setFrameShadow( QButtonGroup::Raised );
    chosePosButtonGroup->setFlat( false );
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
    QToolTip::add( rightBut, i18n("Right Centered" ) );
    bottomLeftBut->setText( QString::null );
    QToolTip::add( bottomLeftBut, i18n("Bottom Left" ) );
    topLeftBut->setText( QString::null );
    QToolTip::add( topLeftBut, i18n("Top Left" ) );
    topRightBut->setText( QString::null );
    QToolTip::add( topRightBut, i18n("Top Right" ) );
    leftBut->setText( QString::null );
    QToolTip::add( leftBut, i18n("Left Centered" ) );
    bottomRightBut->setText( QString::null );
    QToolTip::add( bottomRightBut, i18n("Bottom Right" ) );
    imagePreviewPosLabel->setText( QString::null );
    topBut->setText( QString::null );
    QToolTip::add( topBut, i18n("Top Centered" ) );
    bottomBut->setText( QString::null );
    QToolTip::add( bottomBut, i18n("Bottom Centered" ) );

    ///
    imagePreviewPosLabel->setPixmap( BarIcon("showimg", 64) );
    ImagePositionLayout->addItem( new QSpacerItem( 20, 70, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}


void
ConfShowImg::initVideo(bool enable, const QStringList& availableMovieViewer, int current)
{
	enableVideoCheckBox->setChecked(enable);
	for ( QStringList::ConstIterator it = availableMovieViewer.begin(); it != availableMovieViewer.end(); ++it )
	{
		chooseEngineComboBox->insertItem( *it );
	}
	chooseEngineComboBox->setCurrentItem(current);
}

int
ConfShowImg::getVideoEnabled()
{
	if(!enableVideoCheckBox->isChecked())
		return -1;
	else
		return chooseEngineComboBox->currentItem();
}

void
ConfShowImg::addPage12()
{
	page12 = addPage( i18n("Video"),
			 i18n("Video support"),
			 BarIcon("video", CONFIG_ICON_SIZE));

	VideoConfigLayout = new QVBoxLayout( page12, 11, 6, "VideoConfigLayout");

	videConfigGroupBox = new QGroupBox( page12, "videConfigGroupBox" );
	videConfigGroupBox->setColumnLayout(0, Qt::Vertical );
	videConfigGroupBox->layout()->setSpacing( 6 );
	videConfigGroupBox->layout()->setMargin( 11 );
	videConfigGroupBoxLayout = new QVBoxLayout( videConfigGroupBox->layout() );
	videConfigGroupBoxLayout->setAlignment( Qt::AlignTop );

	enableVideoCheckBox = new QCheckBox( videConfigGroupBox, "enableVideoCheckBox" );
	enableVideoCheckBox->setEnabled( true );
	enableVideoCheckBox->setChecked( true );
	videConfigGroupBoxLayout->addWidget( enableVideoCheckBox );

	videConfigLayout = new QHBoxLayout( 0, 0, 6, "videConfigLayout");

	chooseEngineLabel = new QLabel( videConfigGroupBox, "chooseEngineLabel" );
// 	chooseEngineLabel->setEnabled( false );
	videConfigLayout->addWidget( chooseEngineLabel );

	chooseEngineComboBox = new QComboBox( false, videConfigGroupBox, "chooseEngineComboBox" );
//	chooseEngineComboBox->setEnabled( false );
	videConfigLayout->addWidget( chooseEngineComboBox );
	videConfigSpacer1 = new QSpacerItem( 31, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	videConfigLayout->addItem( videConfigSpacer1 );
	videConfigGroupBoxLayout->addLayout( videConfigLayout );
	VideoConfigLayout->addWidget( videConfigGroupBox );
	videConfigSpacer2 = new QSpacerItem( 20, 220, QSizePolicy::Minimum, QSizePolicy::Expanding );
	VideoConfigLayout->addItem( videConfigSpacer2 );
// 	languageChange();
// 	clearWState( WState_Polished );

	videConfigGroupBox->setTitle( i18n( "Video" ) );
	enableVideoCheckBox->setText( i18n( "Enable video support" ) );
	chooseEngineLabel->setText( i18n( "Choose engine" ) );
	chooseEngineComboBox->clear();
	//chooseEngineComboBox->insertItem( i18n( "Kafeine" ) );
}


#ifdef WANT_LIBKEXIDB
void
ConfShowImg::initCategories(bool enable, bool addAllImages, const QString& type, const QString& sqlitePath, const QString& mysqlUsername, const QString& mysqlPassword, const QString& mysqlHostname)
{
	enableCategoriesCheckBox->setChecked(enable);
	addAllImageCheckBox->setChecked(addAllImages);

	categoriesComboBox->setCurrentItem (0);
	if(categoriesComboBox->currentText().lower()!=type.lower())
		categoriesComboBox->setCurrentItem (1);
// 	categoriesComboBox->setCurrentText(type.lower());

	sqlitePathURL->setURL(sqlitePath);
	mysqlUsernameLineEdit->setText(mysqlUsername);
	mysqlPasswordLineEdit->setText(mysqlPassword);
	mysqlHostnameLineEdit->setText(mysqlHostname);

	m_categoriesSettings=type+"--"+sqlitePath+"--"+mysqlUsername+"--"+mysqlPassword+"--"+mysqlHostname;
}

bool
ConfShowImg::getCategoriesEnabled()
{
	return enableCategoriesCheckBox->isChecked();
}
bool
ConfShowImg::getCategoriesAddAllImages()
{
	return addAllImageCheckBox->isChecked();
}
QString
ConfShowImg::getCategoriesType()
{
	return categoriesComboBox->currentText().lower();
}
QString
ConfShowImg::getCategoriesSqlitePath()
{
	return sqlitePathURL->url().stripWhiteSpace();
}
QString
ConfShowImg::getCategoriesMysqlUsername()
{
	return mysqlUsernameLineEdit->text();
}
QString
ConfShowImg::getCategoriesMysqlPassword()
{
	return mysqlPasswordLineEdit->text();
}
QString
ConfShowImg:: getCategoriesMysqlHostname()
{
	return mysqlHostnameLineEdit->text();
}


void
ConfShowImg::addPage13()
{
	page13 = addPage( i18n("Categories"),
			 i18n("Categories support"),
			 BarIcon("kexi_kexi", CONFIG_ICON_SIZE));
	/////////////

    CategoriesConfigLayout = new QVBoxLayout( page13, 11, 6, "CategoriesConfigLayout");

    categoriesGroupBox = new QGroupBox( page13, "categoriesGroupBox" );
    categoriesGroupBox->setColumnLayout(0, Qt::Vertical );
    categoriesGroupBox->layout()->setSpacing( 6 );
    categoriesGroupBox->layout()->setMargin( 11 );
    categoriesGroupBoxLayout = new QVBoxLayout( categoriesGroupBox->layout() );
    categoriesGroupBoxLayout->setAlignment( Qt::AlignTop );

    enableCategoriesCheckBox = new QCheckBox( categoriesGroupBox, "enableCategoriesCheckBox" );
    enableCategoriesCheckBox->setChecked( true );
    categoriesGroupBoxLayout->addWidget( enableCategoriesCheckBox );

    addAllImageCheckBox = new QCheckBox( categoriesGroupBox, "addAllImageCheckBox" );
    categoriesGroupBoxLayout->addWidget( addAllImageCheckBox );

    categoriesLayout2 = new QHBoxLayout( 0, 0, 6, "categoriesLayout2");

    categoriesTextLabel = new QLabel( categoriesGroupBox, "categoriesTextLabel" );
    categoriesLayout2->addWidget( categoriesTextLabel );

    categoriesComboBox = new QComboBox( false, categoriesGroupBox, "categoriesComboBox" );
    categoriesLayout2->addWidget( categoriesComboBox );
    categoriesSpacer3 = new QSpacerItem( 101, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    categoriesLayout2->addItem( categoriesSpacer3 );
    categoriesGroupBoxLayout->addLayout( categoriesLayout2 );

    categoriesOptionsTabWidget = new QTabWidget( categoriesGroupBox, "categoriesOptionsTabWidget" );
    categoriesOptionsTabWidget->setEnabled( true );

    tab = new QWidget( categoriesOptionsTabWidget, "tab" );
    tabLayout = new QVBoxLayout( tab, 11, 6, "tabLayout");

    categoriesSQLiteGroupBox = new QGroupBox( tab, "categoriesSQLiteGroupBox" );
    categoriesSQLiteGroupBox->setColumnLayout(0, Qt::Vertical );
    categoriesSQLiteGroupBox->layout()->setSpacing( 6 );
    categoriesSQLiteGroupBox->layout()->setMargin( 11 );
    categoriesSQLiteGroupBoxLayout = new QHBoxLayout( categoriesSQLiteGroupBox->layout() );
    categoriesSQLiteGroupBoxLayout->setAlignment( Qt::AlignTop );

    sqlitePathTextLabel = new QLabel( categoriesSQLiteGroupBox, "sqlitePathTextLabel" );
    categoriesSQLiteGroupBoxLayout->addWidget( sqlitePathTextLabel );

    sqlitePathURL = new KURLRequester( categoriesSQLiteGroupBox, "sqlitePathURL" );
    categoriesSQLiteGroupBoxLayout->addWidget( sqlitePathURL );
    tabLayout->addWidget( categoriesSQLiteGroupBox );
    categoriesOptionsTabWidget->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( categoriesOptionsTabWidget, "tab_2" );
    tabLayout_2 = new QHBoxLayout( tab_2, 11, 6, "tabLayout_2");

    categoriesMySQLGroupBox = new QGroupBox( tab_2, "categoriesMySQLGroupBox" );
    categoriesMySQLGroupBox->setColumnLayout(0, Qt::Vertical );
    categoriesMySQLGroupBox->layout()->setSpacing( 6 );
    categoriesMySQLGroupBox->layout()->setMargin( 11 );
    categoriesMySQLGroupBoxLayout = new QGridLayout( categoriesMySQLGroupBox->layout() );
    categoriesMySQLGroupBoxLayout->setAlignment( Qt::AlignTop );

    mysqlUsernameTextLabel = new QLabel( categoriesMySQLGroupBox, "mysqlUsernameTextLabel" );

    categoriesMySQLGroupBoxLayout->addWidget( mysqlUsernameTextLabel, 0, 0 );

    mysqlUsernameLineEdit = new KLineEdit( categoriesMySQLGroupBox, "mysqlUsernameLineEdit" );

    categoriesMySQLGroupBoxLayout->addWidget( mysqlUsernameLineEdit, 0, 1 );

    mysqlPasswordTextLabel = new QLabel( categoriesMySQLGroupBox, "mysqlPasswordTextLabel" );

    categoriesMySQLGroupBoxLayout->addWidget( mysqlPasswordTextLabel, 1, 0 );

    mysqlPasswordLineEdit = new KLineEdit( categoriesMySQLGroupBox, "mysqlPasswordLineEdit" );
    mysqlPasswordLineEdit->setEchoMode( KLineEdit::Password );

    categoriesMySQLGroupBoxLayout->addWidget( mysqlPasswordLineEdit, 1, 1 );

    mysqlHostnameTextLabel = new QLabel( categoriesMySQLGroupBox, "mysqlHostnameTextLabel" );

    categoriesMySQLGroupBoxLayout->addWidget( mysqlHostnameTextLabel, 2, 0 );

    mysqlHostnameLineEdit = new KLineEdit( categoriesMySQLGroupBox, "mysqlHostnameLineEdit" );

    categoriesMySQLGroupBoxLayout->addWidget( mysqlHostnameLineEdit, 2, 1 );
    tabLayout_2->addWidget( categoriesMySQLGroupBox );
    categoriesOptionsTabWidget->insertTab( tab_2, QString::fromLatin1("") );
    categoriesGroupBoxLayout->addWidget( categoriesOptionsTabWidget );
    CategoriesConfigLayout->addWidget( categoriesGroupBox );
    categoriesSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    CategoriesConfigLayout->addItem( categoriesSpacer );

    // signals and slots connections
    connect( enableCategoriesCheckBox, SIGNAL( toggled(bool) ), addAllImageCheckBox, SLOT( setEnabled(bool) ) );
    connect( enableCategoriesCheckBox, SIGNAL( toggled(bool) ), categoriesOptionsTabWidget, SLOT( setEnabled(bool) ) );
    connect( enableCategoriesCheckBox, SIGNAL( toggled(bool) ), categoriesTextLabel, SLOT( setEnabled(bool) ) );
    connect( enableCategoriesCheckBox, SIGNAL( toggled(bool) ), categoriesComboBox, SLOT( setEnabled(bool) ) );

//     setCaption( i18n( "Form2" ) );
    categoriesGroupBox->setTitle( i18n( "Categories" ) );
    enableCategoriesCheckBox->setText( i18n( "Enable categories" ) );
    addAllImageCheckBox->setText( i18n( "Add all images while browsing" ) );
    categoriesTextLabel->setText( i18n( "Choose engine" ) );
    categoriesComboBox->clear();
    categoriesComboBox->insertItem( i18n( "SQLite" ) );
    categoriesComboBox->insertItem( i18n( "MySQL" ) );
    categoriesSQLiteGroupBox->setTitle( i18n( "Options" ) );
    sqlitePathTextLabel->setText( i18n( "Database Path:" ) );
    categoriesOptionsTabWidget->changeTab( tab, i18n( "SQLite (default)" ) );
    categoriesMySQLGroupBox->setTitle( i18n( "Options" ) );
    mysqlUsernameTextLabel->setText( i18n( "Username:" ) );
    mysqlPasswordTextLabel->setText( i18n( "Password:" ) );
    mysqlHostnameTextLabel->setText( i18n( "Hostname:" ) );
    categoriesOptionsTabWidget->changeTab( tab_2, i18n( "MySQL" ) );
}

#endif










#include "confshowimg.moc"
