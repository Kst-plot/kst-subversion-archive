/*****************************************************************************
                          compressedfileitem.h -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#ifndef COMPRESSEDFILEITEM_H
#define COMPRESSEDFILEITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "listitem.h"

// Qt
#include <qfile.h>
#include <qptrlist.h>

class CompressedImageFileIconItem;
class Directory;
class DirectoryView;
class ImageListView;
class ImageViewer;

class QFile;
class QString;

class CompressedFileItem : public ListItem
{
public:
	CompressedFileItem (
			ListItem *a_p_parent,
			const QString& a_filename,
			const QString& a_path);

	virtual ~CompressedFileItem ();

	virtual void load (bool a_refresh=true);
	virtual void unLoad ();
	virtual void removeIconItem ( FileIconItem* a_p_item);

	virtual uint getNumberOfItems() const;

	bool rename(const QString& a_newDirName, QString& a_msg);
	virtual bool acceptDrop()const{return false;} ;

	virtual QString key (int a_column, bool a_ascending) const;

private:
	void updateChildren();

	QPtrList < CompressedImageFileIconItem > m_list;
	int m_numberOfItems;
};


#endif
