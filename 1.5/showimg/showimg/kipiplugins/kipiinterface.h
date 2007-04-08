/***************************************************************************
                          kipiinterface.cpp  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2005 by Aur�lien G�teau, Richard Groult
    email                : aurelien.gateau@free.fr, rgroult@jalix.org
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

#ifndef SHOWIMGKIPIINTERFACE_H
#define SHOWIMGKIPIINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_KIPI

#include <libkipi/interface.h>

class ShowImgKIPIInterfacePrivate;
class MainWindow;

class ShowImgKIPIInterface :public KIPI::Interface
{
	Q_OBJECT

public:
	ShowImgKIPIInterface( MainWindow* parent );
	virtual ~ShowImgKIPIInterface();

	KIPI::ImageCollection currentAlbum();
	KIPI::ImageCollection currentSelection();
	QValueList<KIPI::ImageCollection> allAlbums();
	KIPI::ImageInfo info( const KURL& );
	int features() const;
	bool addImage(const KURL&, QString& err);
	virtual void refreshImages( const KURL::List& urls );

	void selectionChanged( bool b );
	void currentAlbumChanged( const KURL &a_url );

private:
	ShowImgKIPIInterfacePrivate* d;
	KURL m_url;
};

#endif /* HAVE_KIPI */

#endif /* KIPIINTERFACE_H */
