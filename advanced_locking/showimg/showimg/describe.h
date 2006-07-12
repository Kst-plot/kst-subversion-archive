/***************************************************************************
                           describe.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#ifndef DESCRIBE_H
#define DESCRIBE_H

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

class Describe : public KDialogBase
{
	Q_OBJECT

public:
	Describe( QWidget* parent, const QString& imafefile, const char* name = 0 );
	~Describe();

	void setImageFile(const QString& imagefile);

	QSize sizeHint () const;

protected slots:
	void slotOk();
	void slotCancel();
	void slotApply();

signals:
	void close();

protected:
	ImageFileInfo *iinfo;

///
    QLabel* textLabel5;
    KLineEdit* title;
    QGroupBox* groupBox2;
    QLabel* textLabel3;
    KLineEdit* event;
    KLineEdit* people;
    QLabel* textLabel4;
    QLabel* textLabel1;
    KLineEdit* location;
    KLineEdit* date;
    QLabel* textLabel2;
    QGroupBox* groupBox3;
    KTextEdit* longDescr;

    QVBoxLayout* DescribeLayout;
    QHBoxLayout* layout1;
    QHBoxLayout* layout2;
    QVBoxLayout* groupBox2Layout;
    QGridLayout* layout3;
    QHBoxLayout* groupBox3Layout;

};

#endif // Describe_H
