/***************************************************************************
                           extract.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
                               2003 OGINO Tomonori
                               2003 Ian Monroe
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp ian@monroe.nu
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

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

#include "extract.h"

#include "krar.h"

// KDE
#include <kprocess.h>
#include <kio/job.h>
#include <kzip.h>
#include <kar.h>
#include <ktar.h>
#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

// Qt
#include <qdir.h>
#include <qvaluelist.h>

void
Extract::getEntryRecursive( const KArchiveDirectory * entry, const QString& path )
{
	QStringList const & entries(entry->entries());
	QStringList::const_iterator it = entries.begin();
	for(; it != entries.end(); ++it) {
		QString add = path + QChar('/') + *it;
		files += add;
		const KArchiveEntry * child = entry->entry(*it);
		if( child->isDirectory() ){
			const KArchiveDirectory * childdir;
			childdir = dynamic_cast<const KArchiveDirectory *>(child);
			getEntryRecursive( childdir, path + QChar('/') + *it );
		}
	}
}

Extract::Extract (const QString& _filename)
{
	QFileInfo fileinfo ( _filename );

	QString  tmpdir = locateLocal("tmp", "showimg-cpr/");

	dest = tmpdir + '/' + fileinfo.fileName() + '/';
	ext = fileinfo.extension(false).lower();

        KMimeType::Ptr mime = KMimeType::findByPath( _filename );
        // KArchive - sync
	KArchive * arc=NULL;
	if ( mime->name() == QString::fromLatin1("application/x-zip" ))
		arc = new KZip( fileinfo.absFilePath() );
	else if ( mime->name() == QString::fromLatin1("application/x-tar")
			|| mime->name() == QString::fromLatin1("application/x-tarz")
			|| mime->name() == QString::fromLatin1("application/x-tgz")
			|| mime->name() == QString::fromLatin1("application/x-tbz" ))
		arc = new KTar( fileinfo.absFilePath() );
	else if ( mime->name() == QString::fromLatin1("application/x-rar" ))
	{
		arc = new KRar( fileinfo.absFilePath() );
	}
	else if ( mime->name() == QString::fromLatin1("application/x-archive" ))
		arc = new KAr( fileinfo.absFilePath() );

	if(arc)
	{
		if ( arc->open( IO_ReadOnly ) )
		{
			const KArchiveDirectory * dir = arc->directory();

			dir->copyTo( dest );
			getEntryRecursive( dir, QString() );
		}
		files.sort();
	}
	else
		KMessageBox::error( 0,
			"<qt>"+i18n("Unable to open the archive '<b>%1</b>'.").arg(fileinfo.absFilePath())+"</qt>",
			i18n("Archive Error"));
	arc->close();
}


bool
Extract::canExtract( const QString& _filename )
{
	QFileInfo info( _filename );
        //Use KMime to check filetypes out.
        KMimeType::Ptr mime;
        mime = KMimeType::findByPath( _filename, 0, true );
#if  KDE_VERSION < 0x30200
	if( mime->name() == KMimeType::defaultMimeType())
		mime = KMimeType::findByFileContent( _filename );
        if( mime->name() == QString::fromLatin1("application/x-zip")
         || mime->name() == QString::fromLatin1("application/x-tar")
         || mime->name() == QString::fromLatin1("application/x-tarz")
         || mime->name() == QString::fromLatin1("application/x-tgz")
         || mime->name() == QString::fromLatin1("application/x-tbz")
         || mime->name() == QString::fromLatin1("application/x-archive" ))
            return true;
#else
	if( mime->is(KMimeType::defaultMimeType()))
		mime = KMimeType::findByFileContent( _filename );
			if( mime->is("application/x-zip")
				|| mime->is("application/x-tar")
				|| mime->is("application/x-tarz")
				|| mime->is("application/x-tgz")
				|| mime->is("application/x-rar")
				|| mime->is("application/x-archive") )
					return true;
#endif
	return false;
}

