/***************************************************************************
                          imagelistviewsimple.h  -  description
                             -------------------
    begin                : Jan 9 2005
    copyright            : (C) 2005 by Richard Groult
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

#ifndef IMAGELISTVIEWSIMPLE_H
#define IMAGELISTVIEWSIMPLE_H

#include "showimg_export.h"

// QT
#include <qobject.h>
#include <qstringlist.h>

class MainWindow;
class ShowimgOSD;
class ImageMetaInfo;
class ImageViewer;

class KActionCollection;
class KConfig;

class QTimer;

class SHOWIMGCORE_EXPORT ImageListViewSimple: public QObject
{
Q_OBJECT
public:
	ImageListViewSimple(QObject *parent,
						const QString& imageFilePath=QString::null, ImageViewer *imageviewer=NULL);

	virtual ~ImageListViewSimple();

	void initActions(KActionCollection *);
	void readConfig(KConfig *config, bool hideOSD=false);

	void load();

	void setImageFilePath(const QString& imageFilePath);
	QString currentAbsImagePath();

	void startSlideshow(int slideshowTime);

public slots:
	void next ();
	void previous ();
	void first ();
	void last ();

protected:
	void updateOSD(const QString& currentfile);

protected:
	QString currentPath;
	QString imageFilePath;
	QStringList *imagepathList;

	ImageViewer *m_imageviewer;
	ImageMetaInfo *m_imageMetaInfo;
	ShowimgOSD *m_OSDWidget;

	QStringList::iterator currentPos;

	QTimer *timer;
};

#endif
