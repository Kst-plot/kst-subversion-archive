/*****************************************************************************
                          showimgpart.h  -  description
                             -------------------
    begin                : th Jul 27 2004
    copyright            : (C) 2004-2006 by Jonathan Riddell, Richard Groult
    email                : jr@jriddell.org, rgroult@jalix.org
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

#ifndef SHOWIMGPART_H
#define SHOWIMGPART_H

// KDE
#include <kparts/part.h>

// Forward declarations
class KAboutData;
class KAction;
class ShowImgPart;

class ImageListViewSimple;
class ImageViewer;

/**
 * The browser extension is an attribute of ShowImgPart and provides
 * some services to Konqueror.  All Konqueror KParts have one.
 */
class ShowImgPartBrowserExtension: public KParts::BrowserExtension {
	Q_OBJECT

public:
	ShowImgPartBrowserExtension(ShowImgPart* viewPart, const char* name=0L);
	~ShowImgPartBrowserExtension();

//protected slots:
public slots:
	void contextMenu();
	void print();
private:
	ShowImgPart* mShowImgPart;
};

/**
 * A Read Only KPart to view images using ShowImg
 */
class ShowImgPart : public KParts::ReadOnlyPart {
	Q_OBJECT
public:
	ShowImgPart(QWidget*, const char*, QObject*, const char*, const QStringList &);
	virtual ~ShowImgPart();

	/**
	 * Return information about the part
	 */
	static KAboutData* createAboutData();

	/**
	 * Print the image being viewed
	 */
	void print();



public slots:
	virtual bool openURL(const KURL& url);
	virtual void setMessage(const QString& msg);
	void loaded(const KURL& url);

protected slots:
    virtual bool openFile();
	/**
	 * Sets Konqueror's caption with setWindowCaption()
	 * called by loaded() signal
	 */
	void setKonquerorWindowCaption(const KURL& url, const QString& filename);


protected:
	void partActivateEvent(KParts::PartActivateEvent* event);

	/**
	 * The component's widget
	 */
	ImageViewer* mImageViewer;

	ImageListViewSimple *m_imageListSimple;

	/**
	 * This inherits from KParts::BrowserExtention and supplies
	 * some extra functionality to Konqueror.
	 */
	ShowImgPartBrowserExtension* mBrowserExtension;
};

#endif
