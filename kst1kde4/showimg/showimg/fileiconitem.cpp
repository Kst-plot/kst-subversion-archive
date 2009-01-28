/*****************************************************************************
                          fileiconitem.cpp  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2006 by Richard Groult
    email                : rgroult@jalix.org
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

#include "fileiconitem.h"

//-----------------------------------------------------------------------------
#define FILEICONITEM_DEBUG          0
#define FILEICONITEM_DEBUG_SELECT   0
//-----------------------------------------------------------------------------

// Local
#include "categorydbmanager.h"
#include "directory.h"
#include "imagefileinfo.h"
#include "imagelistview.h"
#include "imageviewer.h"
#include "mainwindow.h"
#include "showimg_common.h"

// System
#include <typeinfo>

// KDE
#include <kapplication.h>
#include <kdebug.h>
#include <kfileiconview.h>
#include <kfileitem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kurl.h>
#include <kwordwrap.h>

// Qt
#include <qfont.h>
#include <qiconview.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstring.h>

FileIconItem::FileIconItem(
			const KURL& a_url,
			const QString& filename)
	: KFileIconViewItem (getMainWindow()->getImageListView(),
			filename,
			NULL,
			new KFileItem(KFileItem::Unknown,
					KFileItem::Unknown,
// 					KURL::fromPathOrURL(path+filename),
 					KURL::fromPathOrURL(a_url.url(1)+filename),
					false)),
	m_size(-1),
	m_haspreview(false),
	m_dimension(QSize(0,0))

{
#if FILEICONITEM_DEBUG > 0
	MYDEBUG
		<< " a_url="<<a_url.url()<< " "
		<< " filename="<<filename<< " "
		<< " fullname()="<<fullname()<< " "
		<< " getURL()="<<getURL().url()<< " "
	<<endl;
#endif

	setIsImage(false);
	setIsMovable(false);
	setHasTooltip(true);

	calcRect();
}

FileIconItem::~FileIconItem()
{
	if(getMainWindow()->getImageListView()->getCurrentItem())
		if(fullName() == getMainWindow()->getImageListView()->getCurrentItem()->fullName())
			getMainWindow()->getImageListView()->setCurrentItem(NULL);
}


void
FileIconItem::setPixmap ( const QPixmap& icon, bool a_haspreview )
{
	m_haspreview = a_haspreview;

	KFileIconViewItem::setPixmap(icon);
	calcRect();
	repaint();
}

void
FileIconItem::repaint()
{
    KFileIconViewItem::repaint();

}

void
FileIconItem::setHasPreview (bool preview)
{
	m_haspreview=preview;
}

bool
FileIconItem::hasPreview ()
{
	if(!m_haspreview)
		return false;
	else
	{
		return QFileInfo(fullName()).lastModified()
			 <
			 QFileInfo
			 	(
			 	QDir::homeDirPath()+"/.thumbnails/normal/" +
				(QString)QFile::encodeName(
					KMD5(QFile::encodeName(getURL().url())).hexDigest()
					)+".png"
				)
			 .lastModified();
	}

}

FileIconItem*
FileIconItem::nextItem ()
{
	return dynamic_cast<FileIconItem*>(KFileIconViewItem::nextItem());
}

FileIconItem*
FileIconItem::prevItem ()
{
	return dynamic_cast<FileIconItem*>(KFileIconViewItem::prevItem());
}


QString
FileIconItem::getProtocol()
{
	return getURL().protocol();
}

void
FileIconItem::setProtocol(const QString& protocol)
{
	getURL().setProtocol(protocol);
}

KURL
FileIconItem::getURL() const
{
	return fileInfo()->url();
}

QString
FileIconItem::mimetype() const
{
	return fileInfo()->mimetype();
}

void
FileIconItem::setKey ( const QString& k )
{
	if(k==QString::fromLatin1("name"))
		KIconViewItem::setKey(text());
	else
	if(k==QString::fromLatin1("size"))
		KIconViewItem::setKey((QString::number(m_size).leftJustify(20, ' ')));
	else
	if(k==QString::fromLatin1("type"))
		KIconViewItem::setKey(mimetype().leftJustify( 20, '0' )+' '+text());
	else
	if(k==QString::fromLatin1("date"))
		KIconViewItem::setKey( (QString::number(m_date.toTime_t())).leftJustify( 20, '0' ));
	else
	if(k==QString::fromLatin1("dirname"))
		KIconViewItem::setKey(fullName());
	else
		KIconViewItem::setKey(text());
}

int
FileIconItem::compare ( QIconViewItem * a_p_item ) const
{
	int r = KIconViewItem::compare(a_p_item);

	if( typeid(*a_p_item) == typeid(*this))
	{
		QString key_, keyi_;
		if(key().startsWith("/"))
		{
			QRegExp reg("^(.*)/(.*)$",false);
			reg.search(key());
			QStringList list = reg.capturedTexts();
			reg.search(a_p_item->key());
			QStringList listi = reg.capturedTexts();

			if(list[1]==listi[1])
			{
				key_=list[2];
				keyi_=listi[2];
			}
			else
			{
				return list[1].compare(listi[1]);
			}
		}
		else
		{
			key_=key();
			keyi_=a_p_item->key();
		}

		QRegExp reg("^(\\D*)(\\d+)(\\D*)$",false);
		QString b,e;

		reg.search(key_);
		QStringList list = reg.capturedTexts();
		reg.search(keyi_);
		QStringList listi = reg.capturedTexts();

		bool ok, oki;
		unsigned int num, numi;

		num = list[1].toUInt(&ok);
		numi = listi[1].toUInt(&oki);
		if(ok && oki)
		{
			if(num == numi)
				r = list[1].compare(listi[1]);
			else
				r = num - numi;
		}
		else
		if(list[1] == listi[1])
		{
			num = list[2].toUInt(&ok);
			numi = listi[2].toUInt(&oki);
			if(ok && oki)
			{
				r = num - numi;
			}
		}
	}

#if FILEICONITEM_DEBUG > 5
	MYDEBUG
		<<"typeid(*this)="<<typeid(*this).name() << " - "
		<<"typeid(*a_p_item)="<<typeid(*a_p_item).name()
		<< "r=" << r
	<<endl;
#endif
	return r;
}

void
FileIconItem::setSelected (bool s)
{
#if FILEICONITEM_DEBUG_SELECT > 0
	MYDEBUG<<"BEGIN"<<fullName()<<endl;
#endif
	KIconViewItem::setSelected (s);
	if ( s )
	{
		getMainWindow()->getImageListView()->load(this);
	}
	if(isSelected()) //selection could be changed!
	{
		FileIconItem *l_p_nextIt = nextItem();
		if(l_p_nextIt)
			if (
				getMainWindow()->getImageListView()->preloadIm() &&
				l_p_nextIt->isImage()
			)
				getMainWindow()->getImageViewer()-> preopenURL(l_p_nextIt->fullName());
	}
#if FILEICONITEM_DEBUG_SELECT > 0
	MYDEBUG<<"END"<<fullName()<<endl;
#endif
}

bool
FileIconItem::isImage() const
{
	return m_isimage;
}
void
FileIconItem::setIsImage(bool a_isImage)
{
	m_isimage = a_isImage;
}

bool
FileIconItem::isMovable() const
{
	return m_ismovable;
}

void
FileIconItem::setIsMovable(bool a_isMovable)
{
	m_ismovable = a_isMovable;
}

QString
FileIconItem::name () const
{
	return QFileInfo(fullName()).fileName();
}

QString
FileIconItem::fullName () const
{
// 	return m_full;
	return fileInfo()->url().path();
}

void
FileIconItem::setFullName (const QString& a_fullpath)
{
// 	m_full = a_fullpath;
	KURL l_url = getURL();
	l_url.setPath(a_fullpath);
	fileInfo()->setURL(l_url);
}


void
FileIconItem::setPath(const QString& a_newPath)
{
	QString l_name = name();
	setFullName(a_newPath+l_name);


	KURL l_url = getURL();
	l_url.setPath(fullName());
	fileInfo()->setURL(l_url);

}

QString
FileIconItem::path() const
{
	//return QFileInfo(fullName()).dir().absPath();
	return getURL().directory();
}

QString
FileIconItem::shrink(const QString& str, int len) const
{
	if(str.length()<=(unsigned int)len)
		return str;
	else
	{
		return str.left(len/2) + "..." + str.right(len/2);
	}
}

QStringList
FileIconItem::toolTipArgs() const
{
	QStringList args;
	args
		<< i18n("Name:")     << name()
		<< i18n("Location:") << shrink(getURL().url( ));

	ImageFileInfo iminfo(fullName(), IMAGE, true);
	if (iminfo.hasInfo())
		args << i18n("Description:") << iminfo.getTitle();
	return args;
}

QString
FileIconItem::toolTipStr() const
{
	if (!getHasTooltip())
		return QString::null;

	QString tip;
	QColorGroup cg = getMainWindow()->getImageListView()->colorGroup () ;

	tip = QString("<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\">");

	//
	QStringList args( toolTipArgs() );
	for ( QValueList<QString>::const_iterator it = args.begin(); it!=args.end(); ++it)
	{
		tip += QString("<tr><td>")
		+*it+"</font></td><td>";
		++it;
		tip += *it+"</font></td></tr>";
	}
	//
	tip +=QString("<tr bgcolor=\"%1\"><td COLSPAN=\"2\"><font color=\"%2\"><b>%3</b></font></td></tr>")
			.arg(cg.highlight().name())
			.arg(cg.highlightedText().name())
			.arg(i18n("File"));
	tip += QString("<tr><td>");
	tip += i18n("Type");
	tip += "</font></td><td>";
	tip += fileInfo()->mimeComment();
	tip += "</font></td></tr>";

	tip += QString("<tr><td>");
	tip += i18n("Size");
	tip += "</font></td><td>";
	tip += KGlobal::locale()->formatNumber(fileInfo()->size(), 0);
	tip += "</font></td></tr>";

	tip += QString("<tr><td>");
	tip += i18n("Modification");
	tip += "</font></td><td>";
	tip += fileInfo()->timeString();
	tip += "</font></td></tr>";

	//
	QString meta;
	int nbItem = 0;
	KFileMetaInfo metaInfo = fileInfo()->metaInfo();
	QStringList groups = metaInfo.groups();
	for ( QStringList::Iterator it = groups.begin(); it != groups.end() && nbItem<6; ++it )
	{
		KFileMetaInfoGroup group = metaInfo.group(*it);
		QStringList supportedKeys = group.supportedKeys();
		for ( QStringList::Iterator keys = supportedKeys.begin(); keys != supportedKeys.end() && nbItem<6; ++keys )
		{
			KFileMetaInfoItem item = group.item(*keys);
			QString key = item.key();
			QString value = item.string();

			if(value!="---" && ! value.isEmpty())
			{
				meta += QString("<tr><td>");
				meta += key;
				meta+= "</font></td><td>";
				meta+= value;
				meta += "</font></td></tr>";

				nbItem++;
			}
		}
	}
	if(!meta.isEmpty())
	{
		tip +=QString("<tr bgcolor=\"%1\"><td COLSPAN=\"2\"><font color=\"%2\"><b>%3</b></font></td></tr>")
				.arg(cg.highlight().name())
				.arg(cg.highlightedText().name())
				.arg(i18n("Metadata"));
		tip += meta;
		tip +="</td></tr>";
	}

	//
#ifdef WANT_LIBKEXIDB
	QStringList *cats =
		(getMainWindow()->getCategoryDBManager()?getMainWindow()->getCategoryDBManager()->getCategoryNameListImage(fullName()):NULL);
	if(cats && !cats->isEmpty())
	{
		tip +=QString("<tr bgcolor=\"%1\"><td COLSPAN=\"2\"><font color=\"%2\"><b>%3</b></font></td></tr><tr><td COLSPAN=\"2\">")
				.arg(cg.highlight().name())
				.arg(cg.highlightedText().name())
				.arg(i18n("Categories"));
		tip += cats->join(", ");
		tip += "</td></tr>";
		delete(cats);
	}
#endif
	//
	tip += "</table>";
	return tip;
}

bool
FileIconItem::suppression(bool)
{
	MYWARNING << " TODO FileIconItem::suppression(bool) " << fullName() << endl;
	return false;
}

bool
FileIconItem::suppression()
{
	MYWARNING << " TODO FileIconItem::suppression() " << fullName() << endl;
	return false;
}

bool
FileIconItem::moveToTrash()
{
	MYWARNING << " TODO FileIconItem::moveToTrash() " << fullName() << endl;
	return false;
}

bool
FileIconItem::shred()
{
	MYWARNING << " TODO FileIconItem::shred() " << fullName() << endl;
	return false;
}

QString
FileIconItem::text(int ) const
{
	return KIconViewItem::text();
}

void
FileIconItem::setWallpaper()
{
	MYWARNING << " TODO FileIconItem::setWallpaper() " << fullName() << endl;
}

/**
	modified code from digikam (http://digikam.sourceforge.net/)
	Renchi Rajurenchi_NO_SPAM_FOR_ME_PLEASE_[AT]pooh.tam.uiuc.edu

*/
void
FileIconItem::updateExtraText()
{
    QString extraText;
    bool firstLine = true;


    if (getMainWindow()->getImageListView()->getShowMimeType())
    {
         firstLine = false;
         KMimeType::Ptr mimePtr =
             KMimeType::findByURL(getURL());
         extraText += mimePtr->name();
     }

    if (getMainWindow()->getImageListView()->getShowSize() && m_size >=0 )
    {
        if (!firstLine)
            extraText += '\n';
        else
            firstLine = false;
        extraText += KIO::convertSize(m_size);
    }


    if (getMainWindow()->getImageListView()->getShowDate() && fileInfo()->isLocalFile() )
    {
        if (!firstLine)
            extraText += '\n';
        else
            firstLine = false;
		extraText+=m_date.toString();
    }



/*
    if (settings->getIconShowComments()) {
        QString comments;
        view_->getItemComments(text(), comments);
        if (!comments.isEmpty()) {
            if (!firstLine)
                extraText += '\n';
            else
                firstLine = false;
            extraText += comments;
        }
    }
*/

    if (getMainWindow()->getImageListView()->getShowDimension() && m_dimension.width() != 0 && m_dimension.height()!= 0) {
        if (!firstLine)
            extraText += '\n';
        else
            firstLine = false;
        extraText += QString::number(m_dimension.width())
                     + 'x'
                     + QString::number(m_dimension.height())+
		     +" " + i18n("Pixels");
    }


	m_extraText_short = extraText;
        if (getMainWindow()->getImageListView()->getShowCategoryInfo() && !m_categories.isEmpty())
	{
            if (!firstLine)
                extraText += '\n';
            else
                firstLine = false;
            extraText += m_categories.join(", ");
        }


    m_extraText = extraText;
	//MYDEBUG<<m_extraText<<endl;

}

/**
	modified code from aurelien[dot}gateau[at]free.fr
*/

void
FileIconItem::wrapText()
{
	if(! getMainWindow()->getImageListView()->wordWrapIconText ())
	{
		m_wrapedText=text();
		return;
	}

	static QString dots("...");
	QFontMetrics fm(getMainWindow()->getImageListView()->font());
// If the text fit in the width, don't truncate it
	int width=getMainWindow()->getImageListView()->getCurrentIconSize().width()-2;
	if (fm.boundingRect(text()).width()<=width) {
		m_wrapedText=text();
		return;
	}

// Find the number of letters to keep
	m_wrapedText=text();
	width-=fm.width(dots);
	int len=m_wrapedText.length();
	for(;len>0 && fm.width(m_wrapedText,len)>width;--len);

// Truncate the text
	m_wrapedText.truncate(len);
	m_wrapedText+=dots;
}


void
FileIconItem::calcRect()
{
    QRect itemIconRect = QRect(0,0,0,0);
    QRect itemTextRect = QRect(0,0,0,0);
    m_itemExtraRect = QRect(0,0,0,0);
    QRect itemRect = rect();

    itemRect.setWidth(100);
    itemRect.setHeight(0xFFFFFFFF);

    // set initial pixrect
    //int pw = pixmap()->width();
    //int ph = pixmap()->height();
    int pw = getMainWindow()->getImageListView()->getCurrentIconSize().width();
    int ph = getMainWindow()->getImageListView()->getCurrentIconSize().height();

    itemIconRect.setWidth(pw);
    itemIconRect.setHeight(ph);

    if(! getMainWindow()->getImageListView()->wordWrapIconText ()) m_wrapedText=text(); else wrapText();
    // word wrap main text
    QFontMetrics fm(getMainWindow()->getImageListView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                    0xFFFFFFFF, Qt::AlignHCenter |
                                    Qt::WordBreak | Qt::BreakAnywhere
                                    | Qt::AlignTop,
                                    m_wrapedText));
    r.setWidth(r.width());

    itemTextRect.setWidth(r.width());
    itemTextRect.setHeight(r.height());

    // word wrap extra Text
    if (!m_extraText.isEmpty()) {

        QFont font(getMainWindow()->getImageListView()->font());
        int fontSize = (font.pointSize()*4)/5;
        if (fontSize > 0) {
            font.setPointSize(fontSize);
        }
        else {
            fontSize = font.pixelSize();
            font.setPixelSize(fontSize);
        }

        fm = QFontMetrics(font);

        r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                  0xFFFFFFFF, Qt::AlignHCenter |
                                  Qt::WordBreak | Qt::BreakAnywhere
                                  | Qt::AlignTop,
                                  m_extraText));
        r.setWidth(r.width() );

        m_itemExtraRect.setWidth(r.width());
        m_itemExtraRect.setHeight(r.height());

        itemTextRect.setWidth(QMAX(itemTextRect.width(), m_itemExtraRect.width()));
        itemTextRect.setHeight(itemTextRect.height() + m_itemExtraRect.height());
    }


    // Now start updating the rects
    int w = QMAX(itemTextRect.width(), itemIconRect.width() );
    int h = itemTextRect.height() + itemIconRect.height() ;

    itemRect.setWidth(w);
    itemRect.setHeight(h);

    // Center the pix and text rect
    itemTextRect = QRect((itemRect.width() - itemTextRect.width())/2,
                         itemRect.height() - itemTextRect.height(),
                         itemTextRect.width(), itemTextRect.height());
    if (!m_itemExtraRect.isEmpty()) {
        m_itemExtraRect = QRect((itemRect.width() - m_itemExtraRect.width())/2,
                               itemRect.height() - m_itemExtraRect.height(),
                               m_itemExtraRect.width(), m_itemExtraRect.height());
    }

    // Update rects
    if ( itemIconRect != pixmapRect() )
        setPixmapRect( itemIconRect );
    if ( itemTextRect != textRect() )
        setTextRect( itemTextRect );
     setItemRect( itemRect );

}

void
FileIconItem::paintItem(QPainter *, const QColorGroup& cg)
{
    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);
    QFont font(getMainWindow()->getImageListView()->font());

    QPixmap pix(rect().width(), rect().height());
    pix.fill(cg.base());
    QPainter painter(&pix);
    painter.drawPixmap(pRect.x()+(rect().width()-pixmap()->width())/2,
    			pRect.y() +(pixmapRect(false).height()-pixmap()->height()),
			*pixmap() );

    if (isSelected()) {
        QPen pen;
        pen.setColor(cg.highlight());
        painter.setPen(pen);
        painter.drawRect(0, 0, pix.width(), pix.height());
        painter.fillRect(0, tRect.y(), pix.width(),
                     tRect.height(), cg.highlight() );
        painter.setPen( QPen( cg.highlightedText() ) );
    }
    else
        painter.setPen( cg.text() );

    painter.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
                     Qt::AlignHCenter|Qt::AlignTop, m_wrapedText);

    if (!m_extraText.isEmpty()) {
        int fontSize = (font.pointSize()*4)/5;
        if (fontSize > 0) {
            font.setPointSize(fontSize);
        }
        else {
            fontSize = font.pixelSize();
            font.setPixelSize(fontSize);
        }
        painter.setFont(font);
        if (!isSelected())
            painter.setPen(QPen("steelblue"));

        //font.setItalic(true);
        painter.setFont(font);
        painter.drawText(m_itemExtraRect, Qt::WordBreak|
                         Qt::BreakAnywhere|Qt::AlignHCenter|
                         Qt::AlignTop, m_extraText_short);

	////////////////////////////////////
	QString cats=m_categories.join(", ");
        QFontMetrics fm = QFontMetrics(font);
        QRect r = QRect(fm.boundingRect(0, 0, textRect().width(),0xFFFFFFFF,
				Qt::AlignHCenter | Qt::WordBreak | Qt::BreakAnywhere | Qt::AlignTop,
                                m_extraText_short));
	QColor mycolor("gainsboro");
        if (!isSelected())
            mycolor=mycolor.dark();
	else
	    mycolor=cg.highlight().dark();
	painter.setPen(QPen(mycolor));

	QRect endssss(m_itemExtraRect.left(), m_itemExtraRect.top()+r.height(), m_itemExtraRect.width(), m_itemExtraRect.height()-r.height());
	//MYDEBUG<<fullName()<<m_itemExtraRect<<r<<endssss<<endl;
        painter.drawText(endssss, Qt::WordBreak|
                         Qt::BreakAnywhere|Qt::AlignHCenter|
                         Qt::AlignTop,  cats);
    }

    painter.end();

    QRect r(rect());
    r = QRect(getMainWindow()->getImageListView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(getMainWindow()->getImageListView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width()+4, r.height()+4);
}

MainWindow* FileIconItem::getMainWindow()
{
	return &MAINWINDOW;
}
const MainWindow* FileIconItem::getMainWindow() const
{
	return &MAINWINDOW;
}

