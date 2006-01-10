/***************************************************************************
                          printImageDialog.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C)2001-2005 - Andrew Richards
    email                : ajr@users.sourceforge.net
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

#ifndef PRINTIMAGEDIALOG_H
#define PRINTIMAGEDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qprintdialog.h>
#include <qprinter.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qpushbutton.h>
#include <qstring.h>

// KDE
#include <kapplication.h>
#if KDE_VERSION > 220
#include <kprinter.h>
#endif
#include <kdialog.h>

// Local
#include "numslider.h"


class printImageDialog : public KDialog
{
	Q_OBJECT

public:
	/**
		construct a dialog to print a pixmap
		@param pixmap the pixmap to print
		@param printer the printer object
	*/
#if KDE_VERSION > 220
	printImageDialog(QWidget *parent, const QPixmap& pixmap, const QString& name, KPrinter *p);
#else
	printImageDialog(QWidget *parent, const QPixmap& pixmap, const QString& name, QPrinter *p);
#endif
	virtual ~printImageDialog();

 protected:
	virtual void paintEvent(QPaintEvent *e);

 private slots:
	void newScale(double sc);
	void printImage();
	void cancel();
	void back();

 private:
	void paintImage(QPainter *p);
	QRect pageDimensions();

#if KDE_VERSION > 220
	KPrinter  *prn;
#else
	QPrinter  *prn;
#endif
	QPixmap   pix;
	QRect     pageSize;
	QString   imageName;
	numSlider *slScale;

	double scale, pageScale;
};

#endif
