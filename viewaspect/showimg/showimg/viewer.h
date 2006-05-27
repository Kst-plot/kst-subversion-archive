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
class MainWindow;

class Viewer : public QWidgetStack
{
public:
	enum AvailableViewers
	{
		AV_ImaveViewer=0,
		AV_MovieViewer,
		AV_SVGViewer
	};

	Viewer(MainWindow *mw, QWidget *parent);

	void openURL(KURL url, QString mimetype);

	void setImageViewer(ImageViewer *iv);
	void setMovieViewer(KParts::ReadOnlyPart *mv);
	void setSVGViewer(KParts::ReadOnlyPart *ksvgViewer);

	void setVisibleImageViewer();
	void setVisibleMovieViewer();
	void setVisibleSVGViewer();

protected:
	ImageViewer *iv;
	KParts::ReadOnlyPart *mv;
	KParts::ReadOnlyPart *ksvgViewer;
	AvailableViewers currentViewer;

	MainWindow *mw;

	bool loaded;
};

#endif

