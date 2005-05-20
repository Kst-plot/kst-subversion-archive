/***************************************************************************
                          describeAlbum.h -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
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

#ifndef DESCRIBEALBUM_H
#define DESCRIBEALBUM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "imagefileinfo.h"

// Qt
#include <qvariant.h>

// KDE
#include <kdialogbase.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class KLineEdit;
class QGroupBox;
class KTextEdit;

class ImageFileInfo;


class DescribeAlbum : public KDialogBase
{
	Q_OBJECT

public:
	DescribeAlbum( QWidget* parent,  const QString& dirname, const char* name = 0 );
	~DescribeAlbum();

protected slots:
	void slotOk();

protected:
	ImageFileInfo *iinfo;

    QLabel* textLabel1;
    KLineEdit* title;
    QGroupBox* DescribeAlbumgroupBox1;
    QLabel* textLabel2;
    KLineEdit* shortDescr;
    QLabel* textLabel2_2;
    KTextEdit* longDescr;

    QVBoxLayout* DescribeAlbumLayout;
    QHBoxLayout* DescribeAlbumlayout1;
    QVBoxLayout* DescribeAlbumgroupBox1Layout;

};

#endif
