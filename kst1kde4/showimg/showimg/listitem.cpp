/***************************************************************************
                          listitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
                           (C) 2004 by Jaroslaw Staniek
    email                : rgroult@jalix.org
                           js@iidea.pl
 ***************************************************************************/

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

#include "listitem.h"

//-----------------------------------------------------------------------------
#define LISTITEM_DEBUG 0
//-----------------------------------------------------------------------------

// Local
#include "directoryview.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"

// KDE
#include <kapplication.h>
#include <klistview.h>
#include <klocale.h>

// Qt
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qpainter.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstyle.h>


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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ListItem::ListItem(ListItemView *a_p_dirView, const KURL& a_url /*= KURL()*/)
	:KListViewItem(a_p_dirView),

	m_p_dirView(a_p_dirView)

#ifdef Q_WS_WIN
	,thisIsADriveNode(false)
#endif
{
	if(a_url.isEmpty())
	{
		getURL().setPath(QString::fromLatin1("/") );
	}
	else
		setURL(a_url);

/*	MYDEBUG
		<< " a_path="<<a_path
		<< " getURL()="<<getURL().prettyURL()
		<< " name()="<<name()
		<< " fullName()="<<fullName()
		<<endl;*/

	init();
}

ListItem::ListItem(ListItem *a_p_parent, const QString& a_filename)
	:KListViewItem(a_p_parent),

	m_p_dirView(a_p_parent->getListItemView())
#ifdef Q_WS_WIN
	,thisIsADriveNode(false)
#endif
{
	setURL(a_p_parent->getURL());
	getURL().addPath( a_filename );

#if LISTITEM_DEBUG > 0
	MYDEBUG
		<< " a_filename="<<a_filename
		<< " getURL()="<<getURL().url()
		<< " name()="<<name()
		<< " fullName()="<<fullName()
		<<endl;
#endif

	init();
}

ListItem::~ListItem()
{
	//MYDEBUG<<"destroy"<<endl;
}

void
ListItem::init()
{
	setDropEnabled(false);
	setReadOnly(true);
	setSize(-1);

	hasSpecialIcon = false;
}

QString
ListItem::getProtocol() const
{
	return getURL().protocol();
}

void
ListItem::setProtocol(const QString& a_protocol)
{
	getURL().setProtocol(a_protocol);
}


QString
ListItem::key (int column, bool ascending) const
{
	if(column==DirectoryView::COLUMN_TYPE)
		return  QString::fromLatin1("YY")+text(1);
	else {
#ifdef Q_WS_WIN
		if (thisIsADriveNode)
			return name().lower().left(1) + KListViewItem::key(column, ascending).lower();
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


void
ListItem::setSelected (bool select)
{
#if LISTITEM_DEBUG > 0
	MYDEBUG<<"BEGIN (bool select="<<select<< " - "<< fullName()<<endl;
#endif
	KListViewItem::setSelected(select);

	if(!getListItemView()->isDropping())
	{
#if LISTITEM_DEBUG > 1
		MYDEBUG<<"!getListItemView()->isDropping()"<<endl;
#endif
		KApplication::setOverrideCursor( waitCursor ); // this might take time
		getMainWindow()->updateSelections(this);
//		repaint();
// 		kapp->processEvents();
		if(select)
		{
			getMainWindow()->changeURL(getURL());
// 			kapp->processEvents();
			load(true);
			getListItemView()->setCurrentItem(this);
		}
		else
		{
			unLoad();
		}
		KApplication::restoreOverrideCursor();	// restore original cursor
	}
#if LISTITEM_DEBUG > 1
	else
		MYDEBUG<<"getListItemView()->isDropping()"<<endl;
#endif

#if LISTITEM_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void
ListItem::updateChildren()
{
	setFullName(parent()->fullName()+name()+'/');
    ListItem *l_p_myChild = firstChild();
	while( l_p_myChild )
	{
		l_p_myChild->updateChildren();
		l_p_myChild = l_p_myChild->nextSibling();
	}
}

QString ListItem::displayedName () const
{
	return name();
}

QString
ListItem::text (int column) const
{
	if (column == 0)
	{
		return displayedName();
	}
	else
	if (column == 1)
		return m_extension;
	else
	{
		if(getSize() > -1)
			return  KGlobal::locale()->formatLong(getSize()); //QString::number(m_size);
		else
			return QString();
	}
}

QString
ListItem::name() const
{
	if(!getURL().fileName().isEmpty())
		return getURL().fileName();
	else
		return QString("/");
}

QString
ListItem::path() const
{
	return getURL().directory();
}

QString ListItem::fullName (int a_trailing/*= 0 */) const
{
/*   * @param _trailing May be ( @c -1, @c 0, @c +1 ). @c -1 strips a trailing
   *                  @c '/', @c +1 adds a trailing @c '/' if there is none yet
   *                  and @c 0 returns the path unchanged*/
	return getURL().path(a_trailing);
}

void ListItem::setFullName (const QString& a_fullname, const QString& a_protocol/*="file"*/)
{
#if LISTITEM_DEBUG > 0
	MYDEBUG<<"avant a_fullname="<<a_fullname<<" => m_url="<<getURL().url()<<" protocol="<<getProtocol()<<endl;
#endif

// 	getURL().setProtocol(a_protocol);
	getURL().setPath(a_fullname);
	//m_url = KURL::fromPathOrURL(a_fullname);

#if LISTITEM_DEBUG > 0
	MYDEBUG<<"a_fullname="<<a_fullname<<" => m_url="<<getURL().url()<<endl;
#endif

};

int
ListItem::getSize() const
{
	return m_size;
}
void
ListItem::setSize(int a_size)
{
	m_size = a_size;
}

uint
ListItem::getNumberOfItems() const
{
	return getSize();
}


ListItem*
ListItem::firstChild()
{
	return dynamic_cast<ListItem*>(KListViewItem::firstChild());
}

ListItem*
ListItem::itemBelow()
{
	return dynamic_cast<ListItem*>(KListViewItem::itemBelow());
}

ListItem*
ListItem::itemAbove()
{
	return dynamic_cast<ListItem*>(KListViewItem::itemAbove());
}

ListItem*
ListItem::nextSibling()
{
	return dynamic_cast<ListItem*>(KListViewItem::nextSibling());
}

ListItem*
ListItem::parent()
{
	return dynamic_cast<ListItem*>(KListViewItem::parent());
}

bool
ListItem::refresh(bool )
{
#if LISTITEM_DEBUG > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	unLoad();
	load();

#if LISTITEM_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
	return true;
}


KURL &
ListItem::getURL()
{
	return m_url;
}
KURL
ListItem::getURL() const
{
	return m_url;
}

void ListItem::setURL(const KURL& a_url )
{
	getURL() = a_url;
}

void
ListItem::load (bool )
{
#if LISTITEM_DEBUG > 0
	MYDEBUG<<"BEGIN"<<endl;
#endif
	if(!getMainWindow()->getImageListView()->hasImages())
	{
#if LISTITEM_DEBUG > 1
		MYDEBUG<<"!getMainWindow()->getImageListView()->hasImages()"<<endl;
#endif
		getMainWindow()->getImageListView()->load(QString::null);
		getMainWindow()->getImageListView()->setContentsPos(0,0);
	}
#if LISTITEM_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
}

void
ListItem::unLoad()
{
	MYWARNING << " TODO void ListItem::unLoad()" <<fullName()<<endl;
}

void
ListItem::create(const QString& )
{
	MYWARNING << " TODO void ListItem::create(QString )" << endl;
}

bool
ListItem::rename(const QString&, QString&)
{
	MYWARNING << " TODO void ListItem::rename(QString )" << endl;
	return false;
}

void
ListItem::rename()
{
	MYWARNING << " TODO void ListItem::rename()" << endl;
}

void
ListItem::properties()
{
	MYWARNING << " TODO void ListItem::properties()" << endl;
}

void
ListItem::goTo (const QString&)
{
	MYWARNING << " TODO void ListItem::goTo (QString)" << endl;
}

ListItem*
ListItem::find (const KURL& a_url)
{
	MYWARNING << " TODO ListItem::find (const KURL& a_url)" <<a_url.url( )<<endl;
	return NULL;
}

ListItem*
ListItem::find (const QString& a_end_dir)
{
	MYWARNING << " TODO ListItem::find (const QString& a_end_dir)" <<a_end_dir<<endl;
	return NULL;
}
void
ListItem::removeIconItem ( FileIconItem* )
{
	MYWARNING << " TODO void ListItem::removeImage ( FileIconItem* )" << endl;
}

bool
ListItem::add(const QStringList& )
{
	MYWARNING << " TODO bool ListItem::add(QStringList )" << endl;
	return false;
}

bool
ListItem::acceptDrop()
{
	MYWARNING << " TODO bool ListItem::acceptDrop()" << endl;
	return false;
}

void
ListItem::setReadOnly(bool readOnly)
{
	m_isReadOnly=readOnly;
}
bool
ListItem::isReadOnly()
{
	return m_isReadOnly;

}


MainWindow* ListItem::getMainWindow()
{
	return &MAINWINDOW;
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
#if KDE_IS_VERSION(3,4,0)
        p->fillRect(0, 0, width, height(), backgroundColor(-1));
#else
#warning your KDE_VERSION is < 3.4.0
        p->fillRect(0, 0, width, height(), backgroundColor());
#endif
        // draw the checkbox in the center of the cell
        QRect rect((width-height()+4)/2, 2, height()-4, height()-4);

        TreeHelper::drawCheckBox(p, myCg, rect, isSelected()&&!getListItemView()->isDropping(), true);

        break;
    }
}


