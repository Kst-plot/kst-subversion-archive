/***************************************************************************
                          fileiconitem.cpp  -  description
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

#include "fileiconitem.h"

// Local
#include "mainwindow.h"
#include "imageviewer.h"
#include "imagelistview.h"
#include "directory.h"
#include "categorydbmanager.h"

// KDE
#include <kurl.h>
#include <kiconloader.h>
#include <kfileitem.h>
#include <kfileiconview.h>
#include <kapplication.h>
#include <kwordwrap.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmdcodec.h>

// Qt
#include <qiconview.h>
#include <qstring.h>
#include <qfont.h>
#include <qpainter.h>
#include <qregexp.h>

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

FileIconItem::FileIconItem(
			ListItem *parentDir,
			const QString& path,
			const QString& filename,
			MainWindow* mw)
	: KFileIconViewItem (mw->getImageListView(),
			filename,
			NULL,
			mKFileItem = new KFileItem(KFileItem::Unknown,
					KFileItem::Unknown,
					KURL::fromPathOrURL(path+filename),
					false)),
	m_size(-1),
	f(path+filename)
{
	this->parentDir=parentDir;
	this->mw=mw;

	haspreview=false;
	__isimage__=false;
	__ismovable__=false;
	__hasToolTip__=true;

	dimension_=QSize(0,0);
	calcRect();
}

FileIconItem::~FileIconItem()
{
	if(mw->getImageListView()->curIt)
	if(this->fullName()==mw->getImageListView()->curIt->fullName())
		mw->getImageListView()->curIt=NULL;
}

void
FileIconItem::setName(const QString& n)
{
	myName_=n;
}

void
FileIconItem::setType(const QString& type)
{
	this->type_ = type;
}

QString
FileIconItem::getType() const
{
	return type_;
}

void
FileIconItem::setPixmap ( const QPixmap& icon, bool haspreview )
{
	KFileIconViewItem::setPixmap(icon);
	this->haspreview=haspreview;
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
	this->haspreview=preview;
}

bool
FileIconItem::hasPreview ()
{
	if(!haspreview)
		return false;
	else
	{
		return QFileInfo(fullName()).lastModified()
			 <
			 QFileInfo
			 	(
			 	QDir::homeDirPath()+"/.thumbnails/normal/" +
				(QString)QFile::encodeName(
					KMD5(QFile::encodeName("file://"+QDir::cleanDirPath(fullName()))).hexDigest()
					)+".png"
				)
			 .lastModified();
	}

}

FileIconItem*
FileIconItem::nextItem ()
{
	return (FileIconItem*)KFileIconViewItem::nextItem();
}

FileIconItem*
FileIconItem::prevItem ()
{
	return (FileIconItem*)KFileIconViewItem::prevItem();
}


QString
FileIconItem::name() const
{
	return myName_;
}


KURL
FileIconItem::getURL()
{
	KURL murl;
	murl.setProtocol("file");
	murl.setHost("localhost");
	murl.setPath(fullName());
	return murl;
}


QString
FileIconItem::getFileName(QString *fullName)
{
	int debut = fullName->findRev ("/");
	int fin = fullName->findRev (".");
	return fullName->mid(debut+1, fin-debut-1);
}


QString
FileIconItem::getFileExt(QString *fullName)
{
	return  fullName->right (fullName->length () - fullName->findRev (".") - 1);
}

QString
FileIconItem::getFullName(QString *fullName)
{
	return fullName->right (fullName->length () - fullName->findRev ("/") - 1);
}

QString
FileIconItem::getFullPath(QString *fullName)
{
	return fullName->left(fullName->findRev ("/") + 1);
}

QString
FileIconItem::mimetype() const
{
	return fileInfo()->mimetype();
}

void
FileIconItem::setKey ( const QString& k )
{
	if(k=="name")
		KIconViewItem::setKey(text());
	else
	if(k=="size")
	{
		KIconViewItem::setKey((QString::number(m_size).leftJustify(20, ' ')));
	}
	else
	if(k=="type")
		KIconViewItem::setKey(mimetype().leftJustify( 20, '0' )+" "+text());
	else
	if(k=="date")
		KIconViewItem::setKey( (QString::number(m_date.toTime_t())).leftJustify( 20, '0' ));
	else
	if(k=="dirname")
		KIconViewItem::setKey(fullName());
	else
		KIconViewItem::setKey(text());
}

int
FileIconItem::compare ( QIconViewItem * i ) const
{
	int r = KIconViewItem::compare(i);

	if( ((FileIconItem*)i)->getType()==getType())
	{
		QString key_, keyi_;
		if(key().startsWith("/"))
		{
			QRegExp reg("^(.*)/(.*)$",false);
			reg.search(key());
			QStringList list = reg.capturedTexts();
			reg.search(i->key());
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
			keyi_=i->key();
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
	return key().compare( "00000000000000000000"+i->key() );
}

void
FileIconItem::setSelected (bool s)
{
	KIconViewItem::setSelected (s);
	kapp->processEvents();
	if ( s )
	{
		//KApplication::setOverrideCursor (waitCursor);
		mw->getImageListView()->load(this);
		//kapp->processEvents();
		FileIconItem *nextIt = nextItem();
		if(nextIt)
			if (mw->getImageListView()->preloadIm() && nextIt->isImage())
			{
				mw->getImageViewer()-> preloadImage(nextIt->fullName());
			}
		//KApplication::restoreOverrideCursor ();
	}
}

bool
FileIconItem::isImage() const
{
	return __isimage__;
}

bool
FileIconItem::isMovable() const
{
	return __ismovable__;
}

QString
FileIconItem::fullName () const
{
	return full;
}


void
FileIconItem::setPath(const QString& newPath)
{
	QString name = QFileInfo(fullName()).fileName();
	full = newPath+name;

	KURL murl;
	murl.setPath(fullName());
	murl.setProtocol("file");
	mKFileItem->setURL(murl);
}

QString
FileIconItem::path()
{
	return QFileInfo(fullName()).dir().absPath();
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
	args << i18n("Name:") << QFileInfo(f).fileName ()
			<< i18n("Location:") << shrink(QDir::convertSeparators(QFileInfo(f).dirPath(true)));

	ImageFileInfo iminfo(full, IMAGE, true);
	if (iminfo.hasInfo())
		args << i18n("Description:") << iminfo.getTitle();
	return args;
}

QString
FileIconItem::toolTipStr() const
{
	if (!__hasToolTip__)
		return QString::null;

	QString tip;
	QColorGroup cg = mw->getImageListView()->colorGroup () ;

	tip = QString("<table cellspacing=\"0\" cellpadding=\"0\" width=\"250\">");

	//
	QStringList args( toolTipArgs() );
	for (QStringList::ConstIterator it = args.constBegin(); it!=args.constEnd(); ++it) 
	{
		tip += QString("<tr><td>")
		+*it+"</font></td><td>";
		++it;
		tip += *it+"</font></td></tr>";
	}

	//
	tip +=QString("<tr bgcolor=\"%1\"><td COLSPAN=\"2\"><font color=\"%2\"><b>File:</b></font></td></tr>")
			.arg(cg.highlight().name())
			.arg(cg.highlightedText().name());
	tip += QString("<tr><td>");
	tip += i18n("Type");
	tip+= "</font></td><td>";
	tip+= fileInfo()->mimeComment();
	tip += "</font></td></tr>";
	
	tip += QString("<tr><td>");
	tip += i18n("Size");
	tip+= "</font></td><td>";
	tip+= KGlobal::locale()->formatNumber(fileInfo()->size(), 0);
	tip += "</font></td></tr>";
	
	tip += QString("<tr><td>");
	tip += i18n("Modification");
	tip+= "</font></td><td>";
	tip+= fileInfo()->timeString();
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
		tip +=QString("<tr bgcolor=\"%1\"><td COLSPAN=\"2\"><font color=\"%2\"><b>Metadata:</b></font></td></tr>")
				.arg(cg.highlight().name())
				.arg(cg.highlightedText().name());
		tip += meta;
		tip+="</td></tr>";
	}
	
	//
#ifdef WANT_LIBKEXIDB
	QString cats = mw->getCategoryManager()->getCategoryNameListImage(fullName())->join(", ");
	if(!cats.isEmpty())
	{
		tip +=QString("<tr bgcolor=\"%1\"><td COLSPAN=\"2\"><font color=\"%2\"><b>Categories:</b></font></td></tr><tr><td COLSPAN=\"2\">")
				.arg(cg.highlight().name())
				.arg(cg.highlightedText().name());
		tip+=cats;
		tip+="</td></tr>";
	}
#endif	
	//
	tip += "</table>";
	return tip;
}

bool
FileIconItem::suppression(bool)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::suppression(bool) " << fullName() << endl;
	return false;
}

bool
FileIconItem::suppression()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::suppression() " << fullName() << endl;
	return false;
}

bool
FileIconItem::moveToTrash()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::moveToTrash() " << fullName() << endl;
	return false;
}

bool
FileIconItem::shred()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::shred() " << fullName() << endl;
	return false;
}

QString
FileIconItem::text(int ) const
{
	//kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::text(int ) const " << fullName() << endl;
	return KIconViewItem::text();
}

void
FileIconItem::setWallpaper()
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::setWallpaper() " << fullName() << endl;
}

QString
FileIconItem::getFileName(const QString&)
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::getFileName(QString ) "  << endl;
	return QString();
}

QString
FileIconItem::getFileExt(const QString& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::getFileExt(QString ) "  << endl;
	return QString();
}

QString
FileIconItem::getFullName(const QString& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::getFullName(QString ) "  << endl;
	return QString();
}

QString
FileIconItem::getFullPath(const QString& )
{
	kdWarning() << __FILE__ << __LINE__ << " TODO FileIconItem::getFullPath(QString ) "  << endl;
	return QString();
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


    if (mw->getImageListView()->getShowMimeType())
    {
         firstLine = false;
         KMimeType::Ptr mimePtr =
             KMimeType::findByURL(getURL());
         extraText += mimePtr->name();
     }

    if (mw->getImageListView()->getShowSize() && m_size >=0 )
    {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        extraText += KIO::convertSize(m_size);
    }


    if (mw->getImageListView()->getShowDate())
    {
        if (!firstLine)
            extraText += "\n";
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
                extraText += "\n";
            else
                firstLine = false;
            extraText += comments;
        }
    }

    */

    if (mw->getImageListView()->getShowDimension() && dimension_.width() != 0 && dimension_.height()!= 0) {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        extraText += QString::number(dimension_.width())
                     + "x"
                     + QString::number(dimension_.height())+
		     +" " + i18n("Pixels");
    }

    extraText_ = extraText;

}

/**
	modified code from aurelien[dot}gateau[at]free.fr
*/

void
FileIconItem::wrapText()
{
	if(! mw->getImageListView()->wordWrapIconText ())
	{
		wrapedText_=text();
		return;
	}

	static QString dots("...");
	QFontMetrics fm(mw->getImageListView()->font());
// If the text fit in the width, don't truncate it
	int width=mw->getImageListView()->getCurrentIconSize().width()-2;
	if (fm.boundingRect(text()).width()<=width) {
		wrapedText_=text();
		return;
	}

// Find the number of letters to keep
	wrapedText_=text();
	width-=fm.width(dots);
	int len=wrapedText_.length();
	for(;len>0 && fm.width(wrapedText_,len)>width;--len);

// Truncate the text
	wrapedText_.truncate(len);
	wrapedText_+=dots;
}


void
FileIconItem::calcRect()
{
    QRect itemIconRect = QRect(0,0,0,0);
    QRect itemTextRect = QRect(0,0,0,0);
    itemExtraRect_ = QRect(0,0,0,0);
    QRect itemRect = rect();

    itemRect.setWidth(100);
    itemRect.setHeight(0xFFFFFFFF);

    // set initial pixrect
    //int pw = pixmap()->width();
    //int ph = pixmap()->height();
    int pw = mw->getImageListView()->getCurrentIconSize().width();
    int ph = mw->getImageListView()->getCurrentIconSize().height();

    itemIconRect.setWidth(pw);
    itemIconRect.setHeight(ph);

    if(! mw->getImageListView()->wordWrapIconText ()) wrapedText_=text(); else wrapText();
    // word wrap main text
    QFontMetrics fm(mw->getImageListView()->font());
    QRect r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                    0xFFFFFFFF, Qt::AlignHCenter |
                                    Qt::WordBreak | Qt::BreakAnywhere
                                    | Qt::AlignTop,
                                    wrapedText_));
    r.setWidth(r.width());

    itemTextRect.setWidth(r.width());
    itemTextRect.setHeight(r.height());

    // word wrap extra Text
    if (!extraText_.isEmpty()) {

        QFont font(mw->getImageListView()->font());
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
                                  extraText_));
        r.setWidth(r.width() );

        itemExtraRect_.setWidth(r.width());
        itemExtraRect_.setHeight(r.height());

        itemTextRect.setWidth(QMAX(itemTextRect.width(), itemExtraRect_.width()));
        itemTextRect.setHeight(itemTextRect.height() + itemExtraRect_.height());
    }


    // Now start updating the rects
    int w = QMAX(itemTextRect.width(), itemIconRect.width() );
    int h = itemTextRect.height() + itemIconRect.height() ;

    itemRect.setWidth(w);
    itemRect.setHeight(h);

    // Center the pix and text rect
    /*
    itemIconRect = QRect((itemRect.width() - itemIconRect.width())/2,
                         0,
                         itemIconRect.width(), itemIconRect.height());
    */
    itemTextRect = QRect((itemRect.width() - itemTextRect.width())/2,
                         itemRect.height() - itemTextRect.height(),
                         itemTextRect.width(), itemTextRect.height());
    if (!itemExtraRect_.isEmpty()) {
        itemExtraRect_ = QRect((itemRect.width() - itemExtraRect_.width())/2,
                               itemRect.height() - itemExtraRect_.height(),
                               itemExtraRect_.width(), itemExtraRect_.height());
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
    QFont font(mw->getImageListView()->font());

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
                     Qt::AlignHCenter|Qt::AlignTop, wrapedText_);

    if (!extraText_.isEmpty()) {
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
        painter.drawText(itemExtraRect_, Qt::WordBreak|
                         Qt::BreakAnywhere|Qt::AlignHCenter|
                         Qt::AlignTop,extraText_);
    }

    painter.end();

    QRect r(rect());
    r = QRect(mw->getImageListView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(mw->getImageListView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width()+4, r.height()+4);
}

