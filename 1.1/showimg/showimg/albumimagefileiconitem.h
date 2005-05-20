/***************************************************************************
                          albumimagefileiconitem.h  -  description
                             -------------------
    begin                : Sun Jan 6 2002
    copyright            : (C) 2002 by Richard Groult
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef ALBUMIMAGEFILEICONITEM_H
#define ALBUMIMAGEFILEICONITEM_H

// Local 
#include "imagefileiconitem.h"

/**
  *@author Richard Groult
  */

class ImageListView;
class Album;
class ImageViewer;
class MainWindow;

class AlbumImageFileIconItem : public ImageFileIconItem
{
public: 
	
	 AlbumImageFileIconItem(Album *album, const QString& fullname, MainWindow *mw);
	~AlbumImageFileIconItem();
	
	virtual bool suppression();
	virtual bool suppression(bool suprFile);
	virtual bool moveToTrash();
	virtual bool shred();
	QString toolTipStr() const;


protected:
	Album *album;
	
	void removeEntry();
};

#endif
