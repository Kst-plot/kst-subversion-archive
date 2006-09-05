/***************************************************************************
                          viewer.cpp  -  description
                             -------------------
    begin                : sam nov 16 2002
    copyright            : (C) 2002-2005 by Richard Groult
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

#include "viewer.h"

//Local
#include "mainwindow.h"
#include "directoryview.h"

//KDE
#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

Viewer::Viewer(MainWindow *mw, QWidget *parent)
	:QWidgetStack(parent)
{
	iv=NULL;
	mv=NULL;
	ksvgViewer=NULL;

	this->mw=mw;

	//loaded=false;
	currentViewer=AV_ImaveViewer;
}

void
Viewer::setImageViewer(ImageViewer *iv)
{
	this->iv=iv;
	addWidget(iv);
}

void
Viewer::setMovieViewer(KParts::ReadOnlyPart *mv)
{
	this->mv=mv;
	if(mv) addWidget(mv->widget());
}

void
Viewer::setSVGViewer(KParts::ReadOnlyPart *ksvgViewer)
{
	this->ksvgViewer=ksvgViewer;
	if(ksvgViewer) addWidget(ksvgViewer->widget());
}

void
Viewer::setVisibleImageViewer()
{
	if(id(iv)==id(visibleWidget())) return;
	raiseWidget(iv);
}

void
Viewer::setVisibleMovieViewer()
{
	if(!mv) return;

	if(id(mv->widget())==id(visibleWidget())) return;
	raiseWidget(mv->widget());
}
void
Viewer::setVisibleSVGViewer()
{
	if(!ksvgViewer) return;

	if(id(ksvgViewer->widget())==id(visibleWidget())) return;
	raiseWidget(ksvgViewer->widget());
}

void
Viewer::openURL(KURL url, QString mimetype)
{
	if(mimetype.left(5) == QString::fromLatin1("video") && ! DirectoryView::isImage(url.path()))
	{
		KApplication::setOverrideCursor (waitCursor); // this might take time
		if(mw->getDirectoryView()->getShowVideo() && currentViewer != AV_MovieViewer)
		{
			currentViewer = AV_MovieViewer;
			iv->loadImage(NULL);

			mw->updateGUI( currentViewer );
			setVisibleMovieViewer();
		}
		if(mv) mv->openURL(url);
		KApplication::restoreOverrideCursor ();
	}
	else
	if(mimetype == QString::fromLatin1("image/svg+xml") || mimetype== QString::fromLatin1("image/svg-xml"))
	{
		KApplication::setOverrideCursor (waitCursor); // this might take time
		if(currentViewer != AV_SVGViewer)
		{
			currentViewer = AV_SVGViewer;
			iv->loadImage(NULL);

			mw->updateGUI( currentViewer );
			setVisibleSVGViewer();
		}
		if(ksvgViewer) ksvgViewer->openURL(url);
		KApplication::restoreOverrideCursor ();
	}
	else
	{
		if(currentViewer != AV_ImaveViewer)
		{
			currentViewer = AV_ImaveViewer;

			mw->updateGUI( currentViewer );
			setVisibleImageViewer();
			loaded=false;
		}
		if(iv) iv->loadImage(url.path());
	}
}




