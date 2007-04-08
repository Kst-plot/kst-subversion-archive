/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
                               2003 OGINO Tomonori
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
 ***************************************************************************/

/*****************************************************************************
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU        *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the Free Software             *
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301 *
 *   USA.                                                                    *
 *                                                                           *
 *   For license exceptions see LICENSE.EXC file, attached with the source   *
 *   code.                                                                   *
 *                                                                           *
 *****************************************************************************/

// Local
#include <showimg/kstartuplogo.h>
#include <showimg/mainwindow.h>

#ifdef HAVE_KIPI
#include <libkipi/version.h>
#endif
#ifdef WANT_LIBKEXIDB
#include "../kexi/kexi_version.h"
#endif /* WANT_LIBKEXIDB */

// KDE
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <kwin.h>

// Qt
#include <qpixmapcache.h>
#include <qfile.h>
#include <qfileinfo.h>

static const char *description =
	I18N_NOOP ("Viewer for your desktop");


static KCmdLineOptions options[] =
{
	{"+file", I18N_NOOP ("Name of the image file to view"), 0},
	{"+directory", I18N_NOOP ("Name of the directory to browse"), 0},
	{"fullscreen <on/off>", I18N_NOOP("Run in fullscreen"), 0},
	{"slideshow <time>", I18N_NOOP("Run slideshow with time sec"), 0},
	{"splashscreen <on/off>", I18N_NOOP("Show splashscreen on startup"), 0},
	KCmdLineLastOption
};

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

static MainWindow *frame=NULL;

static const char* SHOWIMG_VERSION_STRING="0.9.5";

/*
class ShowImgApplication : public KUniqueApplication
{
public:
	ShowImgApplication() : KUniqueApplication() { };

	virtual int newInstance()
	{
// 		process args:
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		if(frame)
		{
			MYDEBUG<<args->count()<<endl;
			if (args->count() == 1)
			{
				bool forceFullscreen = args->isSet("fullscreen");
				bool wantFullscreen = forceFullscreen && (QString)args->getOption("fullscreen")==QString::fromLatin1("on") ;

				if(!args->isSet("slideshow") && !wantFullscreen)
					MAINWINDOW.openDir(QFile::decodeName(args->arg (0)));
				else
				{
					MAINWINDOW.openDir(QFile::decodeName(args->arg (0)), true, false);
					MAINWINDOW.slotFullScreen();
				}
			}

#ifndef Q_WS_WIN
			KWin::deIconifyWindow(MAINWINDOW.winId());
			KWin::setOnDesktop(MAINWINDOW.winId(), KWin::currentDesktop());
#if KDE_IS_VERSION(3,3,0)
			KWin::activateWindow(MAINWINDOW.winId());
			KWin::raiseWindow(MAINWINDOW.winId());
#endif
#endif
			if(args->isSet("slideshow"))
			{
				if (!args->getOption("slideshow").isEmpty())
					MAINWINDOW.slotSlideShow();
				else
					MAINWINDOW.slotSlideShow(args->getOption("slideshow").toInt());
			}
		}
		args->clear();
		return 0;
 	 };
};
*/


 class ShowImgApplication : public KApplication
 {
 public:
 	ShowImgApplication() : KApplication()
 	{
 	};
 };

int
main (int argc, char **argv)
{
	QString madate = QString("%1").arg(SHOWIMG_VERSION_STRING);
#ifdef HAVE_KIPI
	madate = madate+QString(", kipi %1").arg(kipi_version);
#endif /* HAVE_KIPI */
#ifdef WANT_LIBKEXIDB
	madate = madate+QString(", kexi %1").arg(KEXI_VERSION_STRING);
#endif /* WANT_LIBKEXIDB */

	KAboutData*  aboutData = new KAboutData("showimg", I18N_NOOP("showimg"),
	        	      madate.ascii(), description, KAboutData::License_GPL,
	        	      "(c) 2001-2006, Richard Groult", I18N_NOOP("(Please e-mail any bug reports...\nor encouragements to me :)"),
	        	      "http://www.jalix.org/projects/showimg/",
	        	      "rgroult@jalix.org" );

	aboutData->addAuthor ("Richard Groult", I18N_NOOP("Developer"), "rgroult@jalix.org", "http://ric.jalix.org");
	aboutData->addAuthor ("Dominik Seichter", I18N_NOOP("to allow me to use his great software krename for showimg, and patch writing"), "domseichter@web.de", "http://www.freshmeat.net/projects/krename" );
	aboutData->addAuthor ("Tomonori Ogino", I18N_NOOP("patch for archives"), "ogino@nn.iij4u.or.jp");

	aboutData->addCredit ("Jarosław Staniek", I18N_NOOP("Help on KexiDB, MS Windows version, GUI improvements"), "js@iidea.pl", "http://kexi-project.org/");


	aboutData->addCredit ("Valérie", I18N_NOOP("for the original logo"));
	aboutData->addCredit ("Benoist", I18N_NOOP("Beta tester, translation, and help for the zoom feature"), "b.gaston@laposte.net");
	aboutData->addCredit ("Romain Vallet", I18N_NOOP("Documentation and spelling corrector"), "rom@jalix.org");
	aboutData->addCredit ("Guillaume Duhamel",I18N_NOOP("Beginning of Japanese translation and beginning of the http/ftp protocol support") , "Guillaume.Duhamel@gmail.com");

	aboutData->addCredit ("Alain Bidaud and the Jalix team", I18N_NOOP("for the web site"));

	aboutData->addCredit ("André Pascual ", I18N_NOOP("for the current logo and additional icons"), "andre@linuxgraphic.org");
	aboutData->addCredit ("Jean-Philippe Martin",I18N_NOOP("for icons and advice about features and design") , "jeanphilippemartin@club-internet.fr");

	aboutData->addCredit ("Hape Schaal ", I18N_NOOP("German translator and a great help for debugging"), "hp.news@gmx.de");

	aboutData->addCredit ("Matthias Wandel", I18N_NOOP("to allow me to use jhead for JPEG-EXIF format support"), "MWandel@rim.net");
	aboutData->addCredit ("Andrew Richards", I18N_NOOP("to allow me to use his printImageDialog class"), "ajr@users.sourceforge.net");

	aboutData->addCredit ("Lots of people", I18N_NOOP("for bugs reports, advice\nand encouragement :)"));

	//
	KCmdLineArgs::init (argc, argv, aboutData);
	KCmdLineArgs::addCmdLineOptions (options);
	ShowImgApplication::addCmdLineOptions();

    KGlobal::locale()->setMainCatalogue( "showimg" );

	bool alreadyRunning = false;//!ShowImgApplication::start();
	if (alreadyRunning)
	{
		kdWarning() << "ShowImg is already running!" << endl;
		return 0;
	}

	ShowImgApplication app;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs ();

	//--------------------------------------------------------------------
	KConfig *config = KGlobal::config();
	config->setGroup("Options");
	KStartupLogo* logo=NULL;

	bool forceFullscreen = args->isSet("fullscreen");
	bool wantFullscreen = forceFullscreen && (QString)args->getOption("fullscreen")==QString::fromLatin1("on") ;

	if( 
		( 
			(
				args->isSet("splashscreen") &&  
				(QString)args->getOption("splashscreen")==QString::fromLatin1("on")
			)
			||
			( 
				!(
					args->isSet("splashscreen") &&  
					(QString)args->getOption("splashscreen")==QString::fromLatin1("off")
				) 
				&& 
				config->readBoolEntry("showSP", true)
			)
		) && 
		! wantFullscreen && 
		! (
			args->count() != 0 && 
			QFileInfo(QFile::decodeName(args->arg(0))).isFile()
		)
	  )
	{
 		//logo = new KStartupLogo();
 		//logo->show();
	}
	//--------------------------------------------------------------------

	if (args->count() == 0)
	{
		frame = new MainWindow ();
	}
	else
	{
		bool runSlideshow = args->isSet("slideshow");
		int timeSlideshow = args->getOption("slideshow").toInt();

		frame = new MainWindow (args->arg(0),
					forceFullscreen&&wantFullscreen, forceFullscreen,
					runSlideshow, timeSlideshow);
	}
	args->clear();

	app.setMainWidget (frame);
	frame->show();

	//--------------------------------------------------------------------
	if(logo)
	{
		logo->hide();
		delete logo;
	}
	//--------------------------------------------------------------------

	return app.exec ();
}
