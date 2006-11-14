/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <QtDBus>

#include <kapplication.h>
#include <kimageio.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kiconloader.h>

#include "kgrabadaptor.h"
#include "kgrab.h"

#define KGRABVERSION "0.1"

static const char description[] = I18N_NOOP("KDE Screen Grabbing Utility");

static KCmdLineOptions options[] =
{
    { "c", 0, 0 },
    { "current", I18N_NOOP("Captures the window under the mouse on startup (instead of the desktop)"), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
  KAboutData aboutData( "kgrab", I18N_NOOP("KGrab"),
    KGRABVERSION, description, KAboutData::License_GPL,
    "(c) 1997-2004, Richard J. Moore,\n(c) 2000, Matthias Ettrich,\n(c) 2002-2003 Aaron J. Seigo\n(c) 2006 Marcus Hufgard ");
  aboutData.addAuthor( "Marcus Hufgard", I18N_NOOP("Maintainer"), "marcus.hufgard@hufgard.de" );
  aboutData.addAuthor( "Richard J. Moore","KSnapshot", "rich@kde.org" );
  aboutData.addAuthor( "Matthias Ettrich","KSnapshot", "ettrich@kde.org" );
  aboutData.addAuthor( "Aaron J. Seigo", "KSnaphsot", "aseigo@kde.org" );
  aboutData.addCredit( "Nadeem Hasan", I18N_NOOP("On KSnapshot\nRegion Grabbing, Reworked GUI"), "nhasan@kde.org" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication app;

  // Create top level window
  KGrab *toplevel;

  if ( args->isSet( "current" ) )
     toplevel = new KGrab( 0, true );
  else
     toplevel = new KGrab();

  new KgrabAdaptor(toplevel);
  QDBusConnection::sessionBus().registerObject("/KGrab", toplevel);
  toplevel->setCaption( app.makeStdCaption("") );
  toplevel->show();
  return app.exec();
}
