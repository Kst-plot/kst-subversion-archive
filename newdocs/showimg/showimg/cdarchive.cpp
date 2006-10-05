/***************************************************************************
                          cdarchive.cpp  -  description
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

#include "cdarchive.h"

// Local
#include "imagefileiconitem.h"
#include "directoryview.h"
#include "mainwindow.h"
#include "imageviewer.h"
#include "album.h"
#include "directory.h"
#include "cdarchiveview.h"

// KDE
#include <kmimetype.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ktar.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kurl.h>
#include <kapplication.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

// Qt
#include <qdir.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class CDArchiveItem;

class CDArchiveImageFileIconItem : public ImageFileIconItem
{
public:
	CDArchiveImageFileIconItem(
			CDArchiveItem *parent,
	 		const QString& fullname,
			MainWindow *mw);

protected:
	CDArchiveItem *parentDir;
};


/////////////////
class CDArchiveItem : public ListItem
{
public:
	CDArchiveItem(
		CDArchive* parent,
		const QString& filename,
		const KArchiveDirectory *dir,
		MainWindow *mw );

	CDArchiveItem(
		CDArchiveItem *parent,
		const QString& filename,
		const KArchiveDirectory *dir,
		MainWindow *mw );

	virtual ~CDArchiveItem();

private:
	void init();
	QString getRelativePath();
	void load(bool);
	void unLoad();
	virtual  ListItem* find (const QString&);
	const KArchiveDirectory* getKArchiveDirectory();

	const KArchiveDirectory* dir;

	bool loaded;
	QString relativePath;

	QPtrList < CDArchiveImageFileIconItem > list;
};


CDArchiveItem::CDArchiveItem(
			CDArchive* parent,
			const QString& filename,
			const KArchiveDirectory *dir,
			MainWindow *mw)
	: ListItem(parent, filename, mw)
{
	this->dir = dir;
	relativePath = parent->getRelativePath()+"/"+filename;
	init();
}



CDArchiveItem::CDArchiveItem(
			CDArchiveItem *parent,
			const QString& filename,
			const KArchiveDirectory *dir,
			MainWindow *mw )
	: ListItem(parent, filename, mw)
{
	this->dir = dir;
	relativePath = parent->getRelativePath()+"/"+filename;
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
	extension = i18n("CD Archive folder");
	setType("CD Archive folder");
	loaded=false;
	full = locateLocal("tmp", "showimg-arc/"+getRelativePath())+"/";
	setProtocol("cdarchive");
}


void
CDArchiveItem::load(bool)
{
	ListItem::load(true);

	QStringList const & entries(dir->entries());
	QStringList::const_iterator it = entries.begin();
	CDArchiveImageFileIconItem *item;

	mw->getCDArchiveView()->loadingIsStarted(this, entries.count());
	setSize(entries.count());

	if(!loaded)
	{
		QString dest = locateLocal("tmp", "showimg-arc/"+getRelativePath());
		dir->copyTo( dest, false );
	}

	for(; it != entries.end(); ++it)
	{
		const KArchiveEntry * child = dir->entry(*it);
		if( !loaded && child->isDirectory() )
		{
			const KArchiveDirectory * childdir;
			childdir = dynamic_cast<const KArchiveDirectory *>(child);
			(void)new CDArchiveItem(this, *it, childdir, mw );
		}
		else
		if(getListItemView()->isImage(new QFileInfo(*it)))
		{
			item = new CDArchiveImageFileIconItem (
						this,
						locateLocal("tmp", "showimg-arc/")+getRelativePath()+"/"+*it,
						mw);
			list.append(item);
		}
		else
		if( !loaded && QFileInfo(*it).extension().lower() == QString::fromLatin1("sia"))
		{
			(void)new Album (this, *it, mw);
		}
	}
	mw->getCDArchiveView()->loadingIsFinished(this, list.count());
	setSize(list.count());

	loaded=true;
}

void
CDArchiveItem::unLoad ()
{
	if(mw->preview()) mw->getImageListView()->stopLoading();
	mw->slotRemoveImage(list.count());
	mw->getImageListView()->setUpdatesEnabled( false );

	for (CDArchiveImageFileIconItem *item = list.first(); item; item = list.next() )
		mw->getImageListView()->takeItem(item);
	list.clear();

	mw->getImageViewer()->updateStatus();
	mw->getImageListView()->setUpdatesEnabled( true );
 	mw->getImageListView()->repaintContents  ( false );
	if(mw->preview()) mw->getImageListView()->slotLoadFirst();
}

ListItem*
CDArchiveItem::find (const QString& path)
{
	QStringList list = QStringList::split( "/", path);
	QString dirName = list[0];
	list.pop_front();
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		if(rootItems->text(0) == dirName)
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
	return dir;
}

QString
CDArchiveItem::getRelativePath()
{
	return 	relativePath;
}



/////////////////////////////////////////////////////

CDArchive::CDArchive( MainWindow *mw )
	: ListItem( mw, mw->getCDArchiveView() )
{
	full = CDArchive_ROOTPATH;
	f.setName (i18n("CD Archive Root"));
	isRoot=true;
	init();
	setReadOnly(true);
	load(true);
}

CDArchive::CDArchive(
			CDArchive* parent,
			const QString& filename,
			MainWindow *mw )
	: ListItem(parent, filename, mw)
{
	full=this->parent()->fullName()+f.name();
	relativePath = f.name();
	isRoot=false;
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
	extension = i18n("CD Archive");
	setType("CD Archive");
	loaded=false;
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
	QString newN=this->parent()->fullName()+"/"+newDirName;
	if(QFileInfo(newN).exists())
	{
		msg=i18n("The archive '<b>%1</b>' already exists").arg(newN);
		return false;
	}

	KURL orgName = KURL("file:"+oldN);
	KURL newName = KURL("file:"+newN);

	f.setName(newDirName);
	updateChildren();

	KIO::SimpleJob *renameJob=KIO::rename(orgName, newName, true );
	QObject::connect(renameJob, SIGNAL(result( KIO::Job *)),
					 getListItemView(), SLOT(renameDone( KIO::Job *)));
	return true;
}

ListItem*
CDArchive::find (const QString& path)
{
	QString m_path=path;
	if(isRoot)
	{
		if(! (   m_path.startsWith(CDArchive_ROOTPATH)
					   || m_path.startsWith(CDArchive_TEMP_ROOTPATH())))
			return NULL;

		if(   QFileInfo(m_path).isDir()
					&& QDir(m_path) == QDir(CDArchive_ROOTPATH))
				return this;

		if(m_path.startsWith(CDArchive_ROOTPATH))
		{
			m_path = QFileInfo(m_path).fileName();
		}
		else /* path.startsWith(CDArchive_TEMP_ROOTPATH())) */
		{
			m_path = m_path.right(m_path.length()-CDArchive_TEMP_ROOTPATH().length());
		}


	}
	QStringList list = QStringList::split( "/", m_path);
	QString dirName = list[0];
	list.pop_front();
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		if(rootItems->text(0) == dirName)
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
	if(loaded) return;
	KApplication::setOverrideCursor( waitCursor ); // this might take time
	if(isRoot)
	{
			mw->setMessage(i18n("Loading CD archives..."));

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
					(void)new CDArchive(this, f->fileName(), mw);
					nbr++;
				}
			}
			setSize(nbr);
	}
	else
	{
		arc = new KTar( fullName() );
		if(!arc) {KApplication::restoreOverrideCursor(); return;}
		if (!arc->open( IO_ReadOnly )) {KApplication::restoreOverrideCursor(); return;}
		const KArchiveDirectory *dir = arc->directory();
		QStringList const & entries(dir->entries());
		QStringList::const_iterator it = entries.begin();
		for(; it != entries.end(); ++it)
		{
			mw->setMessage(i18n("Loading CD archive %1...").arg(text(0)));
			const KArchiveEntry * child = dir->entry(*it);
			if( child->isDirectory() )
			{
				const KArchiveDirectory * childdir;
				childdir = dynamic_cast<const KArchiveDirectory *>(child);
				(void)new CDArchiveItem(this, *it, childdir, mw);
			}
		}
	}
	KApplication::restoreOverrideCursor();	// restore original cursor
	loaded=true;
	mw->setMessage(i18n("Ready"));
}

void
CDArchive::unLoad()
{
}

bool
CDArchive::refresh(bool)
{
	MYDEBUG<<endl;
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
				(void)new CDArchive(this, f->fileName(), mw);
				modified = true;
			}
		}
	}
	return modified;
}

const KArchiveDirectory*
CDArchive::getKArchiveDirectory() const
{
	return arc->directory();
}

KArchive*
CDArchive::getKArchive() const
{
	return arc;
}

QString
CDArchive::getRelativePath()
{
	return f.name();
}

QString
CDArchive::CDArchive_TEMP_ROOTPATH()
{
	return locateLocal("tmp", "showimg-arc/");
}

////////////////////////////
/////////////////////////
CDArchiveImageFileIconItem::CDArchiveImageFileIconItem(
			CDArchiveItem *parent,
	 		const QString& fullname,
			MainWindow *mw)
	:ImageFileIconItem(
		parent,
		QFileInfo(fullname).fileName(),
		QFileInfo(fullname).dirPath(true)+'/',
		mw,
		"",
		false)
{
	setType("CDArchiveImageFileIconItem");
	setIsMovable(false);
	parentDir = parent;
	setKey(mw->getImageListView()->getCurrentKey());
	setProtocol("cdarchiveimage");
}

