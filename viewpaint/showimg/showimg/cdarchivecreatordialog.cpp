/***************************************************************************
                          cdarchivecreatordialog.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2005 by Richard Groult
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

#include "cdarchivecreatordialog.h"

// Local
#include "cdarchivecreator.h"
#include "cdarchive.h"

// KDE
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kstandarddirs.h>

// Qt
#include <qwidget.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

CDArchiveCreatorDialog::CDArchiveCreatorDialog( QString cdromPath, QWidget* parent, const char* name )
:KDialogBase( parent, name, true, "CDArchiveCreatorDialog" ,
 		    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
{
	if ( !name )
			setName( "CD Archive Creator Dialog" );
	QWidget *page = new QWidget( this );
	setMainWidget(page);
////
	CDArchiveCreatorDialogLayout = new QGridLayout( page, 1, 1, 11, 6, "CDArchiveCreatorDialogLayout");

	groupBox1 = new QGroupBox( page, "groupBox1" );
	groupBox1->setColumnLayout(0, Qt::Vertical );
	groupBox1->layout()->setSpacing( 6 );
	groupBox1->layout()->setMargin( 11 );
	groupBox1Layout = new QGridLayout( groupBox1->layout() );
	groupBox1Layout->setAlignment( Qt::AlignTop );

	textLabel1 = new QLabel( groupBox1, "textLabel1" );

	groupBox1Layout->addWidget( textLabel1, 1, 0 );

	textLabel2 = new QLabel( groupBox1, "textLabel2" );

	groupBox1Layout->addWidget( textLabel2, 3, 0 );

	cdRomPathLineEdit = new KLineEdit( groupBox1, "cdRomPathLineEdit" );
	cdRomPathLineEdit->setMinimumSize( QSize( 110, 0 ) );

	groupBox1Layout->addWidget( cdRomPathLineEdit, 1, 1 );

	archiveNameLineEdit = new KLineEdit( groupBox1, "archiveNameLineEdit" );

	groupBox1Layout->addMultiCellWidget( archiveNameLineEdit, 3, 3, 1, 2 );

	browseButton = new KPushButton( groupBox1, "browseButton" );

	groupBox1Layout->addWidget( browseButton, 1, 2 );

	CDArchiveCreatorDialogLayout->addWidget( groupBox1, 0, 0 );
	languageChange();
	page->resize( QSize(356, 111).expandedTo(minimumSizeHint()) );
	clearWState( WState_Polished );

	// signals and slots connections
	connect( browseButton, SIGNAL( clicked() ), this, SLOT( chooseDir() ) );
	connect( archiveNameLineEdit, SIGNAL( textChanged(const QString&) ), this, SLOT( textChanged(const QString&) ) );
	connect( cdRomPathLineEdit, SIGNAL( textChanged(const QString&) ), this, SLOT( textChanged(const QString&) ) );


	////
	enableButtonOK(false);

	m_progressDlg=NULL;
	ar=NULL;
	beginT = new QTime(0,0,0);

	cdRomPathLineEdit->setText(cdromPath);
}


CDArchiveCreatorDialog::~CDArchiveCreatorDialog()
{
	if(ar) ar->wait();
}

void
CDArchiveCreatorDialog::chooseDir()
{
	QString s=KFileDialog::getExistingDirectory(cdRomPathLineEdit->text(),
								this,
								i18n("Specified Directory"));
 	if(!s.isEmpty())
 	{
		cdRomPathLineEdit->setText( s );
		archiveNameLineEdit->setFocus();
	}
}

void
CDArchiveCreatorDialog::textChanged ( const QString& )
{
	if(KStandardDirs::exists(cdRomPathLineEdit->text().stripWhiteSpace()+"/")
	&& !archiveNameLineEdit->text().stripWhiteSpace().isEmpty())
		enableButtonOK(true);
	else	enableButtonOK(false);
}

void
CDArchiveCreatorDialog::accept()
{
	MYDEBUG<<endl;
	QString cdArhivePath =
		KStandardDirs::realPath(cdRomPathLineEdit->text().stripWhiteSpace()+"/");
	QString cdArhiveName =
		archiveNameLineEdit->text().stripWhiteSpace();
	if(QFileInfo(CDArchive_ROOTPATH+"/"+cdArhiveName).dirPath() != CDArchive_ROOTPATH)
	{
		KMessageBox::error(this,
			"<qt>"+i18n("The archive file name '%1' is not correct").arg(cdArhiveName)+"</qt>",
			i18n("CD Archive Creation"));
		return;
	}

	ar = new CDArchiveCreator(this,
		cdArhivePath,
		cdArhiveName);
	connect(ar, SIGNAL(parseDirectoryDone()),
		this, SLOT(parseDirectoryDone()));

	setEnabled(false);
	ar->parseDirectory();
	return ;
}
void
CDArchiveCreatorDialog::parseDirectoryDone()
{
	MYDEBUG<<endl;
	ar->start();
}

void
CDArchiveCreatorDialog::customEvent(QCustomEvent *event)
{
	if (!event) return;
	EventData *d = (EventData*) event->data();
	if (!d) return;

	if (!m_progressDlg)
	{
		m_progressDlg = new KProgressDialog(this, "Find Duplicate Images Operations",
				i18n("CD Archive creation progress"),
				QString::null,
				true);
		m_current = 0;
		m_progressDlg->adjustSize();
		m_progressDlg->show();
	}
	if(m_progressDlg->wasCancelled())
	{
		slotCancel();
		return;
	}
	QString text;
	if (d->starting)
	{
		switch (d->action)
		{
		case(Parse):
			{
			text = i18n("Parsing")+ "<br>" + i18n("'%1'")
				.arg(d->fileName);
			break;
			}
		case(Progress):
			{
			text = i18n("Creating thumbnail for")+ "<br>" + i18n("'%1'")
				.arg(QFileInfo(d->fileName).fileName())
				+QString("<br>(%1/%2)").arg(m_current).arg(m_total);
			break;
			}
		case(Archive):
			{
			text = i18n("Archiving")+ "<br>" + i18n("'%1'")
				.arg(QFileInfo(d->fileName).fileName() );
			break;
			}
		case(Canceled):
			{
			text = d->errString;
			break;
			}
		}
	}
	else
	if (d->success)
	{
		switch (d->action)
		{
		case(Parse):
			{
			text = i18n("Parsing done")
				+ "\n"
				+ i18n("Preparing the thumbnail creation...");
			m_total = d->total;
			break;
			}
		case(Progress):
			{
			m_current++;
			if(m_current==1) beginT = new QTime(QTime::currentTime());
			break;
			}
		case(Archive):
			{
			text = i18n("Archiving '%1' done").arg(QFileInfo(d->fileName).fileName() );
			m_current=m_total+1;
			break;
			}
		case(Canceled):
			{
			text = d->errString;
			break;
			}
		}
	}
	if(d->action==Canceled)
	{
			delete m_progressDlg; m_progressDlg=NULL;
			delete(ar); ar=NULL;
			KMessageBox::error(this,
				"<qt>"+text+"</qt>",
				i18n("CD Archive Creation"));
			slotCancel();
	}
	else
	if(d->action==Archive && d->success)
	{
		m_progressDlg->hide();
		KMessageBox::information(this,
			"<qt>"+i18n("CD Archive '%1' created").arg(QFileInfo(d->fileName).fileName())+"</qt>",
			i18n("CD Archive Creation"));
		delete d;
		(void)KDialog::accept();
	}
	else
	{
		/*
		float elapsedT = beginT->secsTo(QTime::currentTime());
		QString remaining = QTime(0,0,0,0).addSecs((int)((elapsedT/(m_current+1))*(m_total-m_current+1))).toString("hh:mm:ss");
		m_progressDlg->progressBar()->setTotalSteps(m_total+1);
		m_progressDlg->progressBar()->setProgress(m_current);
		m_progressDlg->setLabel(QString("<qt><center>%1<br><br>%2 remaining</center>")
					.arg(text)
					.arg(remaining));
		*/
		m_progressDlg->progressBar()->setTotalSteps(m_total+1);
		m_progressDlg->progressBar()->setProgress(m_current);
		m_progressDlg->setLabel(QString("<qt><center>%1</center></qt>").arg(text));
		if(m_current==1) m_progressDlg->adjustSize();
		kapp->processEvents();
		delete d;
	}
}

void
CDArchiveCreatorDialog::slotCancel()
{
	if(ar && m_progressDlg)
	{
		//m_progressDlg->setLabel(QString("<qt><center>Aborting...</center></qt>"));
		//m_progressDlg->show();
		ar->terminate();
		ar->wait();
		delete m_progressDlg; m_progressDlg=NULL;
		KMessageBox::error(this,
			i18n("CD Archive creation aborted"),
			i18n("CD Archive Creation"));
	}
	setEnabled(true);
	(void)KDialogBase::reject();
	close();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CDArchiveCreatorDialog::languageChange()
{
	setCaption( i18n( "CD Archive Creator" ) );
	groupBox1->setTitle( i18n( "Options" ) );
	textLabel1->setText( i18n( "CD-ROM path:" ) );
	browseButton->setText( i18n( "Browse..." ) );
	textLabel2->setText( i18n( "Archive name:" ) );
}

#include "cdarchivecreatordialog.moc"
