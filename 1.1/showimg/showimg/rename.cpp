/***************************************************************************
                          rename.cpp  -  description
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

#include "rename.h"

#include "batchrenamer.h"

// Qt
#include <qvariant.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>

// KDE
#include <kdatewidget.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <klineedit.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class DateTimeOption : public KDialogBase
{
public:

    DateTimeOption( QWidget* parent );
    ~DateTimeOption();

    void languageChange();

    void setDateFormat(QString format);
    QString getDateFormat();
    void setTimeFormat(QString format);
    QString getTimeFormat();

    void slotDefault();

public:
    QGroupBox* formatOptions;
    QGroupBox* dateFormatOption;
    KLineEdit* dateFormatLine;
    QGroupBox* timeFormatOption;
    KLineEdit* timeFormatLine;

protected:
    QVBoxLayout* DateTimeOptionLayout;
    QVBoxLayout* formatOptionsLayout;
    QHBoxLayout* dateFormatOptionLayout;
    QHBoxLayout* timeFormatOptionLayout;
};


DateTimeOption::DateTimeOption( QWidget* parent )
    : KDialogBase( parent, "DateTimeOption", true, "DateTimeOption",
 		    KDialogBase::Help | KDialogBase::Default | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
		    true )
{
    QWidget *page = new QWidget( this );
    setMainWidget(page);
////
    DateTimeOptionLayout = new QVBoxLayout( page, 11, 6, "DateTimeOptionLayout");

    formatOptions = new QGroupBox( page, "formatOptions" );
    formatOptions->setColumnLayout(0, Qt::Vertical );
    formatOptions->layout()->setSpacing( 6 );
    formatOptions->layout()->setMargin( 11 );
    formatOptionsLayout = new QVBoxLayout( formatOptions->layout() );
    formatOptionsLayout->setAlignment( Qt::AlignTop );

    dateFormatOption = new QGroupBox( formatOptions, "dateFormatOption" );
    dateFormatOption->setColumnLayout(0, Qt::Vertical );
    dateFormatOption->layout()->setSpacing( 6 );
    dateFormatOption->layout()->setMargin( 11 );
    dateFormatOptionLayout = new QHBoxLayout( dateFormatOption->layout() );
    dateFormatOptionLayout->setAlignment( Qt::AlignTop );

    dateFormatLine = new KLineEdit( dateFormatOption, "dateFormatLine" );
    dateFormatOptionLayout->addWidget( dateFormatLine );
    formatOptionsLayout->addWidget( dateFormatOption );

    timeFormatOption = new QGroupBox( formatOptions, "timeFormatOption" );
    timeFormatOption->setColumnLayout(0, Qt::Vertical );
    timeFormatOption->layout()->setSpacing( 6 );
    timeFormatOption->layout()->setMargin( 11 );
    timeFormatOptionLayout = new QHBoxLayout( timeFormatOption->layout() );
    timeFormatOptionLayout->setAlignment( Qt::AlignTop );

    timeFormatLine = new KLineEdit( timeFormatOption, "timeFormatLine" );
    timeFormatOptionLayout->addWidget( timeFormatLine );
    formatOptionsLayout->addWidget( timeFormatOption );
    DateTimeOptionLayout->addWidget( formatOptions );
    languageChange();
    resize( QSize(348, 170).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );


    setHelp("batchRename.formats.anchor", "showimg");

}

/*
 *  Destroys the object and frees any allocated resources
 */
DateTimeOption::~DateTimeOption()
{
    // no need to delete child widgets, Qt does it all for us
}

void
DateTimeOption::setDateFormat(QString format)
{
	dateFormatLine->setText(format);
}
QString
DateTimeOption::getDateFormat()
{
	return dateFormatLine->text();
}
void
DateTimeOption::setTimeFormat(QString format)
{
	timeFormatLine->setText(format);
}
QString
DateTimeOption::getTimeFormat()
{
	return timeFormatLine->text();
}

void
DateTimeOption::slotDefault()
{
	setDateFormat( KGlobal::locale()->dateFormatShort());
	setTimeFormat( KGlobal::locale()->timeFormat());
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DateTimeOption::languageChange()
{
    setCaption( i18n( "Format Options" ) );
    formatOptions->setTitle( i18n( "Format Options" ) );
    dateFormatOption->setTitle( i18n( "Date Format Options" ) );
    timeFormatOption->setTitle( i18n( "Time Format Options" ) );


    QWhatsThis::add( dateFormatOption, "<qt>"+i18n(
   "<u>Changes the current short date format.</u>"
   "<p>The format of the date is a string which contains variables that will"
   " be replaced:"
   "<ul>"
   " <li> %Y with the century (e.g. '19' for '1984')"
   " <li> %y with the lower 2 digits of the year (e.g. '84' for '1984')"
   " <li> %n with the month (January='1', December='12')"
   " <li> %m with the month with two digits (January='01', December='12')"
   " <li> %e with the day of the month (e.g. '1' on the first of march)"
   " <li> %d with the day of the month with two digits (e.g. '01' on the first of march)"
   " <li> %b with the short form of the month (e.g. 'Jan' for January)"
   " <li> %a with the short form of the weekday (e.g. 'Wed' for Wednesday)"
   " <li> %A with the long form of the weekday (e.g. 'Wednesday' for Wednesday)"
   " Everything else in the format string will be taken as is."
   "</ul>"
   " For example, March 20th 1989 with the format '%y:%m:%d' results"
   " in '89:03:20'."
      )
      );
    QWhatsThis::add( timeFormatOption, "<qt>"+i18n(
   " <u>Changes the current time format.</u>"
   " <p>The format of the time is string a which contains variables that will"
   " be replaced:<ul>"
   " <li> %H with the hour in 24h format and 2 digits (e.g. 5pm is '17', 5am is '05')"
   " <li> %k with the hour in 24h format and one digits (e.g. 5pm is '17', 5am is '5')"
   " <li> %I with the hour in 12h format and 2 digits (e.g. 5pm is '05', 5am is '05')"
   " <li> %l with the hour in 12h format and one digits (e.g. 5pm is '5', 5am is '5')"
   " <li> %M with the minute with 2 digits (e.g. the minute of 07:02:09 is '02')"
   " <li> %S with the seconds with 2 digits (e.g. the second of 07:02:09 is '09')"
   " <li> %p with pm or am (e.g. 17.00 is 'pm', 05.00 is 'am')"
   " </ul>"
   " Everything else in the format string will be taken as is."
   " For example, 5.23pm with the format '%H:%M' results"
   " in '17:23'."
      )
      );
}





RenameSeries::RenameSeries( QWidget* parent,  const char* name)
    : KDialogBase( parent, name, true, name,
 		    KDialogBase::Help | KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
		    true )
{

    if ( !name )
	setName( "RenameSeries" );

    QWidget *page = new QWidget( this );
    setMainWidget(page);
////
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    RenameSeriesLayout = new QGridLayout( page, 1, 1, 11, 6, "RenameSeriesLayout");

    groupBox5 = new QGroupBox( page, "groupBox5" );
    groupBox5->setColumnLayout(0, Qt::Vertical );
    groupBox5->layout()->setSpacing( 6 );
    groupBox5->layout()->setMargin( 11 );
    groupBox5Layout = new QVBoxLayout( groupBox5->layout() );
    groupBox5Layout->setAlignment( Qt::AlignTop );

    layout13 = new QHBoxLayout( 0, 0, 6, "layout13");

    remanedPreviewListView = new QListView( groupBox5, "remanedPreviewListView" );
    remanedPreviewListView->addColumn( i18n( "origin" ) );
    remanedPreviewListView->header()->setClickEnabled( FALSE, remanedPreviewListView->header()->count() - 1 );
    remanedPreviewListView->addColumn( i18n( "renamed" ) );
    remanedPreviewListView->header()->setClickEnabled( FALSE, remanedPreviewListView->header()->count() - 1 );
    remanedPreviewListView->header()->setResizeEnabled( FALSE, remanedPreviewListView->header()->count() - 1 );
    remanedPreviewListView->setMouseTracking( FALSE );
    remanedPreviewListView->setFocusPolicy( QListView::NoFocus );
    remanedPreviewListView->setFrameShape( QListView::Box );
    remanedPreviewListView->setFrameShadow( QListView::Plain );
    remanedPreviewListView->setLineWidth( 1 );
    remanedPreviewListView->setMargin( 0 );
    remanedPreviewListView->setMidLineWidth( 0 );
    remanedPreviewListView->setAllColumnsShowFocus( TRUE );
    remanedPreviewListView->setShowSortIndicator( TRUE );
    remanedPreviewListView->setItemMargin( 1 );
    remanedPreviewListView->setRootIsDecorated( FALSE );
    layout13->addWidget( remanedPreviewListView );

    layout12 = new QVBoxLayout( 0, 0, 6, "layout12");

    //moveUpPushButton = new QToolButton( groupBox5, "moveUpPushButton" );
    moveUpPushButton = new QToolButton(Qt::UpArrow,  groupBox5, "moveUpPushButton" );
    moveUpPushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, 0, 0, moveUpPushButton->sizePolicy().hasHeightForWidth() ) );
    moveUpPushButton->setMinimumSize( QSize( 0, 0 ) );
    layout12->addWidget( moveUpPushButton );

    //moveDownPushButton = new QToolButton( groupBox5, "moveDownPushButton" );
    moveDownPushButton = new QToolButton(Qt::DownArrow,  groupBox5, "moveDownPushButton" );
    moveDownPushButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, 0, 0, moveDownPushButton->sizePolicy().hasHeightForWidth() ) );
    layout12->addWidget( moveDownPushButton );
    layout13->addLayout( layout12 );
    groupBox5Layout->addLayout( layout13 );

    layout15 = new QVBoxLayout( 0, 0, 6, "layout15");

    previewCheckBox = new QCheckBox( groupBox5, "previewCheckBox" );
    layout15->addWidget( previewCheckBox );

    layout14_3 = new QHBoxLayout( 0, 0, 6, "layout14_3");
    spacer6_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout14_3->addItem( spacer6_3 );

    PixmapLabel = new QLabel( groupBox5, "PixmapLabel" );
    PixmapLabel->setEnabled( FALSE );
    PixmapLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, PixmapLabel->sizePolicy().hasHeightForWidth() ) );
    PixmapLabel->setFrameShape( QLabel::Box );
    PixmapLabel->setFrameShadow( QLabel::Raised );
    //PixmapLabel->setPixmap( image0 );
    PixmapLabel->setScaledContents( TRUE );
    layout14_3->addWidget( PixmapLabel );
    spacer4_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout14_3->addItem( spacer4_3 );
    layout15->addLayout( layout14_3 );
    groupBox5Layout->addLayout( layout15 );

    RenameSeriesLayout->addMultiCellWidget( groupBox5, 0, 3, 2, 2 );

    GroupBox5 = new QGroupBox( page, "GroupBox5" );
    GroupBox5->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, GroupBox5->sizePolicy().hasHeightForWidth() ) );
    GroupBox5->setColumnLayout(0, Qt::Vertical );
    GroupBox5->layout()->setSpacing( 6 );
    GroupBox5->layout()->setMargin( 11 );
    GroupBox5Layout = new QVBoxLayout( GroupBox5->layout() );
    GroupBox5Layout->setAlignment( Qt::AlignTop );

    CheckBox7 = new QCheckBox( GroupBox5, "CheckBox7" );
    GroupBox5Layout->addWidget( CheckBox7 );

    kDatePicker = new KDateWidget( GroupBox5, "kDatePicker" );
    kDatePicker->setEnabled( FALSE );
    GroupBox5Layout->addWidget( kDatePicker );

    RenameSeriesLayout->addWidget( GroupBox5, 3, 0 );

    GroupBox9 = new QGroupBox( page, "GroupBox9" );
    GroupBox9->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)4, 0, 0, GroupBox9->sizePolicy().hasHeightForWidth() ) );
    GroupBox9->setColumnLayout(0, Qt::Vertical );
    GroupBox9->layout()->setSpacing( 6 );
    GroupBox9->layout()->setMargin( 11 );
    GroupBox9Layout = new QHBoxLayout( GroupBox9->layout() );
    GroupBox9Layout->setAlignment( Qt::AlignTop );

    TextLabel2_2 = new QLabel( GroupBox9, "TextLabel2_2" );
    GroupBox9Layout->addWidget( TextLabel2_2 );

    spinIndex = new QSpinBox( GroupBox9, "spinIndex" );
    spinIndex->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0, spinIndex->sizePolicy().hasHeightForWidth() ) );
    spinIndex->setMaxValue( 99999999 );
    GroupBox9Layout->addWidget( spinIndex );
    spacer12 = new QSpacerItem( 61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    GroupBox9Layout->addItem( spacer12 );

    RenameSeriesLayout->addWidget( GroupBox9, 1, 0 );

    groupBox4 = new QGroupBox( page, "groupBox4" );
    groupBox4->setColumnLayout(0, Qt::Vertical );
    groupBox4->layout()->setSpacing( 6 );
    groupBox4->layout()->setMargin( 11 );
    groupBox4Layout = new QGridLayout( groupBox4->layout() );
    groupBox4Layout->setAlignment( Qt::AlignTop );

    paternLineEdit = new QLineEdit( groupBox4, "paternLineEdit" );
    paternLineEdit->setMinimumSize( QSize( 100, 0 ) );
    paternLineEdit->setCursor( QCursor( 0 ) );

    groupBox4Layout->addWidget( paternLineEdit, 0, 0 );

    EXIFButton = new QPushButton( groupBox4, "EXIFButton" );

    groupBox4Layout->addWidget( EXIFButton, 0, 1 );

    overwrite = new QCheckBox( groupBox4, "overwrite" );

    groupBox4Layout->addMultiCellWidget( overwrite, 2, 2, 0, 1 );

    checkExtension = new QCheckBox( groupBox4, "checkExtension" );
    checkExtension->setChecked( TRUE );

    groupBox4Layout->addMultiCellWidget( checkExtension, 1, 1, 0, 1 );

    RenameSeriesLayout->addWidget( groupBox4, 0, 0 );

    ButtonGroup1 = new QButtonGroup( page, "ButtonGroup1" );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 6 );
    ButtonGroup1->layout()->setMargin( 11 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );

    optionCopy = new QRadioButton( ButtonGroup1, "optionCopy" );

    ButtonGroup1Layout->addMultiCellWidget( optionCopy, 0, 0, 0, 1 );

    optionMove = new QRadioButton( ButtonGroup1, "optionMove" );

    ButtonGroup1Layout->addMultiCellWidget( optionMove, 1, 1, 0, 1 );

    dirname = new QLineEdit( ButtonGroup1, "dirname" );
    dirname->setEnabled( FALSE );

    ButtonGroup1Layout->addWidget( dirname, 3, 0 );

    buttonChooseDir = new QPushButton( ButtonGroup1, "buttonChooseDir" );
    buttonChooseDir->setEnabled( FALSE );
    //buttonChooseDir->setPixmap( image1 );
    buttonChooseDir->setFlat( FALSE );

    ButtonGroup1Layout->addWidget( buttonChooseDir, 3, 1 );

    optionRename = new QRadioButton( ButtonGroup1, "optionRename" );
    optionRename->setChecked( TRUE );

    ButtonGroup1Layout->addMultiCellWidget( optionRename, 2, 2, 0, 1 );

    RenameSeriesLayout->addWidget( ButtonGroup1, 2, 0 );

    Line2 = new QFrame( page, "Line2" );
    Line2->setFrameShape( QFrame::VLine );
    Line2->setFrameShadow( QFrame::Sunken );
    Line2->setFrameShape( QFrame::VLine );

    RenameSeriesLayout->addMultiCellWidget( Line2, 0, 3, 1, 1 );
    languageChange();
    page->resize( QSize(464, 450).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( CheckBox7, SIGNAL( toggled(bool) ), kDatePicker, SLOT( setEnabled(bool) ) );
    connect( optionCopy, SIGNAL( toggled(bool) ), dirname, SLOT( setEnabled(bool) ) );
    connect( optionCopy, SIGNAL( toggled(bool) ), buttonChooseDir, SLOT( setEnabled(bool) ) );
    connect( optionMove, SIGNAL( toggled(bool) ), dirname, SLOT( setEnabled(bool) ) );
    connect( optionMove, SIGNAL( toggled(bool) ), buttonChooseDir, SLOT( setEnabled(bool) ) );
    connect( optionRename, SIGNAL( toggled(bool) ), dirname, SLOT( setDisabled(bool) ) );
    connect( optionRename, SIGNAL( toggled(bool) ), buttonChooseDir, SLOT( setDisabled(bool) ) );

    // tab order
    setTabOrder( paternLineEdit, EXIFButton );
    setTabOrder( EXIFButton, checkExtension );
    setTabOrder( checkExtension, overwrite );
    setTabOrder( overwrite, spinIndex );
    setTabOrder( spinIndex, optionCopy );
    setTabOrder( optionCopy, optionMove );
    setTabOrder( optionMove, optionRename );
    setTabOrder( optionRename, dirname );
    setTabOrder( dirname, buttonChooseDir );
    setTabOrder( buttonChooseDir, CheckBox7 );
    setTabOrder( CheckBox7, previewCheckBox );




////
	connect( previewCheckBox, SIGNAL( stateChanged(int) ), this, SLOT( slotSetImagePreview(int) ) );
	connect( remanedPreviewListView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotUpdatePreview(QListViewItem*) ) );
	connect( moveUpPushButton, SIGNAL( clicked() ), this, SLOT( slotMoveUp() ) );
	connect( moveDownPushButton, SIGNAL( clicked() ), this, SLOT( slotMoveDown() ) );
	connect( buttonChooseDir, SIGNAL( clicked()), this, SLOT( chooseDir() ));
	connect( previewCheckBox, SIGNAL( stateChanged(int) ), this, SLOT( slotSetImagePreview(int) ) );
	connect( previewCheckBox, SIGNAL( toggled(bool) ), PixmapLabel, SLOT( setEnabled(bool) ) );
	connect( remanedPreviewListView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotUpdatePreview(QListViewItem*) ) );

	//
	connect( spinIndex,   SIGNAL( valueChanged( int ) ), this, SLOT( slotUpdateRenamed() ) );
	connect( checkExtension,   SIGNAL( toggled ( bool ) ), this, SLOT( slotUpdateRenamed() ) );
	//
	connect( paternLineEdit, SIGNAL(  textChanged ( const QString& ) ), this, SLOT( slotUpdateRenamed(const QString&)) );
	connect( paternLineEdit, SIGNAL( returnPressed() ), this, SLOT( slotOk() ) );

	//
	connect ( EXIFButton, SIGNAL( clicked() ), SLOT( EXIFButtonClicked() ) );

	//
	remanedPreviewListView->setSorting(-1);
	paternLineEdit->setFocus();

	pix = new QPixmap(180, 160);
	pix->fill(this, 0, 0);
	PixmapLabel->setPixmap(QPixmap(*pix));
	PixmapLabel->setScaledContents( TRUE );
	PixmapLabel->setMaximumSize( QSize(180, 160) );

	paternLineEdit->setText( i18n( "Pattern_#" ) );
	paternLineEdit->setMinimumSize(paternLineEdit->sizeHint());

	buttonChooseDir->setPixmap(QPixmap(BarIcon("folder_open",16 )));

	moveUpPushButton->setFixedWidth(16);
	moveDownPushButton->setFixedWidth(16);

	last=NULL;
	currentItem=NULL;

	clear();

	m_progressDialog = new ProgressDialog( parentWidget(), true, "renamed ProgressDialog", false );
	m_batchRenamer = new BatchRenamer(m_progressDialog);

	EXIFpopup = new QPopupMenu(this);
	EXIFpopup->insertItem (i18n("Options..."));
	EXIFpopup->insertSeparator (  );
	QStringList keys = m_batchRenamer->getKeys();
	for( unsigned int i = 0; i < keys.count(); i++ )
	{
		EXIFpopup->insertItem (keys[i]);
	}
	connect (EXIFpopup, SIGNAL(activated ( int )),
		this, SLOT(EXIFpopupMenuClicked( int )));


	setDateFormat( KGlobal::locale()->dateFormatShort());
	setTimeFormat( KGlobal::locale()->timeFormat());


        languageChange();

	setHelp("batchRename.anchor", "showimg");

	resize( QSize(640, 480).expandedTo(minimumSizeHint()) );

}

/*
 *  Destroys the object and frees any allocated resources
 */
RenameSeries::~RenameSeries()
{
	delete(m_batchRenamer);
	delete(m_progressDialog);
}


void RenameSeries::clear()
{
	arrayNames = QMemArray<QString*>();
	taille=0;
	last=NULL;
	currentItem=NULL;

	remanedPreviewListView->clear();

	previewCheckBox->setChecked(false);

// 	paternLineEdit->setText( i18n( "Pattern_#" ) );
	spinIndex->setValue(1);
}


void RenameSeries::chooseDir()
{
	QString s=KFileDialog::getExistingDirectory( QString::null,
								this,
								i18n("Please give a destination directory."));
 	if(!s.isEmpty())
 		dirname->setText( s );
}


void RenameSeries::addFile(const QString& filename)
{
	int pos = filename.findRev ("/");
	QString name = filename.right (filename.length () - pos-1);

	QListViewItem *item = new QListViewItem( remanedPreviewListView, last);
	last=item;
	item->setText( 0,  name );
	item->setText( 1,  paternLineEdit->text()+QString().setNum(taille));
	taille++;
	QString order;
	order.sprintf ("%0300d",taille);
	item->setText( 2,  order);

	arrayNames.resize(taille);
	arrayNames[taille-1]=new QString(filename);
}

void RenameSeries::slotMoveDown()
{
	remanedPreviewListView->disconnect(this);

	if(currentItem)
	{
		QListViewItem* apres=currentItem->itemBelow();
		if(currentItem->itemBelow ())
		{
			QString sBelow=apres->text(0);
			apres->setText(0, currentItem->text(0));
			currentItem->setText(0, sBelow);

			remanedPreviewListView->setCurrentItem(apres);
			remanedPreviewListView->setSelected(apres, true);
			currentItem = apres;

			////////
			int indice =(int)((float)remanedPreviewListView->itemPos(currentItem)/currentItem->height())-1;

			QString* tmp = arrayNames[indice+1];
			arrayNames[indice+1] = arrayNames[indice];
			arrayNames[indice]=tmp;
		}
	}
	connect( remanedPreviewListView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotUpdatePreview(QListViewItem*) ) );
	slotUpdateRenamed();
}

void RenameSeries::slotMoveUp()
{
	remanedPreviewListView->disconnect(this);
	if(currentItem)
		if(currentItem->itemAbove())
		{
			QListViewItem* avant=currentItem->itemAbove();
			if(avant)
			{
				QString sBelow=avant->text(0);
				avant->setText(0, currentItem->text(0));
				currentItem->setText(0, sBelow);

				remanedPreviewListView->setCurrentItem(avant);
				remanedPreviewListView->setSelected(avant, true);
				currentItem=avant;

				//////
				int indice =(int)((float)remanedPreviewListView->itemPos(currentItem)/currentItem->height()+1);

				QString* tmp = arrayNames[indice-1];
				arrayNames[indice-1] = arrayNames[indice];
				arrayNames[indice]=tmp;
			}
		}
	connect( remanedPreviewListView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotUpdatePreview(QListViewItem*) ) );
	slotUpdateRenamed();
}

void RenameSeries::slotSetImagePreview(int)
{
	if(! previewCheckBox->isChecked())
	{
		PixmapLabel->setPixmap(QPixmap(*pix));
	}
	else
	{
		if(!currentItem)
			return;

		KApplication::setOverrideCursor (waitCursor);	  // this might take time

		int indice =(int)((float)remanedPreviewListView->itemPos(currentItem)/currentItem->height());

		QImage im = QImage(*arrayNames[indice]);
		im = im.smoothScale(PixmapLabel->width(), PixmapLabel->height());
		QPixmap pix;
		pix.convertFromImage(im);
		PixmapLabel->setPixmap(pix);

		KApplication::restoreOverrideCursor ();   // restore original cursor
	}
}

void RenameSeries::slotUpdatePreview(QListViewItem* item)
{
	currentItem = item;

	if(previewCheckBox->isChecked())
	{
		KApplication::setOverrideCursor (waitCursor);	    // this might take time

		int indice =(int)((float)remanedPreviewListView->itemPos(item)/item->height());
		QImage im = QImage(*arrayNames[indice]);
		im = im.smoothScale(PixmapLabel->width(), PixmapLabel->height());
		QPixmap pix;
		pix.convertFromImage(im);
		PixmapLabel->setPixmap(pix);

		KApplication::restoreOverrideCursor ();     // restore original cursor
	}
}

int RenameSeries::exec ()
{
	slotUpdateRenamed();
	return KDialogBase::exec();
}

void RenameSeries::slotUpdateRenamed()
{
	slotUpdateRenamed(NULL);
}

void RenameSeries::slotUpdateRenamed(const QString&)
{
	QString fileName, tmp;
	QListViewItem * myChild = remanedPreviewListView->firstChild();
	int pos=0;
	QString text;
	QFileInfo fi;
	while( myChild )
    	{
		 fileName = QFileInfo(myChild->text(0)).baseName();

		fileName = m_batchRenamer->doEscape( fileName );

		//text = fileName;
		tmp = m_batchRenamer->findBrackets( fileName,
			paternLineEdit->text(),
			QFileInfo(*arrayNames[pos]).absFilePath() );

		tmp = m_batchRenamer->findOldName( fileName, tmp );
		tmp = m_batchRenamer->findOldNameLower( fileName, tmp );
		tmp = m_batchRenamer->findOldNameUpper( fileName, tmp );
		tmp = m_batchRenamer->findStar( fileName, tmp );
		tmp = m_batchRenamer->findNumbers( tmp, pos, arrayNames.size(), spinIndex->value());

		// Add Extension if necesary
		tmp = m_batchRenamer->unEscape( tmp );
		 if(checkExtension->isChecked())
		 {
			fi.setFile( *arrayNames[pos] );
			if( !fi.extension().isEmpty())
			{
				tmp+="."+fi.extension();
			}
		 }

		 myChild->setText(1,tmp);

		 pos++;
		 myChild = myChild->nextSibling();
	}
}

bool RenameSeries::checkErrors(bool checkDir)
{
	// Error checking:
	QDir d;
	if( paternLineEdit->text().stripWhiteSpace().isEmpty() ) {
		KMessageBox::sorry( this, i18n("Please add an expression to rename the files.") );
		return false;
	}
	else
	if( (checkDir) && (dirname->text().isEmpty() && !optionRename->isChecked()) )
	{
		KMessageBox::sorry( this, i18n("Please give a destination directory.") );
		return false;
	}
	else
	if((3+paternLineEdit->text().find('#')+paternLineEdit->text().find('$')+paternLineEdit->text().find('%'))==0)
	{
	 	 KMessageBox::sorry( this, i18n("The expression must contain '#', '$', or '%%'.") );
	 	 return false;
	}
	else
	if( (checkDir) && (!optionRename->isChecked()) )
	{
		if( ! dirname->text().endsWith("/") )
		{
			dirname->setText(dirname->text()+"/");
		}
	}
	else
	if( (checkDir) && (!optionRename->isChecked()) )
	{
		if( d.exists( dirname->text(), FALSE ))
		{
			KMessageBox::sorry( this, i18n( "The given destination directory does not exist." ) );
			return false;
		}
	}
	return true;

}

QString
RenameSeries::getPath( const QString& fn )
{
	int pos = fn.findRev( "/" );
	return fn.mid( 0, pos + 1 );
}



void
RenameSeries::slotOk()
{
	if(!checkErrors())
		return;

	QFileInfo fi;
	data* files = new data[arrayNames.size()];
	values* val = new values;

	m_progressDialog->init(arrayNames.size());
	m_progressDialog->print(i18n("Renaming %n file...", "Renaming %n files...", arrayNames.size()));

	for(unsigned int i=0; i<arrayNames.size();i++)
	{
		fi.setFile( *arrayNames[i] );
		files[i].source = fi.baseName();
		files[i].extension = fi.extension();
		files[i].count = arrayNames.size();
		if(! files[i].extension.isEmpty() )
			files[i].extension.insert( 0, '.' );

	    files[i].source_path = getPath(fi.filePath());
	}

	//
	enum mode m = RENAME;
	if( optionCopy->isChecked() )
	{
	    m = COPY;
	}
	else
	if( optionMove->isChecked() )
	{
	    m = MOVE;
	}
	else
	if( optionRename->isChecked() )
	{
	    m = RENAME;
	}

	//
	val->text = paternLineEdit->text();
	val->dirname = dirname->text();
	val->dvals.date = kDatePicker->date();
	val->index = spinIndex->value();
	val->extension = checkExtension->isChecked();
	val->overwrite = overwrite->isChecked();
	val->dvals.bDate = CheckBox7->isChecked();
	if( val->dvals.bDate )
	{
		val->dvals.changeModification = true;
		val->dvals.changeAccess = true;
		val->dvals.hour = 0;
		val->dvals.minute = 0;
		val->dvals.second = 0;
	}

	hide();
	m_progressDialog->show();
	m_batchRenamer->processFiles( files, m, val, FALSE);
}


void
RenameSeries::EXIFButtonClicked()
{
    EXIFpopup->move( mapToGlobal( EXIFButton->geometry().bottomLeft() ) );
    EXIFpopup->show();

}

void
RenameSeries::EXIFpopupMenuClicked(int pos)
{
	if(EXIFpopup->text ( pos )==i18n("Options..."))
	{
		DateTimeOption *formatOption = new DateTimeOption(this);
		formatOption->setDateFormat( getDateFormat() );
		formatOption->setTimeFormat( getTimeFormat() );
		if(formatOption->exec())
		{
			setDateFormat(formatOption->getDateFormat());
			setTimeFormat(formatOption->getTimeFormat());
		}
	}
	else
		paternLineEdit->insert ( "["+EXIFpopup->text ( pos ) +"]");
}

void
RenameSeries::setDateFormat(const QString& format)
{
	m_batchRenamer->setDateFormat(format);
	slotUpdateRenamed();
}
QString
RenameSeries::getDateFormat()
{
	return m_batchRenamer->getDateFormat();
}
void
RenameSeries::setTimeFormat(const QString& format)
{
	m_batchRenamer->setTimeFormat(format);
	slotUpdateRenamed();
}
QString
RenameSeries::getTimeFormat()
{
	return m_batchRenamer->getTimeFormat();
}
/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void RenameSeries::languageChange()
{
    setCaption( i18n( "Rename Series" ) );
    QWhatsThis::add( this, i18n(
"<u>Description:</u><br>\n"
"\n"
"Add an expression describing the final filename. Valid tokens are:<br>\n"
"<br>\n"
"<b>$</b> old filename<br><b>%</b> old filename converted to lower case<br><b>Both old filenames are without the old file extension.</b><br>\n"
"<br>\n"
"<b>#</b> Adds a number to the filename starting with StartIndex.<br><b>Example:</b>pic_$_#.jpg ." )
);
    EXIFButton->setText( i18n( "EXIF" ) );
    QToolTip::add( EXIFButton, i18n( "Insert EXIF tags" ));
    //GroupBox1->setTitle( i18n( "Options" ) );
    checkExtension->setText( i18n( "&Use original file extension" ) );
    QToolTip::add( checkExtension, i18n( "This option should be enabled." ) );
    QWhatsThis::add( checkExtension, i18n( "<u>Description:</u><br>\n"
"The original file extension will be added to the final filename. Useful if you want to rename many files of different filetypes." ) );
    overwrite->setText( i18n( "Overwrite &existing file" ) );
    paternLineEdit->setText( i18n( "Pattern_#" ) );
    GroupBox9->setTitle( i18n( "Numbers" ) );
    QWhatsThis::add( GroupBox9, i18n( "<u>Description:</u><br>\n"
"<b>This field is only necessary if you added a \"#\" to the filename.</b><br>\n"
"The first file will get the number start index, the second file start index + 1 and so on; for example, if your file name is picture#, the files will be named picture1, picture2, ... .<br>\n"
"<br>\n"
"\"Fill with 0's\" adds 0 before the number if you have more than 9 files to rename. E.g. picture01,picture02, ... picture10." ) );
    TextLabel2_2->setText( i18n( "Start index:" ) );
    QToolTip::add( spinIndex, i18n( "Start index, if numbers (#) are used in the filename." ) );
    groupBox4->setTitle( i18n( "Rename patern" ) );
    ButtonGroup1->setTitle( i18n( "Type" ) );
    QWhatsThis::add( ButtonGroup1, i18n( "<u>Description:</u><br>This option should be self explanatory.<br>  <br><b>Warning:</b> moving files does not work between different partitions but you can copy them." ) );
    optionCopy->setText( i18n( "Cop&y files to destination directory" ) );
    QToolTip::add( optionCopy, i18n( "Your files will be copied<br>to the destination directory and then renamed." ) );
    QWhatsThis::add( optionCopy, QString::null );
    optionMove->setText( i18n( "&Move files to destination directory" ) );
    QToolTip::add( optionMove, i18n( "Your files will be moved<br>to the destination directory and then renamed." ) );
    buttonChooseDir->setText( QString::null );
    QToolTip::add( buttonChooseDir, i18n( "Select a directory where<br>your files should be moved or copied." ) );
    optionRename->setText( i18n( "Re&name input files" ) );
    QToolTip::add( optionRename, i18n( "Your files will be copied<br>to the destination directory and then renamed." ) );
    GroupBox5->setTitle( i18n( "Date" ) );
    CheckBox7->setText( i18n( "Ch&ange date && time" ) );
    QToolTip::add( CheckBox7, i18n( "Changes the creation date and time of files" ) );
    QToolTip::add( kDatePicker, i18n( "Pick a date" ) );
    groupBox5->setTitle( i18n( "Renamed Preview" ) );
    remanedPreviewListView->header()->setLabel( 0, i18n( "Origin" ) );
    remanedPreviewListView->header()->setLabel( 1, i18n( "Renamed" ) );
    remanedPreviewListView->clear();

    moveUpPushButton->setText( i18n( "&up" ) );
    moveDownPushButton->setText( i18n( "&down" ) );
    QToolTip::add( moveDownPushButton, i18n( "Move item down" ) );
    QWhatsThis::add( moveDownPushButton, i18n( "Move the selected item down" ) );
    previewCheckBox->setText( i18n( "Show image &preview" ) );

////
    setButtonText(KDialogBase::Ok, i18n( "&Rename" ));
}

void
RenameSeries::writeConfig(KConfig *config, const QString& group)
{
	config->setGroup(group);
	config->writeEntry( "Date format", getDateFormat() );
	config->writeEntry( "Time format", getTimeFormat() );

	config->writeEntry( "Pattern", paternLineEdit->text() );
	config->writeEntry( "Dest dir", dirname->text() );
}

void
RenameSeries::readConfig(KConfig *config, const QString& group)
{
	config->setGroup(group);
	setDateFormat(config->readEntry("Date format", getDateFormat()));
	setTimeFormat(config->readEntry("Time format", getTimeFormat()));

	paternLineEdit->setText( config->readEntry("Pattern", i18n( "Pattern_#" ) ) );
	dirname->setText( config->readEntry("Dest dir", QDir::homeDirPath() ) );

}
#include "rename.moc"
