/***************************************************************************
                          compressedimagefileiconitem.h -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
                               2003 OGINO Tomonori
    email                : rgroult@jalix.org ogino@nn.iij4u.or.jp
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


#ifndef COMPRESSEDIMAGEFILE_H
#define COMPRESSEDIMAGEFILE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "imagefileiconitem.h"

class CompressedFileItem;
class ImageViewer;
class MainWindow;

class CompressedImageFileIconItem : public ImageFileIconItem
{
public:
	CompressedImageFileIconItem (
	        		 CompressedFileItem * parentDir,
	        		 const QString& archive,
				 const QString& filename,
				 MainWindow *mw);

	~CompressedImageFileIconItem ();

	QString text (int column) const;
	QString text () const;
	QString toolTipStr() const;

	bool suppression (bool del);
	virtual bool moveToTrash();

private:
	bool readable;

	QString filename;
	QString archive;

	CompressedFileItem *parentDir;
};


#endif
