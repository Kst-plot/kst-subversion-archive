/***************************************************************************
                          viewer.h  -  description
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
 ***************************************************************************/

#ifndef VIEWER_H
#define VIEWER_H

#include "imageviewer.h"

#include <qwidgetstack.h>

#include <kparts/part.h>

class KURL;

class Viewer : public QWidgetStack
{
public:
	enum AvailableViewers
	{
		AV_ImaveViewer=0,
		AV_MovieViewer,
		AV_SVGViewer
	};

	Viewer(QWidget *parent);

	void openURL(const KURL & a_url, const QString & mimetype);

	void setImageViewer(ImageViewer *a_p_iv);
	void setMovieViewer(KParts::ReadOnlyPart *a_p_mv);
	void setSVGViewer(KParts::ReadOnlyPart *a_p_ksvgViewer);

	void setVisibleImageViewer();
	void setVisibleMovieViewer();
	void setVisibleSVGViewer();

private :
	ImageViewer *m_p_iv;
	KParts::ReadOnlyPart *m_p_mv;
	KParts::ReadOnlyPart *m_p_ksvgViewer;
	AvailableViewers m_currentViewer;
};

#endif

