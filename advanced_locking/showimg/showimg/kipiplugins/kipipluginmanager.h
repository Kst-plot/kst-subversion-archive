/***************************************************************************
                          kipipluginmanager.h  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2003-2005 Renchi Raju
                               2004-2005 by Richard Groult
    email                : renchi@pooh.tam.uiuc.edu
                           rgroult@jalix.org
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

#ifndef KIPIPLUGINMANAGER_H
#define KIPIPLUGINMANAGER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_KIPI
#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local
#include <mainwindow.h>

// Qt
#include <qptrlist.h>

class KAction;
class ShowImgKIPIInterface;

class KIPIPluginManager : public QObject
{
public:

    KIPIPluginManager(MainWindow *parent);
    ~KIPIPluginManager();

    void loadPlugins();

    static KIPIPluginManager* instance();
    const  QPtrList<KAction>& menuMergeActions();
    KAction* action(const QString actionName);

    void selectionChanged( bool b );
    void currentAlbumChanged( const QString path );

private:
    MainWindow                   *parent_;
    static KIPIPluginManager*    instance_;
    QPtrList<KAction>            menuMergeActions_;
    QStringList                  availablePluginList_;


    QPtrList<KAction>      m_kipiFileActionsExport;
    QPtrList<KAction>      m_kipiFileActionsImport;
    QPtrList<KAction>      m_kipiImageActions;
    QPtrList<KAction>      m_kipiToolsActions;
    QPtrList<KAction>      m_kipiBatchActions;
    QPtrList<KAction>      m_kipiAlbumActions;


    ShowImgKIPIInterface* ShowImgKIPIInterface_;

    void initAvailablePluginList();

    KIPI::Plugin*  pluginIsLoaded(const QString pluginName);
    QPtrList<KIPI::Plugin>    pluginList_;
};

#endif /* HAVE_KIPI */

#endif /* KIPIPLUGINMANAGER_H */
