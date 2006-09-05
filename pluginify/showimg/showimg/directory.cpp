/***************************************************************************
                          directory.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult, 2003 OGINO Tomonori
                           (C) 2004-2005 by Jaroslaw Staniek
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
                           js@iidea.pl
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

#include "directory.h"

// Local
#include "directoryview.h"
#include "imageviewer.h"
#include "imagefileiconitem.h"
#include "dirfileiconitem.h"
#include "compressedimagefileiconitem.h"
#include "compressedfileitem.h"
#include "fileiconitem.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "album.h"
#include "extract.h"

#include <unistd.h>

// Qt
#include <qlistview.h>
#include <qstring.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmapcache.h>

// KDE
#include <kio/job.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

#ifdef Q_WS_WIN
# include "win_utils.h"
#endif

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

Directory::Directory(MainWindow *mw, const QString& path)
	: ListItem(mw, mw->getDirectoryView(), path)
#ifdef Q_WS_WIN
	,driveNodeVolumeName(0)
#endif
	{
#ifdef Q_WS_WIN
	//for win32, we're setting special icon
	thisIsADriveNode = QDir( QDir::convertSeparators(path) ).isRoot();
	if (thisIsADriveNode) {
		hasSpecialIcon = true;
		DriveType dt = driveType(f.name());
		if (dt==RemovableDrive) {
			driveNodeVolumeName = new QString(i18n("Floppy disk"));
			setPixmap(0, BarIcon("3floppy_unmount", getDirectoryView()->getIconSize() ));
		}
		else {
			QString vn( volumeName(f.name()) );
			if (!vn.isEmpty())
				driveNodeVolumeName = new QString( vn );
			if (dt==CDROMDrive)
				setPixmap(0, BarIcon("cdrom_unmount", getDirectoryView()->getIconSize() ));
			else if (dt==RemoteDrive)
				setPixmap(0, BarIcon("nfs_unmount", getDirectoryView()->getIconSize() ));
			else
				setPixmap(0, BarIcon("hdd_unmount", getDirectoryView()->getIconSize() ));
		}
	}
#endif
//	full=( "/" );
	full = f.name();
	init();
}

Directory::Directory(Directory* parent, const QString& filename, MainWindow *mw)
	: ListItem(parent, filename, mw)
#ifdef Q_WS_WIN
	,driveNodeVolumeName(0)
#endif
{
	full=this->parent()->fullName()+f.name()+QDir::separator();
	init();
#ifdef Q_WS_WIN
	if (full == mw->m_myPicturesDirPath) {
		hasSpecialIcon = true;
		setPixmap(0, BarIcon("images", getDirectoryView()->getIconSize() ));
	}
#endif
}


Directory::~Directory()
{
#ifdef Q_WS_WIN
	delete driveNodeVolumeName;
#endif
}

DirectoryView*
Directory::getDirectoryView()
{
	return  mw->getDirectoryView();
}


void
Directory::init()
{
	readable = true;
	if (!hasSpecialIcon)
		setPixmap(0, BarIcon(KMimeType::iconForURL(getURL()), getDirectoryView()->getIconSize() ));
	setDropEnabled(true);
	setType("directory");
	setReadOnly(false);
	loaded=false;
}


ListItem*
Directory::find(const QString& dir_)
{
	QString dir(dir_);
	while(dir.startsWith("/"))
		dir = dir.right(dir.length()-1);
	QStringList list = QStringList::split( "/", dir);
	QString dirName = list[0];
	ListItem* dirt = firstChild();
	while(dirt)
	{
		if(dirName == dirt->text(0)) break;
		dirt = dirt->nextSibling ();
	}
	if(dirt)
		if(list.count()==1)
			return dirt;
		else
			return dirt->find(dir.right(dir.length()-dir.find("/")-1));
	return dirt;
}

void
Directory::recursivelyOpen()
{
	setOpen(true);
	ListItem* dirt=firstChild();
	while(dirt != 0)
	{
		if(dirt->getType() == QString::fromLatin1("directory")) ((Directory*)dirt)->recursivelyOpen();
		dirt = dirt->nextSibling ();
	}
	kapp->processEvents();
}

void
Directory::recursivelyDelete()
{
	if(isSelected())
		unLoad();
	ListItem *dirt=firstChild(), *tmpdir;
	while(dirt != 0)
	{
		if(dirt->getType() == QString::fromLatin1("directory")) ((Directory*)dirt)->recursivelyDelete();
		tmpdir = dirt->nextSibling ();
		delete(dirt);
		dirt = tmpdir;
	}
}

void
Directory::setOpen( bool o )
{
	if ( !isOpen() && o && !childCount() && !loaded)
	{
		if (!checkAccess())
			return;
		KApplication::setOverrideCursor( waitCursor ); // this might take time

		QString s( fullName() );
		QDir thisDir( s );
		if(getDirectoryView()->showHiddenDir())
			thisDir.setFilter(QDir::All|QDir::Hidden);
		else
			thisDir.setFilter(QDir::All);
		const QFileInfoList * files = thisDir.entryInfoList();
		if ( files )
		{
		 	QFileInfoListIterator it( *files );
		 	QFileInfo * f;

		 	while( (f=it.current()) != 0 )
		 	{
				++it;
				  if ( f->fileName() == QString::fromLatin1(".") || f->fileName() ==QString::fromLatin1(".." ))
					  ; // nothing
				  else
				  {
						if ( f->isDir()
#ifndef Q_WS_WIN //no symlinks on win32
						  || (f->isSymLink() && QFileInfo(f->readLink()).isDir())
#endif
						)
							  (void)new Directory( this, f->fileName(), mw);
					  else
					  {
						   if ( getDirectoryView()->getShowCompressedFiles() && Extract::canExtract(f->absFilePath()))
							(void)new CompressedFileItem(this, f->fileName(), fullName(),  mw);
						   else
							   if(QFileInfo(f->fileName()).extension().lower() == QString::fromLatin1("sia"))
							(void)new Album(this, f->fileName(), mw);
					  }
				  }
		 	}
			getDirectoryView()->sort();
		}
		if(!childCount()) setExpandable(false);
		KApplication::restoreOverrideCursor();      // restore original cursor

		getDirectoryView()->startWatchDir(fullName());
		loaded=true;
	}
	else
	if(!o && childCount() )
	{
	}

	if (!hasSpecialIcon) {
		if ( o )
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

	QListViewItem::setOpen( o );
	repaint();
// 	listView()->setUpdatesEnabled( true );
}


bool
Directory::refresh(bool )
{
	mw->getImageListView()->setUpdatesEnabled(false);
	if (!checkAccess())
		return false;

	QString s( fullName() );
	QDir thisDir( s );
	bool isS = isSelected();
	thisDir.setFilter(getDirectoryView()->filter() | QDir::Dirs);

	const QFileInfoList * files = thisDir.entryInfoList();
	if (!files)
		return false;

	//////////////////
	QStringList after;
	QStringList news;
	QStringList deleted;

	QFileInfoListIterator it( *files );
	QFileInfo * f;
	QString ext;

	QString filename;
	FileIconItem *item;
	while( (f=it.current()) != 0 )
	{
		++it;
		if(
			(f->isDir() && f->fileName()!=".")
			||
			(f->isFile () &&
				 (getDirectoryView()->showAllFile() ||
				 getDirectoryView()->isImage(f) ||
				 Extract::canExtract(f->absFilePath())))
		)
		{
			item=mw->getImageListView()->findItem(f->fileName());
			if(!item) news.append(f->fileName());
			after.append(f->fileName());
		}
	}

// 	QPoint tl(mw->getImageListView()->contentsX(), mw->getImageListView()->contentsY());

	bool add=false, del=false;
	for ( item=list.first();  item!= 0; item=list.next() )
	{
		if(!after.contains(item->text()))
		{
			deleted.append(item->text());
			del=true;
		}
	}
	ListItem* dirt=firstChild();
	while(dirt != 0)
	{
		if(!after.contains(dirt->text(0)))
		{
			deleted.append(dirt->text(0));
			del=true;
		}
		dirt = dirt->nextSibling ();
	}

	int nbrAddedFiles=0;
	///
	for ( QStringList::Iterator it = news.begin(); it != news.end(); ++it )
	{
		QFileInfo fi(fullName()+*it);
		if(isS &&
				 (getDirectoryView()->isImage(&fi) ||
				 (getDirectoryView()->showAllFile() &&
		  	(!fi.isDir() || Extract::canExtract(fi.absFilePath()))))
		)
		{
			item = new ImageFileIconItem(this, *it, fullName(), mw);
			list.append(item);
			nbrAddedFiles++;
			setSize(getSize()+1);
			add=true;
		}
		if(fi.fileName() == QString::fromLatin1(".") || find(*it))
			continue;

		if(fi.isDir() && fi.fileName()!="..")
		{
			(void)new Directory(this, *it, mw);
			setExpandable( true );
		}
		if(isS && fi.isDir() && getDirectoryView()->showDir() )
		{
			item = new DirFileIconItem(this, *it, fullName(), mw);
			list.append(item);
			nbrAddedFiles++;
			setSize(getSize()+1);
			add=true;
		}
		if (!fi.isDir() && Extract::canExtract(fi.absFilePath()))
		{
			(void)new CompressedFileItem(this, fi.fileName(), fullName(),  mw);
			setExpandable( true );
			setOpen(true);
		}
	}
	int toDel=0;
	for ( QStringList::Iterator it = deleted.begin(); it != deleted.end(); ++it )
	{
		if(QFileInfo(fullName()+*it).exists())
			continue;
		if(QFileInfo(fullName()+*it).isDir())
			continue;
		getDirectoryView()->removeDir(fullName()+*it);
		if(isS)
		{
			if( (item=mw->getImageListView()->findItem(*it)) != NULL)
			{
				list.remove(item);
				delete(item);
				toDel++;
			}
		}
	}

	mw->getImageListView()->setUpdatesEnabled(true);

	if(news.isEmpty() && deleted.isEmpty()) return false;
	if(isOpen() && !childCount()) setExpandable( false );
	if(isS)
	{
		//FIXME pb with archives
// 		MYDEBUG<<news<<endl;
// 		MYDEBUG<<deleted<<endl;

		setSize(getSize()-toDel);
		mw->slotRemoveImage(toDel);
		mw->slotAddImage(nbrAddedFiles);

		mw->getImageListView()->sort();
		repaint();
// 		MYDEBUG<<fullName()<<endl;
		return true;
	}
	else
	{
		repaint();
// 		MYDEBUG<<fullName()<<endl;
		return false;
	}
}

void
Directory::goTo(const QString&)
{
	kdWarning()<<"Directory::goTo(const QString&) TODO"<<endl;
}

void
Directory::setup()
{
	setExpandable( true );
	ListItem::setup();
}


QString
Directory::path()
{
	QDir dir=QFileInfo(fullName()).dir();
	if (dir.cdUp())
		return dir.absPath();
	else
		return QString();
}

QString
Directory::text( int column ) const
{
	if ( column == DirectoryView::COLUMN_NAME ) {
#ifdef Q_WS_WIN
		//a special way for displaying win32 drive names
		if (thisIsADriveNode) {
			return (driveNodeVolumeName && !driveNodeVolumeName->isEmpty())
				? (*driveNodeVolumeName + QString(" (%1)").arg(f.name().left(2).upper()))
				: f.name().left(2);
		}
#endif
		return f.name();
	}
	else
	if(column == DirectoryView::COLUMN_SIZE )
	{
		if(getSize()>=0)
			return QString::number(getSize());
		else
			return QString();
	}
	else
	if(column == DirectoryView::COLUMN_TYPE)
	{
		if ( readable )
			return i18n("Directory");
		else
			return i18n("Locked");
	}
	return "";
}

void
Directory::loadFirst()
{
}


void
Directory::removeImage(ImageFileIconItem *imf)
{
	if(!imf)
		return;

	if(list.find(imf)!=-1)
	{
		delete(imf);
		mw->slotRemoveImage(1);
	}
}

void
Directory::unLoad()
{
	if(mw->preview()) mw->getImageListView()->stopLoading();
	mw->slotRemoveImage(getSize());
	mw->getImageListView()->setUpdatesEnabled( false );

	for (FileIconItem *item = list.first(); item; item = list.next() )
		delete(item);
	list.clear();

	mw->getImageViewer()->updateStatus();
	mw->getImageListView()->setUpdatesEnabled( true );
 	mw->getImageListView()->repaintContents  ( false );
}


void
Directory::load(bool /*loadThumbnails*/)
{
	if (!checkAccess())
		return;
	ListItem::load(true);


	QString s( fullName() );
	QDir thisDir( s );
	thisDir.setFilter(getDirectoryView()->filter());
	const QFileInfoList * files = thisDir.entryInfoList();
	if ( files )
	{
		getDirectoryView()->loadingIsStarted(this, files->count());
		setSize(files->count()-2);

		int c_size=0;
		QFileInfoListIterator it( *files );
		QFileInfo * finfo;
		QString ext;
		FileIconItem *item=NULL;
		while( (finfo=it.current()) != 0 )
		{
			++it;
			if(
				finfo->isFile () &&
						(getDirectoryView()->showAllFile() ||
						(!getDirectoryView()->showAllFile() && getDirectoryView()->isImage(finfo))||
						(getDirectoryView()->getShowVideo() && getDirectoryView()->isVideo(finfo))))
			{
				item = new ImageFileIconItem(this, finfo->fileName(), fullName(), mw);
				c_size+=(item->isImage()?1:0);
			}
			else
			if(getDirectoryView()->showDir())
			{
				bool dotDotAllowed = !QDir( f.name() ).isRoot() && !hasSpecialIcon;
				if(finfo->isDir() && finfo->fileName()!="." && !(finfo->fileName() == QString::fromLatin1("..") && !dotDotAllowed))
				{
					item = new DirFileIconItem(this, finfo->fileName(), fullName(), mw);
				}
			}

			if(item) list.append(item);
			item=NULL;

			getDirectoryView()->setHasSeenFile();
		}
		setSize(c_size);
		getDirectoryView()->loadingIsFinished(this, files->count());
	}
	else
	{
		getDirectoryView()->loadingIsStarted(this, 0);
		getDirectoryView()->loadingIsFinished(this, 0);
		KApplication::restoreOverrideCursor();	// restore original cursor
	}

	mw->setMessage(i18n("Ready"));
}


void
Directory::createDir(const QString& dirName)
{
#ifndef Q_WS_WIN
	KURL url = KURL(getProtocol()+":"+fullName()+dirName);
	KonqOperations::mkdir(mw, url);
	setExpandable(true);
#endif
}

void
Directory::createAlbum(const QString& albumName)
{
	KURL urld = KURL(getProtocol()+":"+fullName()+albumName);
	KURL urlo = KURL(getProtocol()+":"+KStandardDirs().findResource("templates",".source/TextFile.txt"));
	KIO::copy(urlo, urld, false);
	setExpandable(true);
	mw->addToBookmark(i18n("Albums"),fullName()+albumName);
	if(isOpen())
		(void)new Album(this, albumName, mw);
}

void
Directory::rename()
{
	if(!newDirName.isEmpty())
	{
		f.setName(newDirName);
		full=path()+"/"+newDirName+"/";
		repaint();
	}
}

bool
Directory::rename(const QString& newDirName, QString& msg)
{
	this->newDirName=newDirName;

	QString oldN=fullName();
	QString newN=path()+"/"+newDirName;

	if(QFileInfo(newN).exists())
	{
		msg=i18n("The directory '<b>%1</b>' already exists").arg(newN);
		return false;
	}

	KURL orgName = KURL(getProtocol()+":"+oldN);
	KURL newName = KURL(getProtocol()+":"+newN);

	KIO::FileCopyJob *renameJob=KIO::file_move(orgName, newName, true );
	QObject::connect(renameJob, SIGNAL(result( KIO::Job *)),
					 getDirectoryView(), SLOT(renameDone( KIO::Job *)));

	f.setName(newDirName);
	repaint();
	updateChildren();

	return true;
}


void
Directory::properties()
{
}

void
Directory::updateChildren()
{
	ListItem::updateChildren();
	if(isSelected())
	{
		FileIconItem *item;
		for ( item = list.first(); item; item = list.next() )
		{
			item->setPath(fullName());
		}
	}
}

bool Directory::checkAccess()
{
	if (QDir(fullName()).isReadable())
		return true;
	mw->showUnableOpenDirectoryError(fullName());
	return false;
}

