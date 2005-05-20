/***************************************************************************
                          displayCompare.h  -  description
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

#ifndef DISPLAYCOMPARE_H
#define DISPLAYCOMPARE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt
#include <qptrvector.h>
#include <qdict.h>
#include <qfile.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qheader.h>

// KDE
#include <kapplication.h>
#include <klocale.h>
#include <kdialog.h>


class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QGroupBox;
class QLabel;
class QListView;
class QListViewItem;
class QPushButton;
class QPrtList;
class QPrtVector;
class QFile;

class KSqueezedTextLabel;

class  DisplayCompare : public KDialog
{
	Q_OBJECT
public:
	DisplayCompare(QWidget* parent, QDict < QPtrVector < QFile > >* cmp);
	~DisplayCompare();

public slots :
	void slotDisplayRight(QListViewItem *);
	void slotDisplayLeft(QListViewItem *);
	void accept();
	void reject();
	void suppression();

private:
    QDict < QPtrVector < QFile > >* cmp;

    QGroupBox* GroupBox2;
    KSqueezedTextLabel* OriginalNameLabel;
    KSqueezedTextLabel* originalInfoLabel;
    QLabel* preview1;
    QGroupBox* GroupBox2_2;
    KSqueezedTextLabel* similarNameLabel;
    KSqueezedTextLabel* similarInfoLabel;
    QLabel* preview2;
    QGroupBox* GroupBox8;
    QListView* listName;
    QListView* listEq;
    QPushButton* delettePushButton;
    QPushButton* closePushButton;

protected:
    QVBoxLayout* CompareLayout;
    QHBoxLayout* layout3;
    QVBoxLayout* GroupBox2Layout;
    QHBoxLayout* layout3_2;
    QVBoxLayout* GroupBox2_2Layout;
    QHBoxLayout* layout4;
    QHBoxLayout* GroupBox8Layout;
    QHBoxLayout* layout2;

protected slots:
    virtual void languageChange();
};

#endif

