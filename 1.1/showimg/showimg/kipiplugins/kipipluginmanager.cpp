/***************************************************************************
                          kipipluginmanager.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2003 Renchi Raju 
                               2004 by Richard Groult
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

// Local 
#include "kipipluginmanager.h"

#ifdef HAVE_KIPI

// Local 
#include "kipiinterface.h"

// Qt 
#include <qfile.h>
#include <qvariant.h>
#include <qpopupmenu.h>

// KDE 
#include <ktrader.h>
#include <kxmlguifactory.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>


KIPIPluginManager::KIPIPluginManager(MainWindow *parent)
    : QObject(parent)
{
    instance_ = this;
    parent_=parent;
}

KIPIPluginManager::~KIPIPluginManager()
{
    instance_ = 0;
}

#ifdef HAVE_KIPI
void 
KIPIPluginManager::loadPlugins() 
{
    parent_->unplugActionList( QString::fromLatin1("file_actions_export") );
    parent_->unplugActionList( QString::fromLatin1("file_actions_import") );
    parent_->unplugActionList( QString::fromLatin1("image_actions") );
    parent_->unplugActionList( QString::fromLatin1("tool_actions") );
    parent_->unplugActionList( QString::fromLatin1("batch_actions") );
    parent_->unplugActionList( QString::fromLatin1("album_actions") );
    
    m_kipiImageActions.clear();
    m_kipiFileActionsExport.clear();
    m_kipiFileActionsImport.clear();
    m_kipiToolsActions.clear();
    m_kipiBatchActions.clear();
    m_kipiAlbumActions.clear();
    menuMergeActions_.clear();
       
    	// Sets up the plugin interface, and load the plugins
	ShowImgKIPIInterface_ = new ShowImgKIPIInterface(parent_);
	KIPI::PluginLoader* loader = new KIPI::PluginLoader(QStringList(), ShowImgKIPIInterface_ );
	loader->loadPlugins();

	KIPI::PluginLoader::PluginList list = loader->pluginList();
    
    for( KIPI::PluginLoader::PluginList::Iterator it = list.begin() ; it != list.end() ; ++it ) 
        {
        KIPI::Plugin* plugin = (*it)->plugin();
	pluginList_.append(plugin);
        
        if ( !plugin || !(*it)->shouldLoad() )
            continue;

        plugin->setup( parent_ );
        QPtrList<KAction>* popup = 0;
        
        // Plugin category identification using KAction method based.
        KActionPtrList actions = plugin->actions();
            
        for( KActionPtrList::Iterator it2 = actions.begin(); it2 != actions.end(); ++it2 ) 
            {
 	    
	    if((*it2)->text()==i18n("Rename Images..."))
	    {
		continue;
	    }
	    
		
            if ( plugin->category(*it2) == KIPI::IMAGESPLUGIN )
               popup = &m_kipiImageActions;

            else if ( plugin->category(*it2) == KIPI::EXPORTPLUGIN )
               popup = &m_kipiFileActionsExport;
        
            else if ( plugin->category(*it2) == KIPI::IMPORTPLUGIN )
               popup = &m_kipiFileActionsImport;

            else if ( plugin->category(*it2) == KIPI::TOOLSPLUGIN )
               popup = &m_kipiToolsActions;

            else if ( plugin->category(*it2) == KIPI::BATCHPLUGIN )
               popup = &m_kipiBatchActions;
        
            else if ( plugin->category(*it2) == KIPI::COLLECTIONSPLUGIN )
               popup = &m_kipiAlbumActions;
            else 
               popup = &m_kipiToolsActions;
                
            // Plug the KIPI plugins actions in according with the KAction method.   
                
            if ( popup )            
	    {
               popup->append( *it2 );
	       menuMergeActions_.append(*it2 );
	    }
            else 
               MYDEBUG << "No menu found for a plugin!!!" << endl;
            }
            

	       plugin->actionCollection()->readShortcutSettings();
	
	
        }
	
	
			
        
    // Create GUI menu in according with plugins.
    parent_->plugActionList( QString::fromLatin1("file_actions_export"), m_kipiFileActionsExport );
    parent_->plugActionList( QString::fromLatin1("file_actions_import"), m_kipiFileActionsImport );
    parent_->plugActionList( QString::fromLatin1("image_actions"), m_kipiImageActions );
    parent_->plugActionList( QString::fromLatin1("tool_actions"), m_kipiToolsActions );
    parent_->plugActionList( QString::fromLatin1("batch_actions"), m_kipiBatchActions );
    parent_->plugActionList( QString::fromLatin1("album_actions"), m_kipiAlbumActions );

	
}
#else
void KIPIPluginManager::loadPlugins() {
	QPopupMenu *popup = static_cast<QPopupMenu*>(
		factory()->container( "plugins", this));
	delete popup;
}
#endif



KIPI::Plugin*
KIPIPluginManager::pluginIsLoaded(const QString pluginName)
{
	if(pluginList_.isEmpty()) return NULL;

	KIPI::Plugin *plugin;
	for ( plugin = pluginList_.first(); plugin; plugin = pluginList_.next() )
	{
		if(plugin->name() == pluginName)
			return plugin;
	}
	return NULL;
}



void
KIPIPluginManager::initAvailablePluginList()
{
    KTrader::OfferList offers = KTrader::self()->query("KIPI/Plugin");
    KTrader::OfferList::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
	KService::Ptr service = *iter;
	availablePluginList_.append(service->name());
	availablePluginList_.append(service->comment());
    }
}

KAction*
KIPIPluginManager::action(const QString actionName)
{
	QPtrList<KAction> m_actionlist = menuMergeActions();
	for ( KAction* m_action = m_actionlist.first(); m_action; m_action=m_actionlist.next() )
	{
		if(m_action->text()==i18n(actionName.utf8()))
			return m_action;
	}
	return NULL;
}


const QPtrList<KAction>& KIPIPluginManager::menuMergeActions()
{
    return menuMergeActions_;
}

KIPIPluginManager* KIPIPluginManager::instance()
{
    return instance_;
}


KIPIPluginManager* KIPIPluginManager::instance_ = 0;


void 
KIPIPluginManager::selectionChanged( bool b )
{
	ShowImgKIPIInterface_->selectionChanged(b);
}

void 
KIPIPluginManager::currentAlbumChanged( const QString path )
{
	ShowImgKIPIInterface_->currentAlbumChanged(path);
}

#else
#ifdef __GNUC__
#warning NO KIPI
#endif
#endif /* HAVE_KIPI */
