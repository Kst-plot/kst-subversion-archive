/*****************************************************************************
                          cdarchive.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2006 by Richard Groult
    email                : rgroult@jalix.org
 *****************************************************************************/

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

#include "cdarchive.h"

// Local
#include "album.h"
#include "cdarchiveview.h"
#include "directory.h"
#include "directoryview.h"
#include "imagefileiconitem.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "tools.h"

// KDE
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kurl.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

// Qt
#include <qdir.h>

class CDArchiveItem;

class CDArchiveImageFileIconItem : public ImageFileIconItem
{
public:
	CDArchiveImageFileIconItem(
	 		const QString& a_fullname);
};


/////////////////
class CDArchiveItem : public ListItem
{
public:
	CDArchiveItem(
		CDArchive* a_p_parent,
		const QString& a_filename,
		const KArchiveDirectory *a_p_dir);

	CDArchiveItem(
		CDArchiveItem *a_p_parent,
		const QString& a_filename,
		const KArchiveDirectory *a_p_dir);

	virtual ~CDArchiveItem();

private:
	void init();
	QString getRelativePath();
	void load(bool);
	void unLoad();

	virtual ListItem* find (const KURL&);
	virtual ListItem* find (const QString& a_end_dir);

	const KArchiveDirectory* getKArchiveDirectory();

	const KArchiveDirectory* m_p_dir;

	bool loaded;
	QString relativePath;

	QPtrList < CDArchiveImageFileIconItem > list;
};


CDArchiveItem::CDArchiveItem(
			CDArchive* a_p_parent,
			const QString& a_filename,
			const KArchiveDirectory *a_p_dir)
	: ListItem(a_p_parent, a_filename)
{
	m_p_dir = a_p_dir;
	relativePath = a_p_parent->getRelativePath()+'/'+a_filename;
	init();
}



CDArchiveItem::CDArchiveItem(
			CDArchiveItem *a_p_parent,
			const QString& a_filename,
			const KArchiveDirectory *a_p_dir)
	: ListItem(a_p_parent, a_filename)
{
	m_p_dir = a_p_dir;
	relativePath = a_p_parent->getRelativePath()+'/'+a_filename;
	init();
}

CDArchiveItem::~CDArchiveItem()
{
}

void
CDArchiveItem::init()
{
	setPixmap(0, BarIcon("folder", getListItemView()->getIconSize() ));
	setDropEnabled(false);
	setExtension(i18n("CD Archive folder"));
	loaded=false;
	setFullName(locateLocal("tmp", "showimg-arc/"+getRelativePath())+'/');
	setProtocol("cdarchive");
}


void
CDArchiveItem::load(bool)
{
	ListItem::load(true);

	QStringList const & entries(m_p_dir->entries());
	QStringList::const_iterator it = entries.begin();
	CDArchiveImageFileIconItem *item;

	getMainWindow()->getCDArchiveView()->loadingIsStarted(this, entries.count());
	setSize(entries.count());

	if(!loaded)
	{
		QString dest = locateLocal("tmp", "showimg-arc/"+getRelativePath());
		m_p_dir->copyTo( dest, false );
	}

	for(; it != entries.end(); ++it)
	{
		const KArchiveEntry * child = m_p_dir->entry(*it);
		if( !loaded && child->isDirectory() )
		{
			const KArchiveDirectory * childdir;
			childdir = dynamic_cast<const KArchiveDirectory *>(child);
			(void)new CDArchiveItem(this, *it, childdir);
		}
		else
		if(Tools::isImage(QFileInfo(*it)))
		{
			item = new CDArchiveImageFileIconItem (
						locateLocal("tmp", "showimg-arc/")+getRelativePath()+'/'+*it);
			list.append(item);
		}
		else
		if( !loaded && QFileInfo(*it).extension().lower() == QString::fromLatin1("sia"))
		{
			(void)new Album (this, *it);
		}
	}
	getMainWindow()->getCDArchiveView()->loadingIsFinished(this, list.count());
	setSize(list.count());

	loaded=true;
}

void
CDArchiveItem::unLoad ()
{
	if(getMainWindow()->preview()) getMainWindow()->getImageListView()->stopLoading();
	getMainWindow()->slotRemoveImage(list.count());
	getMainWindow()->getImageListView()->setUpdatesEnabled( false );

	for (CDArchiveImageFileIconItem *item = list.first(); item; item = list.next() )
		getMainWindow()->getImageListView()->takeItem(item);
	list.clear();

	getMainWindow()->getImageViewer()->updateStatus();
	getMainWindow()->getImageListView()->setUpdatesEnabled( true );
 	getMainWindow()->getImageListView()->repaintContents  ( false );
	if(getMainWindow()->preview()) getMainWindow()->getImageListView()->slotLoadFirst();
}

ListItem*
CDArchiveItem::find (const KURL& a_url)
{
	return find(a_url.path());
}

ListItem* CDArchiveItem::find (const QString& a_end_dir)
{
	QStringList list = QStringList::split( "/", a_end_dir);
	QString dirName = list[0];
	list.pop_front();
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		if(rootItems->name() == dirName)
			if(list.isEmpty())
				return rootItems;
			else
				return rootItems->find(list.join("/"));
		rootItems = rootItems->nextSibling();
	}
	return NULL;
}


const KArchiveDirectory*
CDArchiveItem::getKArchiveDirectory()
{
	return m_p_dir;
}

QString
CDArchiveItem::getRelativePath()
{
	return 	relativePath;
}



/////////////////////////////////////////////////////

CDArchive::CDArchive( )
	: ListItem(getMainWindow()->getCDArchiveView() )
{
	setFullName(CDArchive_ROOTPATH);
	//FIXME getFile().setName (i18n("CD Archive Root"));
	m_isRoot=true;
	init();
	setReadOnly(true);
	load(true);
}

CDArchive::CDArchive(
			CDArchive* parent,
			const QString& filename)
	: ListItem(parent, filename)
{
	setFullName(this->parent()->fullName()+name());
	m_relativePath = name();
	m_isRoot=false;
	init();
	setReadOnly(false);
}


CDArchive::~CDArchive()
{
}


void
CDArchive::init()
{
	setPixmap(0, BarIcon("cdimage", getListItemView()->getIconSize() ));
	setDropEnabled(false);
	setExtension(i18n("CD Archive"));
	setProtocol("cdarchive");
	m_loaded=false;
}

bool
CDArchive::rename(const QString& newDirName, QString& msg)
{
	if(newDirName.isEmpty())
	{
		msg=i18n("The given name is empty");
		return false;
	}
	QString oldN=fullName();
	QString newN=this->parent()->fullName()+'/'+newDirName;
	if(QFileInfo(newN).exists())
	{
		msg=i18n("The archive '<b>%1</b>' already exists").arg(newN);
		return false;
	}

	KURL orgName = KURL("file:"+oldN);
	KURL newName = KURL("file:"+newN);

	updateChildren();

	KIO::SimpleJob *renameJob=KIO::rename(orgName, newName, true );
	QObject::connect(renameJob, SIGNAL(result( KIO::Job * )),
					 getListItemView(), SLOT(renameDone( KIO::Job * )));
	return true;
}

ListItem*
CDArchive::find (const KURL& a_url)
{
	QString l_path(a_url.path());
	if(m_isRoot)
	{
		if(! (   l_path.startsWith(CDArchive_ROOTPATH)
					   || l_path.startsWith(CDArchive_TEMP_ROOTPATH())))
			return NULL;

		if(   QFileInfo(l_path).isDir()
					&& QDir(l_path) == QDir(CDArchive_ROOTPATH))
				return this;

		if(l_path.startsWith(CDArchive_ROOTPATH))
		{
			l_path = QFileInfo(l_path).fileName();
		}
		else /* path.startsWith(CDArchive_TEMP_ROOTPATH())) */
		{
			l_path = l_path.right(l_path.length()-CDArchive_TEMP_ROOTPATH().length());
		}


	}
	return find(l_path);
}

ListItem*
CDArchive::find (const QString& a_end_dir)
{
	QStringList list = QStringList::split( "/", a_end_dir);
	QString dirName = list[0];
	list.pop_front();
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		if(rootItems->name() == dirName)
			if(list.isEmpty())
				return rootItems;
			else
				return rootItems->find(list.join("/"));
		rootItems = rootItems->nextSibling();
	}
	return NULL;
}

void
CDArchive::load(bool)
{
	if(m_loaded) return;
	KApplication::setOverrideCursor( waitCursor ); // this might take time
	if(m_isRoot)
	{
			//emit sigSetMessage(i18n("Loading CD archives..."));

			QDir thisDir( CDArchive_ROOTPATH );
			thisDir.setNameFilter(QString("*.")+CDArchive_EXTENSION);
			const QFileInfoList * files = thisDir.entryInfoList();
			int nbr=0;
			if ( files )
			{
				QFileInfoListIterator it( *files );
				QFileInfo * f;
				while( (f=it.current()) != 0 )
				{
					++it;
					(void)new CDArchive(this, f->fileName());
					nbr++;
				}
			}
			setSize(nbr);
	}
	else
	{
		m_p_arc = new KTar( fullName() );
		if(!getKArchive()) {KApplication::restoreOverrideCursor(); return;}
		if (!getKArchive()->open( IO_ReadOnly )) {KApplication::restoreOverrideCursor(); return;}
		const KArchiveDirectory *dir = getKArchiveDirectory();
		QStringList const & entries(dir->entries());
		QStringList::const_iterator it = entries.begin();
		for(; it != entries.end(); ++it)
		{
			//emit sigSetMessage(i18n("Loading CD archive %1...").arg(text(0)));
			const KArchiveEntry * child = dir->entry(*it);
			if( child->isDirectory() )
			{
				const KArchiveDirectory * childdir;
				childdir = dynamic_cast<const KArchiveDirectory *>(child);
				(void)new CDArchiveItem(this, *it, childdir);
			}
		}
	}
	KApplication::restoreOverrideCursor();	// restore original cursor
	m_loaded=true;
	//emit sigSetMessage(i18n("Ready"));
}

void
CDArchive::unLoad()
{
}

bool
CDArchive::refresh(bool)
{
	bool modified = false;

	//remove CDArchive
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		ListItem *ssrep = rootItems;
		rootItems = rootItems->nextSibling();

		QFileInfo *f = new QFileInfo(ssrep->fullName());
		if(!f->exists())
		{
			delete(ssrep);
			modified = true;
		}
	}
	//add CDArchive
	QDir thisDir( CDArchive_ROOTPATH );
	thisDir.setNameFilter(QString("*.")+CDArchive_EXTENSION);
	const QFileInfoList * files = thisDir.entryInfoList();
	if ( files )
	{
		QFileInfoListIterator it( *files );
		QFileInfo * f;
		while( (f=it.current()) != 0 )
		{
			++it;
			if(!find(f-> absFilePath()))
			{
				(void)new CDArchive(this, f->fileName());
				modified = true;
			}
		}
	}
	return modified;
}

const KArchiveDirectory*
CDArchive::getKArchiveDirectory() const
{
	if(!m_p_arc)
		return  NULL;
	else
		return m_p_arc->directory();
}

KArchive*
CDArchive::getKArchive() const
{
	return m_p_arc;
}

QString
CDArchive::getRelativePath() const
{
	return m_relativePath;
}

QString
CDArchive::CDArchive_TEMP_ROOTPATH()
{
	return locateLocal("tmp", "showimg-arc/");
}

////////////////////////////
/////////////////////////
CDArchiveImageFileIconItem::CDArchiveImageFileIconItem(
// 			CDArchiveItem *a_p_parent,
	 		const QString& a_fullname)
	:ImageFileIconItem(
		QFileInfo(a_fullname).fileName(),
		QFileInfo(a_fullname).dirPath(true)+'/',
		"",
		false)
{
	setIsMovable(false);
	setKey(getMainWindow()->getImageListView()->getCurrentKey());
	setProtocol("cdarchiveimage");
}

