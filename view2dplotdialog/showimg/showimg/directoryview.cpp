/***************************************************************************
                          directoryview.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#include "directoryview.h"

// Local
#include "describealbum.h"
#include "directory.h"
#include "imageviewer.h"
#include "imagefileiconitem.h"
#include "compressedimagefileiconitem.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "album.h"
#include "listitem.h"
#include "krar.h"

#ifdef HAVE_KIPI
#include "kipiplugins/kipipluginmanager.h"
#endif /* HAVE_KIPI */

// Qt
#include <qstring.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qprogressdialog.h>
#include <qdropsite.h>
#include <qdragobject.h>
#include <qtooltip.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qclipboard.h>
#include <qpopupmenu.h>
#include <qheader.h>

// KDE
#include <kurlrequesterdlg.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>
#include <kglobalsettings.h>
#include <kpropertiesdialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kdirwatch.h>
#include <kapplication.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kimageio.h>
#include <kmimetype.h>
#include <kinputdialog.h>
#include <kdirlister.h>

#ifndef Q_WS_WIN
#include <konq_operations.h>
#endif

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

DirectoryView::DirectoryView (
			QWidget *parent,
			MainWindow *mw,
			const char* name)
	: ListItemView (parent, mw, name),

	m_showCompressedFiles(true)
{
	setShowHiddenDir(false);

	/////
	dirWatch = new KDirWatch();
	connect(dirWatch, SIGNAL(dirty (const QString&)),
			mw, SLOT(slotDirChange(const QString&)));
	connect(dirWatch, SIGNAL(created (const QString&)),
			mw, SLOT(slotDirChange_created(const QString&)));
	connect(dirWatch, SIGNAL(deleted (const QString&)),
			mw, SLOT(slotDirChange_deleted(const QString&)));

	//
	m_dirLister = new KDirLister();

	connect(mw, SIGNAL(lastDestDirChanged(const QString&)),
			this, SLOT(updateDestDirTitle(const QString&)));


	startWatchDir();
}


DirectoryView::~DirectoryView()
{
}

void
DirectoryView::readConfig(KConfig *config)
{
	config->setGroup("Options");
	setShowHiddenDir(config->readBoolEntry("showhiddenDir", false));
	setShowHiddenFile(config->readBoolEntry("showhiddenFile", false));
	setShowDir(config->readBoolEntry("showDir", true));
	setShowAllFile(config->readBoolEntry("showallFile", false));
	setLoadFirstImage(config->readBoolEntry("loadFirstImage", false));
	setShowVideo(config->readBoolEntry("enable video", true));
	setUnrarPath(config->readPathEntry("unrarPath", "unrar"));
	setShowCompressedFiles(config->readBoolEntry("showArchives", true));

	config->setGroup("DirectoryView");
	setColumnWidth (COLUMN_TYPE, config->readNumEntry("COLUMN_TYPE", 0 ));
	setColumnWidth (COLUMN_SIZE, config->readNumEntry("COLUMN_SIZE", 60 ));
	setColumnWidth (COLUMN_SELECT, config->readNumEntry("COLUMN_SELECT", 3*ListItem::TREE_CHECKBOX_MAXSIZE/2 ));
}

void
DirectoryView::writeConfig(KConfig *config)
{
	config->setGroup("Options");
	config->writeEntry( "showhiddenDir", showHiddenDir() );
	config->writeEntry( "showhiddenFile", showHiddenFile() );
	config->writeEntry( "showDir", showDir() );
	config->writeEntry( "showallFile", showAllFile() );
	config->writeEntry( "loadFirstImage", loadFirstImage() );
	config->writeEntry( "enable video", getShowVideo() );
	config->writePathEntry( "unrarPath", getUnrarPath() );
	config->writeEntry( "showArchives", getShowCompressedFiles() );

	config->setGroup("DirectoryView");
	config->writeEntry( "COLUMN_TYPE", columnWidth(COLUMN_TYPE) );
	config->writeEntry( "COLUMN_SIZE", columnWidth(COLUMN_SIZE) );
	config->writeEntry( "COLUMN_SELECT", columnWidth(COLUMN_SELECT) );


	config->sync();
}


void
DirectoryView::initActions(KActionCollection *actionCollection)
{
	aDirCopy =
			new KAction(i18n("Copy Folder To..."), "editcopy", 0,
						this, SLOT(slotDirCopy()),
						actionCollection, "editdircopy");
	aDirMove =
			new KAction(i18n("Move Folder To..."), 0,
						this, SLOT(slotDirMove()),
						actionCollection, "editdirmove");

	aDirMoveToLast =
			new KAction(i18n("Move Folder to Last Directory"), 0,
								this, SLOT(slotDirMoveToLast()),
								actionCollection, "moveDirToLast");
	aDirCopyToLast =
			new KAction(i18n("Copy Folder to Last Directory"), 0,
								this, SLOT(slotDirCopyToLast()),
								actionCollection, "copyDirToLast");

	aDirPasteFiles =
			new KAction(i18n("Paste Files"), "editpaste", 0,
						this, SLOT(slotDirPasteFiles()),
						actionCollection , "editdirpaste files");
	aDirRecOpen =
			new KAction(i18n("Recursively Open"), 0,
						this, SLOT(recursivelyOpen()),
						actionCollection , "dirRecOpen");

	aDirRename =
			new KAction(i18n("&Rename Item..."), "item_rename", 0,
						this, SLOT(slotRename()),
						actionCollection , "editdirrename");

	aDirTrash =
			new KAction(i18n("&Move Item to Trash"), "edittrash", 0,
						this, SLOT(slotTrash()),
						actionCollection , "editdirtrash");
	aDirDelete =
			new KAction(i18n("&Delete"), "editdelete", 0,
						this, SLOT(slotSuppr()),
						actionCollection , "editdirdelete");

	aPreviousDir =
			new KAction(i18n("Previous Directory"), "1leftarrow", KShortcut(SHIFT+Key_Space),
						this, SLOT(goToPreviousDir()),
						actionCollection, "Previous Directory" );
	aNextDir =
			new KAction(i18n("Next Directory"), "1rightarrow", KShortcut(CTRL+Key_Space),
						this, SLOT(goToNextDir()),
						actionCollection, "Next Directory");

	aDirInfo =
			new KAction(i18n("Describe Directory..."), 0,
						this, SLOT(slotDirInfo()),
						actionCollection, "Dir Info");
	aDirProperties =
			new KAction(i18n("Properties"), "info", 0,
						this, SLOT(slotDirProperty()),
						actionCollection, "Dir Properties");

	aDirNewFolder =
			new KAction(i18n("New Directory..."), "folder_new", 0,
						this, SLOT(slotNewDir()),
						actionCollection, "editdirnew");
	aDirNewAlbum =
			new KAction(i18n("New Album..."), "txt", 0,
						this, SLOT(slotNewAlbum()),
						actionCollection, "editalbumnew");

	////////////////////////////////////////////////////////////////////////////
	aDetailType =
			new KAction(i18n("Show/Hide Type"), 0,
						this, SLOT(slotShowHideDetail_Type()),
						actionCollection, "dirview showhide type");
	aDetailSize =
			new KAction(i18n("Show/Hide Size"), 0,
						this, SLOT(slotShowHideDetail_Size()),
						actionCollection, "dirview showhide size");
	aDetailSelect  =
			new KAction(i18n("Show/Hide Select"), 0,
						this, SLOT(slotShowHideDetail_Select()),
						actionCollection, "dirview showhide select");
	KActionMenu *aDetailShowHide =
			new KActionMenu( i18n("Directory Details"), "view_tree",
							 actionCollection, "dirview details" );
	aDetailShowHide->insert(aDetailType);
	aDetailShowHide->insert(aDetailSize);
	aDetailShowHide->insert(aDetailSelect);


	////////////////////////////////////////////////////////////////////////////
	connect(this, SIGNAL(sigTotalNumberOfFiles(int)),
			mw, SLOT(slotAddImage(int)));
	connect(this, SIGNAL(sigHasSeenFile(int)),
			mw, SLOT(slotPreviewDone(int)));
	connect(this, SIGNAL(loadingFinished(int)),
			mw, SLOT(slotDone(int)));

}

void
DirectoryView::initMenu(KActionCollection *actionCollection)
{
	this->actionCollection=actionCollection;
	popup = new KPopupMenu();
	popup->insertTitle("", 1);
	if(!actionCollection) return;

	actionNewItems =
			new KActionMenu( i18n("Create &New"), "filenew",
							 actionCollection, "dirview create_new_items" );
	actionNewItems->insert(aDirNewFolder);
	actionNewItems->insert(aDirNewAlbum);

	actionNewItems->plug(popup);
	aDirRecOpen->plug(popup);

	//
	popup->insertSeparator();
	aCopyActions =
			new KActionMenu( i18n("Copy"), 0,
							actionCollection, "Copy Folders actions" );
	aCopyActions->plug(popup);
	aCopyActions->popupMenu()->insertTitle(i18n("Copy..."), 1);
	aCopyActions->insert(aDirCopyToLast);
	aCopyActions->insert(aDirCopy);

	aMoveActions =
			new KActionMenu( i18n("Move"), 0,
							 actionCollection, "Move Folders actions" );
	aMoveActions->plug(popup);
	aMoveActions->popupMenu()->insertTitle(i18n("Move..."), 1);
	aMoveActions->insert(aDirMoveToLast);
	aMoveActions->insert(aDirMove);

	//
	aDirRename->plug(popup);
	aDirTrash->plug(popup);
	aDirDelete->plug(popup);
	popup->insertSeparator();
	aDirPasteFiles->plug(popup);
	popup->insertSeparator();
	aDirInfo->plug(popup);
	aDirProperties->plug(popup);
}


void
DirectoryView::updateActions(ListItem *item)
{
	if(isDropping() || !actionCollection) return ;
	bool enableAction=true;
	if(!item)
	{
		mw->getImageListView()->load(NULL);
		enableAction=false;
	}
	else
	{
		if(!(item->getType () == QString::fromLatin1("directory") || item->getType () == QString::fromLatin1("album"))) enableAction=false;
	}
	aDirPasteFiles->setEnabled(enableAction);
	aDirNewFolder->setEnabled(enableAction);
	aDirNewAlbum->setEnabled(enableAction);
	aDirRecOpen->setEnabled(enableAction);
	aDirCopy->setEnabled(enableAction);
	aPreviousDir->setEnabled(enableAction);
	aNextDir->setEnabled(enableAction);
	aDirProperties->setEnabled(enableAction);
	actionNewItems->setEnabled(enableAction);

	enableAction = item&&!item->isReadOnly();
	const bool isRoot =
#ifdef Q_WS_WIN
			QDir( QDir::convertSeparators(item->fullName()) ).isRoot();
#else
		false;
#endif
	aDirMove->setEnabled(enableAction && !isRoot);
	aDirCopyToLast->setEnabled(enableAction && !isRoot && !mw->getLastDestDir().isEmpty());
	aDirMoveToLast->setEnabled(enableAction && !isRoot && !mw->getLastDestDir().isEmpty());
	aDirTrash->setEnabled(enableAction && !isRoot);
	aDirDelete->setEnabled(enableAction && !isRoot);
	aDirPasteFiles->setEnabled(enableAction);
	aDirRename->setEnabled(enableAction && !isRoot);
	aDirInfo->setEnabled(enableAction);
}


ListItem*
DirectoryView::getDir(const QString& path)
{
	if(isImage(path)) return NULL;
	ListItem *ssrep;
	ListItem *rootItems = firstChild ();
	while(rootItems)
	{
		if(path.startsWith(rootItems->fullName()))
		{
			ssrep = rootItems->find(path);
			if (ssrep)
			{
				return ssrep;
			}
		}
		rootItems = rootItems->nextSibling();
	}
	return NULL;
}


void
DirectoryView::removeDir(const QString& dirfullname)
{
	ListItem * item = getDir(dirfullname);
	if(item)
	{
		if(item->getType() == QString::fromLatin1("directory"))
			((Directory*)item)->recursivelyDelete();
		delete(item);
	}
	else
	{
	}
}


void
DirectoryView::slotDirInfo()
{
	if(clickedItem)
	{
		   (void)DescribeAlbum(mw, clickedItem->fullName()).exec();
	}
}


void
DirectoryView::slotDirProperty()
{
	if(!clickedItem) clickedItem=currentItem();
	if(clickedItem)
	{
		KApplication::setOverrideCursor (waitCursor);
		KFileItem *item = new KFileItem(KFileItem::Unknown,
				KFileItem::Unknown,
				clickedItem->getURL(),
				true);
			KPropertiesDialog prop( item,
			mw->getImageViewer(), "KPropertiesDialog",
			true, false);

		KApplication::restoreOverrideCursor ();
		prop.exec();
		delete(item);
	}
}


void
DirectoryView::startWatchDir(QString dir)
{
	KURL mDirURL; mDirURL.setPath(dir);
	QFileInfo info(dir);
	if(info.isDir())
	{
		dirWatch->addDir(dir);
		m_dirLister->openURL(mDirURL);
	}
	else
	if(info.isFile())
	{
		dirWatch->addFile(dir);
	}
}


void DirectoryView::stopWatchDir(QString dir)
{
	if(QFileInfo(dir).isDir())
		dirWatch->removeDir(dir);
	else
	if(QFileInfo(dir).isFile())
		dirWatch->removeFile(dir);
}


void
DirectoryView::startWatchDir()
{
	dirWatch->startScan(true, true);
}


void
DirectoryView::stopWatchDir()
{
}


void
DirectoryView::contentsDropEvent (QDropEvent * event)
{
	contentsDropEvent_begin();

	if ( !QUriDrag::canDecode(event) || !dropItem)
	{
		contentsDropEvent_end();
		event->ignore();
	}
	else
	{
		event->acceptAction(true);
// 		mw->getImageListView()->stopLoading();

		QStrList lst;
		if (QUriDrag::decode (event, lst))
		{
			event->acceptAction();
			if (dropItem->getType() == QString::fromLatin1("album"))
				((Album*)dropItem)->addURL(QStringList::fromStrList(lst));
			else
			if (dropItem->getType() != "directory")
			{
				contentsDropEvent_end();
				KMessageBox::error (this,
						"<qt>"+i18n("Adding file in '%1' is not yet implemented")
							.arg(((ListItem*) dropItem)->text (1))+"</qt>",
					 	i18n("File(s) Copy/Move"));
			}
			else
			if(!QFileInfo(((Directory*)dropItem)->fullName()).isWritable ())
			{
				contentsDropEvent_end();
				KMessageBox::error (this,
						i18n("The destination directory is not writable"),
						i18n("File(s) Copy/Move"));
			}
			else
			switch (event->action ())
			{
				case QDropEvent::Copy:
				case QDropEvent::Move:
					{
						bool canBeMoved = true;
						if(event->source() == mw->getImageListView())
							canBeMoved = mw->getImageListView()->currentDragItemAreMovable();

						if (event->action () == QDropEvent::Move && canBeMoved )
							move(QStringList::fromStrList(lst), dropItem->fullName());
						else
							copy(QStringList::fromStrList(lst), dropItem->fullName());
						break;
					}
				default:break;
			}
		}
		contentsDropEvent_end();
	}
}


void
DirectoryView::copyingDirDone( /*KIO::CopyJob*/ KIO::Job *job)
{
	if(job->error()==0)
	{
		ListItem* dest=getDir(dirDest);
		if(dest)
		{
			if(!dest->isOpen())
			{
				//rien a faire
			}
			else
			{
				QString name=QDir(dirOrg).dirName();
				if(!getDir(dirDest+name))
				{
					//ajout du rep
					if(dest->getType() == QString::fromLatin1("directory"))
					{
						(void)new Directory( (Directory*)dest, name, mw);
					}
					dest->setExpandable(true);
				}
			}
		}
		else
		{
		}
	}
	else
	{
		job->showErrorDialog(mw);
	}

}


void
DirectoryView::movingDirDone( KIO::Job *job)
{
	if(job->error()==0)
	{
		mw->slotRefresh();
		emit moveFilesDone(((KIO::CopyJob*)job)->srcURLs(), ((KIO::CopyJob*)job)->destURL());
	}
	else
		job->showErrorDialog(mw);
}

void
DirectoryView::renameDone( KIO::Job *job)
{
	if(job->error()==0)
		emit renameListItemDone(((KIO::FileCopyJob*)job)->srcURL(), ((KIO::FileCopyJob*)job)->destURL());
	else
		job->showErrorDialog();
}

void
DirectoryView::copyingDone( KIO::Job *job)
{
	if(job->error()==0)
		mw->setLastDestDir( ((KIO::CopyJob*)job)->destURL().path() );
	else
		job->showErrorDialog();
}


void
DirectoryView::movingDone( KIO::Job *job )
{
	if(job->error()==0)
	{
		mw->setLastDestDir( ((KIO::CopyJob*)job)->destURL().path() );
		emit moveFilesDone(((KIO::CopyJob*)job)->srcURLs(), ((KIO::CopyJob*)job)->destURL());
	}
	else
		job->showErrorDialog();
}


bool
DirectoryView::copy (QString *dep, QString *dest)
{
	copy(QStringList(*dep), *dest);
	return true;
}


bool
DirectoryView::move (QString *dep, QString *dest)
{
	move(QStringList(*dep), *dest);
	return true;
}


void
DirectoryView::copy(const QStringList& uris, const QString& dest)
{
#ifndef Q_WS_WIN
	if(!QFileInfo(dest).isDir())
	{
		 KMessageBox::error(mw->getImageViewer(), "<qt>"
		 	+i18n("Unable to copy files into '%1' because it is not a directory.").arg(dest)
			+"</qt>",
			i18n("File(s) Copy"));
		 return;
	}
	KURL urldest;
	urldest.setPath(dest);
	//
	KURL::List list;
	QStringList _uris_=uris;
	KURL url;
	for ( QStringList::Iterator it = _uris_.begin(); it != _uris_.end(); ++it )
	{
		KURL tmp_url(*it);
		url.setPath(tmp_url.path());
		list.append(url);
	}
	KIO::CopyJob *job = KIO::copy(list, urldest);
	connect(job, SIGNAL(result( KIO::Job *)),
		this, SLOT(copyingDone( KIO::Job *)));
#endif
}


void
DirectoryView::move(const QStringList& uris, const QString& dest)
{
#ifndef Q_WS_WIN
	if(!QFileInfo(dest).isDir())
	{
		 KMessageBox::error(mw->getImageViewer(), "<qt>"
		 	+i18n("Unable to move files into '%1' because it is not a directory.").arg(dest)
			+"</qt>",
			i18n("File(s) Move"));
		 return;
	}
	KURL urldest;
	urldest.setPath(dest);
	//
	KURL::List list;
	QStringList _uris_=uris;
	KURL url;
	for ( QStringList::Iterator it = _uris_.begin(); it != _uris_.end(); ++it )
	{
		KURL tmp_url(*it);
		url.setPath(tmp_url.path());
		list.append(url);
	}
	KIO::CopyJob *job = KIO::move(list, urldest);
	connect(job, SIGNAL(result( KIO::Job *)),
		this, SLOT(movingDone( KIO::Job *)));
#endif
}


void
DirectoryView::slotDirPasteFiles()
{
	KURL::List uris;
	if(KURLDrag::decode(KApplication::clipboard()->data(), uris))
	{
		if(!uris.isEmpty())
			copy(uris.toStringList(), clickedItem->fullName());
	}

}


void
DirectoryView::slotNewAlbum()
{
	if(!clickedItem) clickedItem=currentItem();
	slotNewAlbum(clickedItem);
}


void
DirectoryView::slotNewAlbum (ListItem *item)
{
	if(!item) return;

	bool ok;
	QString newName;
	KURL url;

	newName = QString(KInputDialog::getText(i18n("Create New Album in %1").arg(shrinkdn(((Directory*)item)->fullName())),
					i18n("Enter album name:"),
					 i18n("Album"),
					 &ok, mw->getImageViewer()).stripWhiteSpace());
	url = KURL(item->getProtocol()+":"+item->fullName()+"/"+newName+".sia");

	while(ok && !newName.isEmpty() && QFileInfo(url.path()).exists())
	{
		 KMessageBox::error(mw->getImageViewer(), "<qt>"+i18n("The album <b>%1</b> already exists").arg(url.fileName())+"</qt>");

		 newName = QString(KInputDialog::getText(i18n("Create New Album in %1").arg(shrinkdn(((Directory*)item)->fullName())),
						   i18n("Enter album name:"),
						   newName,
						   &ok, mw->getImageViewer()).stripWhiteSpace());
		 url = KURL(item->getProtocol()+":"+item->fullName()+"/"+newName+".sia");
	}
	if(ok && !newName.isEmpty() && !QFileInfo(url.path()).exists())
		((Directory*)item)->createAlbum(url.fileName());
}


void
DirectoryView::slotNewDir()
{
	if(!clickedItem) clickedItem=currentItem();
	slotNewDir(clickedItem);
}


void
DirectoryView::slotNewDir (ListItem *item)
{
	if(!item) return;

	bool ok;
	QString newName;
	KURL url;
	newName = KInputDialog::getText(i18n("Create New Directory in %1").arg(shrinkdn(((Directory*)item)->fullName())),
					i18n("Enter directory name:"),
					 i18n("Directory"),
					 &ok, mw->getImageViewer()).stripWhiteSpace();

	url = KURL(item->getProtocol()+":"+item->fullName()+"/"+newName);
	while(ok && !newName.isEmpty() && QFileInfo(url.path()).exists())
	{
		KMessageBox::error(mw->getImageViewer(), "<qt>"+i18n("The directory ``<b>%1</b>'' already exists").arg(url.fileName())+"</qt>");

		newName = QString(KInputDialog::getText(i18n("Create New Directory in %1").arg(shrinkdn(((Directory*)item)->fullName())),
						  i18n("Enter directory name:"),
						  newName,
						  &ok, mw->getImageViewer()).stripWhiteSpace());
		url = KURL(item->getProtocol()+":"+item->fullName()+"/"+newName);
	}
	if(ok && !newName.isEmpty() && !QFileInfo(url.path()).exists())
		((Directory*)item)->createDir(url.fileName());

}


void
DirectoryView:: slotTrash()
{
	if(!clickedItem) clickedItem=currentItem();
	slotTrash(clickedItem);
}


void
DirectoryView::slotTrash (ListItem *item)
{
#ifndef Q_WS_WIN
	if(!item) return;
	ListItem *dir=(ListItem*)item;
// 	dir->setOpen(false);
	KonqOperations::del(mw, KonqOperations::TRASH, dir->getURL());
#endif
}


void
DirectoryView::slotSuppr (ListItem *item)
{
#ifndef Q_WS_WIN
	if(!item) return;
	Directory *dir=(Directory*)item;
// 	dir->setOpen(false);
	KonqOperations::del(mw, KonqOperations::DEL, dir->getURL());
#endif
}

void
DirectoryView::slotDirCopyToLast()
{
	Directory* item=(Directory*)clickedItem;
	if(!item) return;
	if(mw->getLastDestDir().isEmpty())
	{
		slotDirCopy();
		return;
	}
	//
	QString destDir = mw->getLastDestDir();
	KURL urlorg, urldest;
	urlorg.setPath(item->fullName());
	urldest.setPath(destDir);

	KIO::CopyJob *job = KIO::copy(urlorg, urldest);
	connect(job, SIGNAL(result( KIO::Job *)),
			this, SLOT(copyingDone( KIO::Job *)));
}


void
DirectoryView::slotDirCopy()
{
	Directory* item=(Directory*)clickedItem;
	if(!item)
		return;

	QString destDir =
			KFileDialog::getExistingDirectory(mw->getLastDestDir().isEmpty()?item->fullName():mw->getLastDestDir(),
									mw,
									i18n("Copy Directory %1 To").arg(shrinkdn(item->fullName())));

	if(!destDir.isEmpty())
	{
		mw->setLastDestDir(destDir);
		QString dest=destDir+"/";

		KURL
			urlorg(item->getProtocol()+":"+item->fullName()),
			urldest(item->getProtocol()+":"+dest);
		dirOrg=item->fullName();
		dirDest=dest;

		KIO::CopyJob *job = KIO::copy(urlorg, urldest, true);
		connect(job, SIGNAL(result( KIO::Job *)), this,
				SLOT(copyingDirDone( KIO::Job *)));
	}
}


void
DirectoryView::slotDirMoveToLast()
{
	Directory* item=(Directory*)clickedItem;
	if(!item) return;
	if(mw->getLastDestDir().isEmpty())
	{
		slotDirMove();
		return;
	}
	//
	QString destDir = mw->getLastDestDir();
	KURL urlorg, urldest;
	urlorg.setPath(item->fullName());
	urldest.setPath(destDir);

	KIO::CopyJob *job = KIO::move(urlorg, urldest);
	connect(job, SIGNAL(result( KIO::Job *)),
			this, SLOT(movingDone( KIO::Job *)));
}

void
DirectoryView::slotDirMove()
{
	Directory* item=(Directory*)clickedItem;
	if(!item)
		return;

	QString destDir = KFileDialog::getExistingDirectory(
								mw->getLastDestDir().isEmpty() ? item->fullName() : mw->getLastDestDir(),
								mw,
								i18n("Move Directory %1 To").arg(shrinkdn(item->fullName())));
	QString dest;
	bool ok = true;

	while(!ok && !destDir.isEmpty())
	{
		ok = true;
		dest=destDir+"/"+item->text(0);
		if(QFileInfo(dest).exists())
		{
		 	 KMessageBox::error(mw->getImageViewer(), "<qt>"
			 	+i18n("The directory '<b>%1</b>' already exists").arg(shrinkdn(dest))+"</qt>");
		 	 ok = false;
		}
		else
		if(!QFileInfo(QFileInfo(dest).dirPath()).isWritable())
		{
		 	 KMessageBox::error(mw->getImageViewer(), "<qt>"
			 	+i18n("The directory '<b>%1</b>' is not writable").arg(shrinkdn(dest))+"</qt>");
			 ok = false;
		}
		if(!ok)
		{
			destDir = KFileDialog::getExistingDirectory(mw->getLastDestDir().isEmpty()?item->fullName():mw->getLastDestDir(),
					mw,
					i18n("Move Directory %1 To").arg(shrinkdn(item->fullName())));
		}
	}

	if(!ok || destDir.isEmpty()) return;
	mw->setLastDestDir(destDir);

	KURL urlorg, urldest;
	urlorg.setPath(item->fullName());
	urldest.setPath(destDir);

	KIO::CopyJob *job = KIO::move(urlorg, urldest);
	connect(job, SIGNAL(result( KIO::Job *)),
			this, SLOT(movingDone( KIO::Job *)));

	dirOrg=item->fullName();
	dirDest=destDir+"/";
}


//------------------------------------------------------------------------------


void
DirectoryView::setShowHiddenDir(bool show)
{
	m_showHiddenDir=show;
}


bool
DirectoryView::showHiddenDir()
{
	return m_showHiddenDir;
}

bool
DirectoryView::getShowCompressedFiles()
{
	return m_showCompressedFiles;
}


void
DirectoryView::setShowCompressedFiles(bool show)
{
	m_showCompressedFiles = show;
}


void
DirectoryView::setShowHiddenFile(bool show)
{
	m_showHiddenFile=show;
}


bool
DirectoryView::showHiddenFile()
{
	return m_showHiddenFile;
}


void
DirectoryView::setShowAllFile(bool show)
{
	m_showAllFile=show;
}


void
DirectoryView::setShowDir(bool show)
{
	m_showDir=show;
}


bool
DirectoryView::showDir()
{
	return m_showDir;
}

//------------------------------------------------------------------------------

int
DirectoryView::filter ()
{
	int fil;
	if(showHiddenFile())
		fil = QDir::Hidden|QDir::Files;
	else
		fil = QDir::Files;
// 	if(showDir())
		fil = fil | QDir::Dirs;
	return fil;
}

void
DirectoryView::updateDestDirTitle(const QString& dir)
{
	aCopyActions->popupMenu()->changeTitle(1, dir);
	aMoveActions->popupMenu()->changeTitle(1, dir);
}

void
DirectoryView::setUnrarPath(const QString& path)
{
	KRar::setUnrarPath(path);
// 	m_unurarPath = path;
}
QString
DirectoryView::getUnrarPath()
{
	return KRar::getUnrarPath();
// 	return m_unurarPath;
}

bool
DirectoryView::getShowVideo()
{
	return m_showVideo;
}
void
DirectoryView::setShowVideo(bool show)
{
	m_showVideo = show;
}


#include "directoryview.moc"
