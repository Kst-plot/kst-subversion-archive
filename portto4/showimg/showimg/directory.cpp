/***************************************************************************
                          directory.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult, 2003 OGINO Tomonori
                           (C) 2004-2005 by Jaroslaw Staniek
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
                           js@iidea.pl
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

#include "directory.h"

//-----------------------------------------------------------------------------
#define DIRECTORY_DEBUG        0
#define DIRECTORY_DEBUG_LOAD   0
#define DIRECTORY_DEBUG_OPEN   0
#define DIRECTORY_DEBUG_DELETE 0
//-----------------------------------------------------------------------------

// Local
#include "album.h"
#include "compressedfileitem.h"
#include "compressedimagefileiconitem.h"
#include "directoryview.h"
#include "dirfileiconitem.h"
#include "extract.h"
#include "fileiconitem.h"
#include "imagefileiconitem.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"
#include "tools.h"

// System
#include <unistd.h>

// Qt
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <qpixmapcache.h>
#include <qstring.h>

// KDE
#include <kapplication.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kstandarddirs.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

#ifdef Q_WS_WIN
# include "win_utils.h"
#endif

Directory::Directory(const KURL& a_url /*= KURL()*/)
	: ListItem(getMainWindow()->getDirectoryView(), a_url)
	{
#ifdef Q_WS_WIN
	//for win32, we're setting special icon
	thisIsADriveNode = QDir( QDir::convertSeparators(a_path) ).isRoot();
	if (thisIsADriveNode) 
	{
		hasSpecialIcon = true;
		DriveType dt = driveType(name());
		if (dt==RemovableDrive) {
			m_driveNodeVolumeName = i18n("Floppy disk");
			setPixmap(0, BarIcon("3floppy_unmount", getDirectoryView()->getIconSize() ));
		}
		else {
			QString vn( volumeName(name()) );
			if (!vn.isEmpty())
				m_driveNodeVolumeName = QString( vn );
			if (dt==CDROMDrive)
				setPixmap(0, BarIcon("cdrom_unmount", getDirectoryView()->getIconSize() ));
			else if (dt==RemoteDrive)
				setPixmap(0, BarIcon("nfs_unmount", getDirectoryView()->getIconSize() ));
			else
				setPixmap(0, BarIcon("hdd_unmount", getDirectoryView()->getIconSize() ));
		}
		m_displayedName = m_driveNodeVolumeName.isEmpty()
		? (m_driveNodeVolumeName + QString(" (%1)").arg(name().left(2).upper()))
		: name().left(2);
	}
#endif
	KURL l_url;
	if(a_url.isEmpty())
	{
 		l_url.setProtocol( "file" );
		l_url.setPath( "/" );
	}
	else
	{
		l_url=a_url;
	}
	setURL(l_url);
	init();
	m_displayedName = getURL().prettyURL();
}

Directory::Directory(Directory* a_parent, const QString& a_filename)
	: ListItem(a_parent, a_filename)
{
#if DIRECTORY_DEBUG > 0
	MYDEBUG
		<< "parent()->getURL()="<<parent()->getURL().url( ) << " "
		<< "a_filename="<<a_filename << " "
		<< "getURL()="<<getURL().url( ) << " "
	<<endl;
#endif
	init();
	m_displayedName = name();	
	
#if DIRECTORY_DEBUG > 0
	MYDEBUG
		<< "getURL()="<<getURL().url( ) << " "
	<<endl;
#endif


#ifdef Q_WS_WIN
	if (getFullName() == getMainWindow()->m_myPicturesDirPath)
	{
		hasSpecialIcon = true;
		setPixmap(0, BarIcon("images", getDirectoryView()->getIconSize() ));
	}
#endif
}


Directory::~Directory()
{
#if DIRECTORY_DEBUG > 2
	MYDEBUG<<fullName()<<endl;
#endif
}

DirectoryView*
Directory::getDirectoryView()
{
	return  getMainWindow()->getDirectoryView();
}


void
Directory::init()
{
	m_readable = true;
	m_loaded=false;

	setReadOnly(false);
	setDropEnabled(true);
	if (!hasSpecialIcon)
		setPixmap(0, BarIcon(KMimeType::iconForURL(getURL()), getDirectoryView()->getIconSize() ));
}

bool Directory::hasSubDirectory (const QString& a_dir)
{
	ListItem* l_list_item = firstChild();
	bool l_found=false;
	while(l_list_item && ! l_found)
	{
		l_found = (a_dir == l_list_item->name());
		l_list_item = l_list_item->nextSibling ();
	}
	return l_found;
}



ListItem *
Directory::find (const KURL& a_url)
{
#if DIRECTORY_DEBUG_OPEN > 0
	MYDEBUG
		<< "getURL()="<<getURL().path( ) << " "
		<< "a_url="<<a_url.path() << " "
	<<endl;
#endif
	if(a_url == getURL())
		return this;
	else
		return find(a_url.path());
}

ListItem*
Directory::find(const QString& a_end_dir)
{
#if DIRECTORY_DEBUG_OPEN > 2
	MYDEBUG
		<< "getURL()="<<getURL().url( ) << " "
		<< "a_end_dir="<<a_end_dir << " "
	<<endl;
#endif
	if(!isOpen())
	{
#if DIRECTORY_DEBUG_OPEN > 3
	MYDEBUG
		<< "I'll open dir "
	<<endl;
#endif
		setOpen(true);
		return NULL;		
	}

	QString l_dir(a_end_dir);
	while(l_dir.startsWith("/"))
		l_dir = l_dir.right(l_dir.length()-1);
	QStringList l_list = QStringList::split( "/", l_dir);
	QString l_dirName = l_list[0];
#if DIRECTORY_DEBUG_OPEN > 3
	MYDEBUG
		<< "l_list="<<l_list << " "
		<< "l_dirName="<<l_dirName << " "
		<< "isOpen()="<<(isOpen()?"true":"false") << " "
	<<endl;
#endif
	bool l_is_found=false;
	ListItem* l_list_item = firstChild();
	while(l_list_item && !l_is_found)
	{
		if(l_dirName == l_list_item->name())
		{
#if DIRECTORY_DEBUG_OPEN > 3
		MYDEBUG
			<< "found name="<<l_dirName << " "
			<<endl;
#endif
		
			l_is_found = true;
		}
		else
		{
			l_list_item = l_list_item->nextSibling ();
		}
	}
	if(l_list_item && l_list.count() != 1)
	{
#if DIRECTORY_DEBUG_OPEN > 3
		MYDEBUG
			<< "i'll find for subdirs "
			<<endl;
#endif
			l_list_item = l_list_item->find(l_dir.right(l_dir.length()-l_dir.find("/")-1));
	}
	return l_list_item;
}

void
Directory::recursivelyOpen()
{
	setOpen(true);
	ListItem* l_p_item=firstChild();
	while(l_p_item != 0)
	{
		Directory * l_p_dir = dynamic_cast<Directory*>(l_p_item);
		if(l_p_dir)
			l_p_dir->recursivelyOpen();
		l_p_item = l_p_item->nextSibling ();
	}
}

void
Directory::recursivelyDelete()
{
#if DIRECTORY_DEBUG_DELETE > 0
	MYDEBUG<<fullName()<<endl;
#endif

	if(isSelected())
		unLoad();

	ListItem
		* l_p_item   = firstChild(),
		* l_p_tmpdir = NULL;

	while(l_p_item != 0)
	{
		Directory * l_p_dir = dynamic_cast<Directory*>(l_p_item);
		if(l_p_dir)
			l_p_dir->recursivelyDelete();
		l_p_tmpdir = l_p_item->nextSibling ();
		delete(l_p_item);
		l_p_item = l_p_tmpdir;
	}
}

bool Directory::insertListItem(const KURL& a_url)
{
#if DIRECTORY_DEBUG_OPEN > 1
	MYDEBUG << "a_url="<<a_url.url() << endl;
#endif
	KMimeType::Ptr l_mime =
		KMimeType::findByURL( a_url, 0 /*, a_url.isLocalFile()*/);

	if (
		a_url.fileName() == QString::fromLatin1(".") ||
		a_url.fileName() == QString::fromLatin1(".." )
	)
		; // nothing
	else
	{
			if (
				getDirectoryView()->getShowCompressedFiles() &&
				a_url.isLocalFile() &&
				QFileInfo(a_url.path()).exists() &&
				Extract::canExtract(a_url.path())
			)
			{
#if DIRECTORY_DEBUG_OPEN > 2
 				MYDEBUG<<"Creating CompressedFileItem"<<endl;
#endif
				(void)new CompressedFileItem(this, a_url.fileName(), getURL().path());
			}
			else
			if(l_mime->is("application/sia"))
			{
#if DIRECTORY_DEBUG_OPEN > 2
 				MYDEBUG<<"Creating Album"<<endl;
#endif
				(void)new Album(this, a_url.fileName());
			}
			else
				(void)new Directory( this, a_url.fileName());
	}
	return true;
}

bool Directory::insertIconItem(const KURL& a_url)
{
	KMimeType::Ptr l_mime =
		KMimeType::findByURL( a_url, 0, a_url.isLocalFile());

	FileIconItem *l_p_item = NULL;

	if(!l_mime->is("inode/directory"))
	{
#if DIRECTORY_DEBUG > 0
	MYDEBUG
		<< "getURL()="<<getURL().url( ) << " "
		<< "a_url="<<a_url.url() << " "
		<< "a_url.fileName()="<<a_url.fileName() << " "
		<< "fullName(1)="<<fullName(1) << " "
	<<endl;
#endif
		l_p_item = new ImageFileIconItem(a_url.fileName(), getURL());
	}
	else
	if(getDirectoryView()->showDir())
	{
		bool dotDotAllowed = !QDir( name() ).isRoot() && !hasSpecialIcon;
		if(
			a_url.fileName()!="." &&
			!(
				a_url.fileName() == QString::fromLatin1("..") &&
				!dotDotAllowed
			)
		)
		{
			l_p_item = new DirFileIconItem(a_url.fileName(), fullName(1));
		}
	}

	if(l_p_item)
	{
		getFileIconItemList().append(l_p_item);
		setSize(getFileIconItemList().count());
	}
	l_p_item=NULL;

	getDirectoryView()->setHasSeenFile();
	return true;
}


void
Directory::setOpen( bool a_is_open )
{
#if DIRECTORY_DEBUG_OPEN > 0
 	MYDEBUG
		<<"getURL()="<<getURL().url()<< " "
		<<"a_is_open="<<a_is_open<< " "
		<<endl;
#endif
	if(a_is_open)
	{
		getDirectoryView()->slotShowItem(this);
		if (
			!isOpen()     &&
			!m_loaded
		)
		{
#if DIRECTORY_DEBUG_OPEN > 0
 		MYDEBUG
			<<"I'll start watch dir "
			<<endl;
#endif
			KApplication::setOverrideCursor( waitCursor );
			QListViewItem::setOpen( true );
			getDirectoryView()->startWatchURL(getURL());
			m_loaded=true;
		}
		else
		if(m_loaded)
		{
#if DIRECTORY_DEBUG_OPEN > 0
 		MYDEBUG
			<<"I'll open dir "
			<<endl;
#endif
			QListViewItem::setOpen( true );
			getDirectoryView()->openingFinished(this);
		}
	}
	else // if(a_is_open)
	{
		if(isOpen())
		{
			QListViewItem::setOpen( false );
		}
	}

	
	if (!hasSpecialIcon) 
	{
		if ( a_is_open )
		{
			if(KMimeType::iconForURL(getURL())== QString::fromLatin1("folder"))
			{
				setPixmap(0, BarIcon("folder_open", getDirectoryView()->getIconSize() ));
			}
		}
		else
		{
			setPixmap(0, BarIcon(KMimeType::iconForURL(getURL()), getDirectoryView()->getIconSize() ));
		}
	}

	repaint();
	KApplication::restoreOverrideCursor();     
}


bool
Directory::refresh(bool )
{
	getDirectoryView()->refresh(getURL());
	return false;
}

void
Directory::setup()
{
	setExpandable( true );
	ListItem::setup();
}


QString Directory::displayedName () const
{
	return m_displayedName;
}


void
Directory::removeIconItem(FileIconItem *a_p_imf)
{
	if(a_p_imf)
	{
		if(getFileIconItemList().find(a_p_imf) != -1 )
		{
			getFileIconItemList().removeRef(a_p_imf);
			delete(a_p_imf);
			getMainWindow()->slotRemoveImage(1);
		}
#if DIRECTORY_DEBUG > 0
		else
			MYWARNING<<"a_p_imf not managed by this Directory: "<<a_p_imf->getURL().url()<<endl;
#endif
	}
#if DIRECTORY_DEBUG > 0
	else
		MYWARNING<<" FileIconItem *a_p_imf == NULL"<<endl;
#endif
}

void
Directory::unLoad()
{
#if DIRECTORY_DEBUG_LOAD > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif

	if(getMainWindow()->preview())
		getMainWindow()->getImageListView()->stopLoading();
	getMainWindow()->slotRemoveImage(getSize());
//	getMainWindow()->getImageListView()->setUpdatesEnabled( false );

	getDirectoryView()->stopWatchSelectedItem(getURL());

	for (
		FileIconItem * l_p_item = getFileIconItemList().first();
		l_p_item;
		l_p_item = getFileIconItemList().next()
	)
	{
#if DIRECTORY_DEBUG_LOAD > 1
		MYDEBUG<<"I willl delete..."<<endl;
#endif
		delete(l_p_item);
#if DIRECTORY_DEBUG_LOAD > 1
		MYDEBUG<<"done."<<endl;
#endif
	}
	getFileIconItemList().clear();

	getMainWindow()->getImageViewer()->updateStatus();
//	getMainWindow()->getImageListView()->setUpdatesEnabled( true );
 	//getMainWindow()->getImageListView()->repaintContents  ( false );

#if DIRECTORY_DEBUG_LOAD > 0
	MYDEBUG<<"END"<<endl;
#endif
}


void
Directory::load(bool /*loadThumbnails*/)
{
#if DIRECTORY_DEBUG_LOAD > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif

	if (!checkAccess())
		return;
	ListItem::load(true);
	getDirectoryView()->startWatchSelectedItem(getURL());

#if DIRECTORY_DEBUG_LOAD > 0
	MYDEBUG<<"END"<<endl;
#endif

}


void
Directory::createDir(const QString& a_dirName)
{
#ifndef Q_WS_WIN
	KURL url = KURL(getProtocol()+":"+fullName()+a_dirName);
	KonqOperations::mkdir(getMainWindow(), url);
	setExpandable(true);
#endif
}

void
Directory::createAlbum(const QString& a_albumName)
{
	KURL urld = KURL(getProtocol()+':'+fullName()+a_albumName);
	KURL urlo = KURL(getProtocol()+':'+KStandardDirs().findResource("templates",".source/TextFile.txt"));
	KIO::copy(urlo, urld, false);
	setExpandable(true);
	getMainWindow()->addToBookmark(i18n("Albums"),fullName()+a_albumName);
	if(isOpen())
		(void)new Album(this, a_albumName);
}

void
Directory::rename()
{
	if(!getNewDirName().isEmpty())
	{
		//FIXME
		getURL().setFileName(getNewDirName());
		setFullName(path()+'/'+getNewDirName()+'/');
		repaint();
	}
}

bool
Directory::rename(const QString& a_newDirName, QString& a_msg)
{

	QString newN=path()+'/'+a_newDirName;
	if(QFileInfo(newN).exists())
	{
		a_msg=i18n("The directory '<b>%1</b>' already exists").arg(newN);
		return false;
	}

	setNewDirName(a_newDirName);

	QString oldN=fullName();

	KURL orgName(getProtocol()+':'+oldN);
	KURL newName(getProtocol()+':'+newN);

	KIO::FileCopyJob *renameJob=KIO::file_move(orgName, newName, true );
	QObject::connect(renameJob, SIGNAL(result( KIO::Job * )),
					 getDirectoryView(), SLOT(renameDone( KIO::Job * )));

	//FIXME getFile().setName(getNewDirName());
	getURL().setFileName(getNewDirName());
	repaint();
	updateChildren();

	return true;
}


void
Directory::updateChildren()
{
	ListItem::updateChildren();
	if(isSelected())
	{
		FileIconItem *l_p_item;
		for ( l_p_item = getFileIconItemList().first(); l_p_item; l_p_item = getFileIconItemList().next() )
		{
			l_p_item->setPath(fullName());
		}
	}
}

bool Directory::checkAccess()
{
	return true;
}

