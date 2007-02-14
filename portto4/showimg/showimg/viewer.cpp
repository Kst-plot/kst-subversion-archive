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

//-----------------------------------------------------------------------------
#define VIEWER_DEBUG 0
//-----------------------------------------------------------------------------

//Local
#include "mainwindow.h"
#include "directoryview.h"
#include "showimg_common.h"
#include "tools.h"

//KDE
#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

Viewer::Viewer(QWidget *parent)
	:QWidgetStack(parent)
{
#if VIEWER_DEBUG > 0
	MYDEBUG<< endl;
#endif
	m_p_iv          = NULL;
	m_p_mv          = NULL;
	m_p_ksvgViewer  = NULL;

	m_currentViewer = AV_ImaveViewer;
}

void
Viewer::setImageViewer(ImageViewer *a_p_iv)
{
	m_p_iv = a_p_iv;
	addWidget(m_p_iv);
}

void
Viewer::setMovieViewer(KParts::ReadOnlyPart *a_p_mv)
{
	m_p_mv = a_p_mv;
	if(m_p_mv)
		addWidget(m_p_mv->widget());
}

void
Viewer::setSVGViewer(KParts::ReadOnlyPart *a_p_ksvgViewer)
{
	m_p_ksvgViewer = a_p_ksvgViewer;
	if(m_p_ksvgViewer)
		addWidget(m_p_ksvgViewer->widget());
}

void
Viewer::setVisibleImageViewer()
{
	if(id(m_p_iv)==id(visibleWidget()))
		return;
	raiseWidget(m_p_iv);
}

void
Viewer::setVisibleMovieViewer()
{
	if(!m_p_mv)
		return;

	if(id(m_p_mv->widget())==id(visibleWidget()))
		return;
	raiseWidget(m_p_mv->widget());
}
void
Viewer::setVisibleSVGViewer()
{
	if(!m_p_ksvgViewer)
		return;

	if(id(m_p_ksvgViewer->widget())==id(visibleWidget()))
		return;
	raiseWidget(m_p_ksvgViewer->widget());
}

void
Viewer::openURL(const KURL& a_url, const QString & a_mimetype)
{
#if VIEWER_DEBUG > 0
	MYDEBUG<<"BEGIN"<<a_url.url()<<"-"<< a_mimetype<< endl;
#endif
	if(
		  Tools::isVideo(a_url, a_mimetype) &&
		! Tools::isImage(a_url, a_mimetype)
	)
	{
#if VIEWER_DEBUG > 0
		MYDEBUG<< "isvideo && !isimage " <<endl;
#endif
		KApplication::setOverrideCursor (waitCursor);
		if(
			MAINWINDOW.getDirectoryView()->getShowVideo() &&
			m_currentViewer != AV_MovieViewer
		)
		{
			m_currentViewer = AV_MovieViewer;
			m_p_iv->openURL(NULL);

			MAINWINDOW.updateGUI( m_currentViewer );
			setVisibleMovieViewer();
		}
		if(m_p_mv)
			m_p_mv->openURL(a_url);
		KApplication::restoreOverrideCursor ();
	}
	else
	if(Tools::isSVG(a_url, a_mimetype))
	{
#if VIEWER_DEBUG > 0
		MYDEBUG<< "isSVG" <<endl;
#endif
		KApplication::setOverrideCursor (waitCursor); // this might take time
		if(m_currentViewer != AV_SVGViewer)
		{
			m_currentViewer = AV_SVGViewer;
			m_p_iv->openURL(NULL);

			MAINWINDOW.updateGUI( m_currentViewer );
			setVisibleSVGViewer();
		}
		if(m_p_ksvgViewer)
			m_p_ksvgViewer->openURL(a_url);
		KApplication::restoreOverrideCursor ();
	}
	else
	{
#if VIEWER_DEBUG > 0
		MYDEBUG<< endl;
#endif
		if(m_currentViewer != AV_ImaveViewer)
		{
#if VIEWER_DEBUG > 0
			MYDEBUG<< endl;
#endif
			m_currentViewer = AV_ImaveViewer;

			MAINWINDOW.updateGUI( m_currentViewer );
			setVisibleImageViewer();
		}
		if(m_p_iv)
			m_p_iv->openURL(a_url);
	}
#if VIEWER_DEBUG > 0
		MYDEBUG<<"END"<<endl;
#endif
}




