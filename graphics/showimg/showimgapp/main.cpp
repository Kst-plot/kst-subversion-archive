/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult, 2003 OGINO Tomonori
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
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

// Local
#include <showimg/kstartuplogo.h>
#include <showimg/mainwindow.h>

// KDE
#include <kimageio.h>
#include <kprocess.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <kdebug.h>
#include <dcopclient.h>

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

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

static MainWindow *frame=NULL;

static const char* SHOWIMG_DATE="0.9.5 2005 05 15";

class ShowImgApplication : public KUniqueApplication
{
public:
	ShowImgApplication() : KUniqueApplication() { };

	virtual int newInstance()
	{
		// process args:
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		if(frame)
		{
			MYDEBUG<<args->count()<<endl;
			if (args->count() == 1)
			{
				bool forceFullscreen = args->isSet("fullscreen");
				bool wantFullscreen = forceFullscreen && args->getOption("fullscreen")=="on" ;

				if(!args->isSet("slideshow") && !wantFullscreen)
					frame->instance()->openDir(QFile::decodeName(args->arg (0)));
				else
				{
					frame->instance()->openDir(QFile::decodeName(args->arg (0)), true, false);
					frame->instance()->slotFullScreen();
				}
			}

#ifndef Q_WS_WIN
			KWin::deIconifyWindow(frame->instance()->winId());
			KWin::setOnDesktop(frame->instance()->winId(), KWin::currentDesktop());
			KWin::activateWindow(frame->instance()->winId());
			KWin::raiseWindow(frame->instance()->winId());
#endif
			if(args->isSet("slideshow"))
			{
				if (!args->getOption("slideshow").isEmpty())
					frame->instance()->slotSlideShow();
				else
					frame->instance()->slotSlideShow(args->getOption("slideshow").toInt());
			}
		}
		args->clear();
		return 0;
 	 };
};
// class ShowImgApplication : public KApplication
// {
// public:
// 	ShowImgApplication() : KApplication()
// 	{
// 	};
// };

int
main (int argc, char **argv)
{
	KAboutData*  aboutData = new KAboutData("showimg", I18N_NOOP("showimg"),
	        	      SHOWIMG_DATE, description, KAboutData::License_GPL,
	        	      "(c) 2001-2004, Richard Groult", I18N_NOOP("(Please e-mail any bug reports...\nor encouragements to me :)"),
	        	      "http://www.jalix.org/projects/showimg/",
	        	      "rgroult@jalix.org" );

	aboutData->addAuthor ("Richard Groult", I18N_NOOP("Developer"), "rgroult@jalix.org", "http://ric.jalix.org");
	aboutData->addAuthor ("Dominik Seichter", I18N_NOOP("to allow me to use his great software krename for showimg, and patch writing"), "domseichter@web.de", "http://www.freshmeat.net/projects/krename" );
	aboutData->addAuthor ("Tomonori Ogino", I18N_NOOP("patch for archives"), "ogino@nn.iij4u.or.jp");

	aboutData->addCredit ("Valérie", I18N_NOOP("for the original logo"));
	aboutData->addCredit ("Benoist", I18N_NOOP("Beta tester, translation, and help for the zoom feature"), "b.gaston@laposte.net");
	aboutData->addCredit ("Romain Vallet", I18N_NOOP("Documentation and spelling corrector"), "rom@jalix.org");
	aboutData->addCredit ("Guillaume Duhamel",I18N_NOOP("Beginning of Japanese translation and beginning of the http/ftp protocol support") , "Guillaume.Duhamel@gmail.com");

	aboutData->addCredit ("Alain Bidaud and the Jalix team", I18N_NOOP("for the web site"));

	aboutData->addCredit ("André Pascual ", I18N_NOOP("for the current logo and additional icons"), "andre@linuxgraphic.org");
	aboutData->addCredit ("Jean-Philippe Martin",I18N_NOOP("for icons and advice about features and design") , "jeanphilippemartin@club-internet.fr");

	aboutData->addCredit ("Hape Schaal ", I18N_NOOP("German translator and a great help for debugging"), "hp.news@gmx.de");

	aboutData->addCredit ("Jarosław Staniek", I18N_NOOP("MS Windows version, GUI improvements"), "js@iidea.pl", "http://www.kexi-project.org/pjs.html");

	aboutData->addCredit ("Matthias Wandel", I18N_NOOP("to allow me to use jhead for JPEG-EXIF format support"), "MWandel@rim.net");
	aboutData->addCredit ("Andrew Richards", I18N_NOOP("to allow me to use his printImageDialog class"), "ajr@users.sourceforge.net");

	aboutData->addCredit ("Lots of people", I18N_NOOP("for bugs reports, advice\nand encouragement :)"));

	//
	KCmdLineArgs::init (argc, argv, aboutData);
	KCmdLineArgs::addCmdLineOptions (options);
	ShowImgApplication::addCmdLineOptions();

	bool alreadyRunning=!ShowImgApplication::start();
	if (alreadyRunning)
	{
		kdWarning() << "ShowImg is already running!" << endl;
		return 0;
	}

	ShowImgApplication kApp;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs ();

	//--------------------------------------------------------------------
	KConfig *config = KGlobal::config();
	config->setGroup("Options");
	KStartupLogo* logo=NULL;

	bool forceFullscreen = args->isSet("fullscreen");
	bool wantFullscreen = forceFullscreen && args->getOption("fullscreen")=="on" ;

	if( ( ( args->isSet("splashscreen") &&  args->getOption("splashscreen")=="on") ||
			   ( !(args->isSet("splashscreen") &&  args->getOption("splashscreen")=="off") && config->readBoolEntry("showSP", true))
		)
		&& ! wantFullscreen
		&& ! (args->count() != 0 && QFileInfo(QFile::decodeName(args->arg(0))).isFile())
	  )
	{
		MYDEBUG<<endl;
		logo = new KStartupLogo();
		logo->show();
	}
	//--------------------------------------------------------------------

	MYDEBUG<<args->count()<<endl;
	if (args->count() == 0)
	{
		MYDEBUG<<args->count()<<endl;
		frame = new MainWindow ();
	}
	else
	{
		bool runSlideshow = args->isSet("slideshow");
		int timeSlideshow = args->getOption("slideshow").toInt();
// 		MYDEBUG << runSlideshow << " " << args->getOption("slideshow").toInt()<<endl;

		frame = new MainWindow (QFile::decodeName(args->arg(0)),
					forceFullscreen&&wantFullscreen, forceFullscreen,
					runSlideshow, timeSlideshow);
	}

	frame->setIcon(kApp.icon());
	kApp.setMainWidget (frame);

	//--------------------------------------------------------------------
	if(logo)
	{
		logo->hide();
		delete logo;
	}
	//--------------------------------------------------------------------

	args->clear();
	return kApp.exec ();
}
