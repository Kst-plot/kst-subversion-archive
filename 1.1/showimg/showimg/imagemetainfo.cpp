/***************************************************************************
                          imagemetainfo.cpp  -  description
                             -------------------
    begin                : Fri Apr 9 2004
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

#include "imagemetainfo.h"

// KDE
#include <kurl.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <kfilemetainfo.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klistview.h>
#include <kiconloader.h>
#include <ksqueezedtextlabel.h>
#include <kimageio.h>
#include <kpixmapio.h>

#ifdef HAVE_LIBKEXIF
#include <libkexif/kexifdata.h>
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
#endif /* HAVE_LIBKEXIF */

// QT
#include <qcolor.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qheader.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpainter.h>

#include "imagemetainfo.moc"

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

class  MetainfoItem : public KListViewItem
{
public:
	MetainfoItem(QListView *parent, QString str) : KListViewItem(parent, str){};
	MetainfoItem(KListViewItem *parent, QString str1, QString str2) : KListViewItem(parent, str1, str2){};

	virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
	{
		QFont font = p->font();
		if(column==1)
		{
			font.setBold(true);
		}
		font.setPointSize((font.pointSize()*4)/5);
		p->setFont(font);
		KListViewItem::paintCell(p, cg, column, width, alignment);
	}
};







ImageMetaInfo::ImageMetaInfo(QWidget *parent, const char* name)
	: QWidget  (parent, name)
{
    setMinimumSize( QSize( 0, 250 ) );
    Form1Layout = new QVBoxLayout( this, 11, 6, "Form1Layout");

    imagePathLabel = new KSqueezedTextLabel( this, "imagePathLabel" );
    imagePathLabel->setFrameShape( KSqueezedTextLabel::PopupPanel );
    imagePathLabel->setFrameShadow( KSqueezedTextLabel::Sunken );
    imagePathLabel->setAlignment( int( KSqueezedTextLabel::WordBreak | KSqueezedTextLabel::AlignCenter ) );
    Form1Layout->addWidget( imagePathLabel );

    info = new KListView( this, "info" );
    info->addColumn( i18n( "Key" ) );
    info->addColumn( i18n( "Value" ) );
    info->setProperty( "selectionMode", "NoSelection" );
    info->setShowSortIndicator( TRUE );
    info->setRootIsDecorated( TRUE );
    //info->setResizeMode( KListView::NoColumn );
    //info->setFullWidth( TRUE );
    info->setTreeStepSize(10);
    Form1Layout->addWidget( info );

    EXIFThumbnailTxtLabel = new QLabel( this, "EXIFThumbnailTxtLabel" );
    EXIFThumbnailTxtLabel->setFrameShape( QLabel::PopupPanel );
    EXIFThumbnailTxtLabel->setFrameShadow( QLabel::Sunken );
    EXIFThumbnailTxtLabel->setAlignment( int( QLabel::AlignCenter ) );
    Form1Layout->addWidget( EXIFThumbnailTxtLabel );

    EXIFThumbLabel = new QLabel( this, "EXIFThumbLabel" );
    EXIFThumbLabel->setFrameShape( QLabel::StyledPanel );
    EXIFThumbLabel->setFrameShadow( QLabel::Sunken );
    EXIFThumbLabel->setAlignment( int( QLabel::AlignCenter ) );
    Form1Layout->addWidget( EXIFThumbLabel );

    commentLabel = new QLabel( this, "commentLabel" );
    commentLabel->setFrameShape( QLabel::PopupPanel );
    commentLabel->setFrameShadow( QLabel::Sunken );
    commentLabel->setAlignment( int( QLabel::AlignCenter ) );
    Form1Layout->addWidget( commentLabel );

    comments = new KTextEdit( this, "comments" );
    comments->setMinimumSize( QSize( 0, 32 ) );
    comments->setMaximumSize( QSize( 32767, 64 ) );
    Form1Layout->addWidget( comments );
    //languageChange();
    resize( QSize(281, 387).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    /////////
    imagePathLabel->setText( i18n( "Exif Information" ) );
    info->header()->setLabel( 0, i18n( "Key" ) );
    info->header()->setLabel( 1, i18n( "Value" ) );
    commentLabel->setText( i18n( "Comment" ) );
    EXIFThumbnailTxtLabel->setText( i18n( "EXIF thumbnail" ) );
    info->header()->setMovingEnabled(false);
    info->header()->setStretchEnabled(true, -1);

    //////////
    lastComment=QString();
    hasComment = false;
    EXIFThumbLabel->hide();EXIFThumbnailTxtLabel->hide();
}

void
ImageMetaInfo::reload()
{
	setURL(m_current_url, m_current_mimeType);
}
void
ImageMetaInfo::setURL(const KURL& url, const QString& mimeType)
{
	m_current_url = url;
	m_current_mimeType = mimeType;
	m_datetime = QDateTime();

	if(hasComment
	&& lastComment!=comments->text()
	&&(!lastComment.isEmpty() || !comments->text().isEmpty()))
	{
		QVariant value(comments->text());
		fileMetaInfoItemComment->setValue( value );
		if(!metaInfo->applyChanges())
		{
			kdWarning()
				<<i18n("Unable to apply metainfo changes to %1")
					.arg(metaInfo->path())
				<<endl;
		}
		delete(fileMetaInfoItemComment);
	}
	textChanged=false;

	///
	if(url.isEmpty())
	{
		info->clear();
		imagePathLabel->setText(i18n("Exif Information"));
		comments->hide(); commentLabel->hide();
		EXIFThumbLabel->hide(); EXIFThumbnailTxtLabel->hide();
		info->setEnabled(false);
		return;
	}

	///
	KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url, true );
	metaInfo = new KFileMetaInfo(fileItem.metaInfo());
	QStringList groups = metaInfo->groups();
	hasComment=false;

	imagePathLabel->setText(QFileInfo(url.path()).fileName());
	info->clear();

	int col1size = info -> header () -> sectionSize (0);
	MetainfoItem* currentGroup = 0;
	comments->setText("");
	for ( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
	{
		currentGroup = new MetainfoItem(info, *it);
		currentGroup->setOpen(true);
		KFileMetaInfoGroup group = metaInfo->group(*it);
		QStringList supportedKeys = group.supportedKeys();
		for ( QStringList::Iterator keys = supportedKeys.begin(); keys != supportedKeys.end(); ++keys )
		{
			KFileMetaInfoItem item = group.item(*keys);
			QString key = item.key();
			QString value = item.string();

			if(!key.stripWhiteSpace().isEmpty() && value!="---")
			{
				(void)new MetainfoItem(currentGroup, key, value);
			}
			if(key=="Dimensions")
			{
				m_dimensions = value;
			}
			else
			if(key=="Date/time")
			{
				m_datetime = QDateTime::fromString(value, Qt::ISODate);
			}
			else
			if(key=="Comment" && mimeType.left(5)=="image")
			{
				const char newline[]={255, 254, 0, 74};
				comments->setTextFormat (Qt::PlainText);
				//FIXME pb the
				//QString newline=QString("%1%2%3%4").arg(QChar((uchar)255)).arg(QChar((uchar)254)).arg(QChar((uchar)0)).arg(QChar((uchar)74));
				value.replace(newline,"\n");
				//MYDEBUG << "'"<<newline<<"'"<<endl;
				//MYDEBUG << "'"<< value <<"'"<<endl;
				//value.replace(newline, "\n");
				//MYDEBUG << "'"<< value <<"'"<<endl;
				//MYDEBUG << "'"<< item.value()<<"'"<<endl;
				comments->setText(value);

				comments->setReadOnly(FALSE);
				lastComment=value;
				hasComment=true;
				fileMetaInfoItemComment = new KFileMetaInfoItem(item);
			}
		}
	}

#ifdef HAVE_LIBKEXIF
	if(mimeType == "image/jpeg")
	{
		KExifData exifData;
		exifData.readFromFile(url.path());
		QPixmap pix = KPixmapIO().convertToPixmap(exifData.getThumbnail());
		if(!pix.isNull())
		{
			EXIFThumbLabel->setPixmap(pix);
			EXIFThumbLabel->show(); EXIFThumbnailTxtLabel->show();
		}
		else
		{
			EXIFThumbLabel->setPixmap(QPixmap());
			EXIFThumbLabel->hide();EXIFThumbnailTxtLabel->hide();
		}
	}
	else
	{
		EXIFThumbLabel->setPixmap(QPixmap());
		EXIFThumbLabel->hide();EXIFThumbnailTxtLabel->hide();
	}
#endif /* HAVE_LIBKEXIF */

	if(!hasComment || (!QFileInfo(url.path()).isWritable() && comments->text().isEmpty()))
	{
		hasComment=false;
		comments->hide();commentLabel->hide();
	}
	else
	{
		comments->show();commentLabel->show();
	}
	hasClicked=false;
	info->setEnabled(info->childCount()!=0);
	comments->setEnabled(info->childCount()!=0 && QFileInfo(url.path()).isWritable());
	info -> header () -> resizeSection(0,col1size);
	info -> header () -> adjustHeaderSize () ;
	repaint();kapp->processEvents();
}

void
ImageMetaInfo::slotClicked(int, int)
{
	if(lastComment.isEmpty())
	{
		comments->setTextFormat (Qt::PlainText);
		comments->setText(lastComment);
	}
	hasClicked=true;
}


void
ImageMetaInfo::slotTextChanged ()
{
	textChanged=true && hasClicked;
}

QString
ImageMetaInfo::toString ()
{
	QListViewItem *item=info->firstChild ();
	QString output;
	while(item)
	{
		output+=item->text(0)+ " " + item->text(1)+"\n";
		item=item->itemBelow ();
	}
	return output;
}

QString
ImageMetaInfo::getComments()
{
	return comments->text();
}

KURL
ImageMetaInfo::getURL()
{
	return m_current_url;
}

QString
ImageMetaInfo::getDimensions()
{
	return m_dimensions;
}


QDateTime
ImageMetaInfo::getDatetime()
{
	if(!m_datetime.isValid())
		m_datetime = QFileInfo(getURL().path()).lastModified();
	return m_datetime;
}









