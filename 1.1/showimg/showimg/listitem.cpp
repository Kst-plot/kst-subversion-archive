/***************************************************************************
                          listitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001 by Richard Groult
                           (C) 2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           js@iidea.pl
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

#include "listitem.h"

// Local
#include "directoryview.h"
#include "imageviewer.h"
#include "imagelistview.h"
#include "mainwindow.h"

// KDE
#include <klistview.h>
#include <kapplication.h>
#include <klocale.h>

// Qt
#include <qstring.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qcheckbox.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

/* TreeHelper class: */
/***************************************************************************
 *   Copyright (C) 2003 by Michael Christen                                *
 *   starcube@my-mail.ch                                                   *
 ***************************************************************************/
class TreeHelper {

protected:
    TreeHelper() {
    }

    ~TreeHelper() {
    }

public:
	static void drawCheckBox(QPainter* p, const QColorGroup& cg, QRect rect, bool checked, bool enabled) {

    // we forc the checkbox to  amaximu size of TREE_CHECKBOX_MAXSIZE
    if (rect.height() > ListItem::TREE_CHECKBOX_MAXSIZE) {
        rect.setTop(2 + (rect.height() - ListItem::TREE_CHECKBOX_MAXSIZE) / 2);
        rect.setLeft(rect.left() + (rect.width() - ListItem::TREE_CHECKBOX_MAXSIZE) / 2);
        rect.setHeight(ListItem::TREE_CHECKBOX_MAXSIZE);
        rect.setWidth(ListItem::TREE_CHECKBOX_MAXSIZE);
    }

    static QCheckBox checkBox(0);
    checkBox.setChecked(checked);
    checkBox.setEnabled(enabled);

    QStyle& style = KApplication::kApplication()->style();

    // copied from qcheckbox.cpp
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( checkBox.isEnabled() )
        flags |= QStyle::Style_Enabled;
    if ( checkBox.hasFocus() )
        flags |= QStyle::Style_HasFocus;
    if ( checkBox.isDown() )
        flags |= QStyle::Style_Down;
    if ( checkBox.hasMouse() )
        flags |= QStyle::Style_MouseOver;
    if ( checkBox.state() == QButton::On )
        flags |= QStyle::Style_On;
    else if ( checkBox.state() == QButton::Off )
        flags |= QStyle::Style_Off;
    else if ( checkBox.state() == QButton::NoChange )
        flags |= QStyle::Style_NoChange;

    // draw the checkbox
    style.drawControl(QStyle::CE_CheckBox, p, &checkBox, rect, cg, flags);
}


	static void drawCheckBox(QPainter* p, const QColorGroup& cg, QRect rect, int tristate, bool enabled) {

    // we force the checkbox to  amaximu size of TREE_CHECKBOX_MAXSIZE
    if (rect.height() > ListItem::TREE_CHECKBOX_MAXSIZE) {
        rect.setTop(2 + (rect.height() - ListItem::TREE_CHECKBOX_MAXSIZE) / 2);
        rect.setLeft(rect.left() + (rect.width() - ListItem::TREE_CHECKBOX_MAXSIZE) / 2);
        rect.setHeight(ListItem::TREE_CHECKBOX_MAXSIZE);
        rect.setWidth(ListItem::TREE_CHECKBOX_MAXSIZE);
    }

    static QCheckBox checkBox(0);
    if (tristate == 0) {
        checkBox.setTristate(true);
        checkBox.setNoChange();
    } else {
        checkBox.setChecked(tristate > 0);
    }
    checkBox.setEnabled(enabled);

    QStyle& style = KApplication::kApplication()->style();

    // copied from qcheckbox.cpp
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( checkBox.isEnabled() )
        flags |= QStyle::Style_Enabled;
    if ( checkBox.hasFocus() )
        flags |= QStyle::Style_HasFocus;
    if ( checkBox.isDown() )
        flags |= QStyle::Style_Down;
    if ( checkBox.hasMouse() )
        flags |= QStyle::Style_MouseOver;
    if ( checkBox.state() == QButton::On )
        flags |= QStyle::Style_On;
    else if ( checkBox.state() == QButton::Off )
        flags |= QStyle::Style_Off;
    else if ( checkBox.state() == QButton::NoChange )
        flags |= QStyle::Style_NoChange;

    // draw the checkbox
    style.drawControl(QStyle::CE_CheckBox, p, &checkBox, rect, cg, flags);
}
};














ListItem::ListItem(MainWindow *mw, ListItemView *dirView, const QString& path)
	:KListViewItem(dirView),
	f(path.isEmpty() ? QString::fromLatin1("/") : path)
#ifdef Q_WS_WIN
	,thisIsADriveNode(false)
#endif
{
	this->mw  = mw;
	this->dirView=dirView;
	init();
}

ListItem::ListItem(ListItem *parent, const QString& filename, MainWindow *mw)
	:KListViewItem(parent),
	f(filename)
#ifdef Q_WS_WIN
	,thisIsADriveNode(false)
#endif
{
	this->mw  = mw;
	this->dirView=parent->getListItemView();
	init();
}

ListItem::~ListItem()
{
}

void
ListItem::init()
{
	setDropEnabled(false);
	setReadOnly(TRUE);
	size="";
	hasSpecialIcon = false;
}


QString
ListItem::key (int column, bool ascending) const
{
	if(column==DirectoryView::COLUMN_TYPE)
		return  QString::fromLatin1("YY")+text(1);
	else {
#ifdef Q_WS_WIN
		if (thisIsADriveNode)
			return f.name().lower().left(1) + KListViewItem::key(column, ascending).lower();
		return QString::fromLatin1("0") + KListViewItem::key(column, ascending).lower();
#endif
		return KListViewItem::key(column, ascending).lower();
	}
}

int
ListItem::compare (QListViewItem * i, int col, bool ascending ) const
{
	if(col==DirectoryView::COLUMN_SIZE)
		return text(2).toInt() - i->text(2).toInt();
	else
	if(col==DirectoryView::COLUMN_NAME)
	{
		int r = KListViewItem::compare(i,col,ascending);
		QRegExp reg("^(\\D*)(\\d+)(\\D*)$",false);
		QString b,e;

		reg.search(text(0));
		QStringList list = reg.capturedTexts();
		reg.search(i->text(0));
		QStringList listi = reg.capturedTexts();

		bool ok, oki;
		unsigned int num, numi;

		num = list[1].toUInt(&ok);
		numi = listi[1].toUInt(&oki);
		if(ok && oki)
		{
			if(num == numi)
				return list[1].compare(listi[1]);
			else
				return  num - numi;
		}
		if(list[1] == listi[1])
		{
			num = list[2].toUInt(&ok);
			numi = listi[2].toUInt(&oki);
			if(ok && oki)
			{
				return  num - numi;
			}
			else
				return r;
		}
		return r;
	}
	return KListViewItem::compare(i,col,ascending);
}

QString
ListItem::fullName()
{
	return full;
}

void
ListItem::setSelected (bool select)
{
	MYDEBUG<<fullName()<<" "<<select<<endl;

	KApplication::setOverrideCursor( waitCursor ); // this might take time
	KListViewItem::setSelected(select);
	mw->updateSelections(this);

	if(!getListItemView()->isDropping())
	{
		repaint();kapp->processEvents();
		if(select)
		{
			mw->changeDirectory(fullName());
			mw->setMessage(i18n("Loading %1...").arg(text(0)));
			kapp->processEvents();
			load(true);
			getListItemView()->setCurrentItem(this);
			getListItemView()->startWatchDir(fullName());
		}
		else
		{
// 			getListItemView()->stopWatchDir(fullName());
			unLoad();
		}
	}
	KApplication::restoreOverrideCursor();	// restore original cursor
}

void
ListItem::updateChildren()
{

	full = parent()->fullName()+f.name()+"/";
        ListItem *myChild = firstChild();
        while( myChild )
	{
            myChild->updateChildren();
            myChild = myChild->nextSibling();
        }
}

QString
ListItem::text (int column) const
{
	if (column == 0)
	{
		return f.name();
	}
	else
	if (column == 1)
		return (extension);
	else
		return (size);
}

QString
ListItem::name ()
{
	return f.name();
}

QString
ListItem::path ()
{
	return QFileInfo(fullName()).dir().absPath();
}

uint
ListItem::getSize()
{
	return size.toUInt();
}

ListItem*
ListItem::firstChild()
{
	return (ListItem*)KListViewItem::firstChild();
}

ListItem*
ListItem::itemBelow()
{
	return (ListItem*)KListViewItem::itemBelow();
}

ListItem*
ListItem::itemAbove()
{
	return (ListItem*)KListViewItem::itemAbove();
}

ListItem*
ListItem::nextSibling()
{
	return (ListItem*)KListViewItem::nextSibling();
}

ListItem*
ListItem::parent()
{
	return (ListItem*)KListViewItem::parent();
}

bool
ListItem::refresh(bool )
{
	unLoad();
	load();

	return true;
}


KURL
ListItem::getURL()
{
	KURL murl;
	murl.setProtocol("file");
	murl.setHost("localhost");
	murl.setPath(fullName());
	return murl;
}


QPtrList < FileIconItem >
ListItem::getFileIconItemList()
{
	return list;
}


void
ListItem::load (bool )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::load (bool )" << endl;
}

void
ListItem::unLoad()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::unLoad()" <<fullName()<<endl;
}

void
ListItem::create(const QString& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::create(QString )" << endl;
}

bool
ListItem::rename(const QString&, QString&)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::rename(QString )" << endl;
	return false;
}

void
ListItem::rename()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::rename()" << endl;
}

void
ListItem::properties()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::properties()" << endl;
}

void
ListItem::goTo (const QString&)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::goTo (QString)" << endl;
}

ListItem*
ListItem::find (const QString&)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO ListItem::find (QString)" <<fullName()<<endl;
	return NULL;
}

void
ListItem::removeImage ( FileIconItem* )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO void ListItem::removeImage ( FileIconItem* )" << endl;
}

bool
ListItem::add(const QStringList& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO bool ListItem::add(QStringList )" << endl;
	return false;
}

bool
ListItem::acceptDrop()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO bool ListItem::acceptDrop()" << endl;
	return false;
}

void
ListItem::setReadOnly(bool readOnly)
{
	this->isReadOnly_=readOnly;
}
bool
ListItem::isReadOnly()
{
	return isReadOnly_;

}

ListItemView*
ListItem::getListItemView()
{
	return dirView;
}


/* this paintCell method */
/***************************************************************************
 *   Copyright (C) 2003 by Michael Christen                                *
 *   starcube@my-mail.ch                                                   *
 ***************************************************************************/
void ListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment) {

    QColorGroup myCg(cg);
/*
    if  (text(2).isEmpty())
    {
        myCg.setColor(QColorGroup::Text, cg.mid());
    }
*/
    switch (column)
    {
    case DirectoryView::COLUMN_NAME :
        KListViewItem::paintCell(p, myCg, column, width, alignment);
        break;


    case DirectoryView::COLUMN_TYPE :
    {
        QFont oldFont = p->font();
        /*
	if  (!text(2).isNull()) {
            QFont font = QFont(oldFont);
            font.setBold(true);
            p->setFont(font);
        }
	*/
        KListViewItem::paintCell(p, myCg, column, width, alignment);
        //p->setFont(oldFont);
        break;
    }


    case DirectoryView::COLUMN_SIZE :
        KListViewItem::paintCell(p, myCg, column, width, alignment);
        break;


    case DirectoryView::COLUMN_SELECT :
        // paint the cell with the alternating background color
#if KDE_VERSION > 330
        p->fillRect(0, 0, width, height(), backgroundColor(-1));
#else
        p->fillRect(0, 0, width, height(), backgroundColor());
#endif
        // draw the checkbox in the center of the cell
        QRect rect((width-height()+4)/2, 2, height()-4, height()-4);

        TreeHelper::drawCheckBox(p, myCg, rect, isSelected()&&!getListItemView()->isDropping(), true);

        break;
    }
}
