/*****************************************************************************
                          imagemetainfo.cpp  -  description
                             -------------------
    begin                : Fri Apr 9 2004
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

#include "imagemetainfo.h"

//Local
#include "showimg_common.h"
#include "tools.h"

// KDE
#include <kapplication.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kfilemetainfo.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klistview.h>
#include <klocale.h>
#include <kpixmapio.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kurl.h>

#ifdef HAVE_LIBEXIV2
#include <exiv2/exif.hpp>
#include "metadata/dmetadata.h"
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIV2
#endif
#endif /* HAVE_LIBEXIV2 */

// QT
#include <qcolor.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstringlist.h>

#include "imagemetainfo.moc"


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


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


ImageMetaInfoView::ImageMetaInfoView(QWidget *parent)
	: QScrollView(parent, "ImageMetaInfoView")
{
	setResizePolicy ( AutoOneFit );

	m_imageMetaInfo = new ImageMetaInfo(viewport());
	addChild(m_imageMetaInfo);
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


ImageMetaInfo::ImageMetaInfo(QWidget *parent, const char* name)
	: QWidget  (parent, name),
	
	m_lastComment(""),

	m_hasComment(false), 
	textChanged(false), 
	hasClicked(false),
	
	m_p_fileMetaInfoItemComment(NULL),
	m_p_metaInfo(NULL),

	m_current_url(),
	
	m_current_mimeType(""),
	m_dimensions(""),
	
	m_datetime()

{
    //setMinimumSize( QSize( 0, 250 ) );
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
    info->setShowSortIndicator( true );
    info->setRootIsDecorated( true );
    //info->setResizeMode( KListView::NoColumn );
    //info->setFullWidth( true );
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


    //QSpacerItem *spacer4 = new QSpacerItem( 20, 41, QSizePolicy::Minimum, QSizePolicy::Expanding );
    //Form1Layout->addItem( spacer4 );


    //languageChange();
    //resize( QSize(281, 387).expandedTo(minimumSizeHint()) );
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
    m_lastComment=QString();
    m_hasComment = false;
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

	if(
		m_hasComment
		&& m_lastComment!=comments->text()
		&&(!m_lastComment.isEmpty() || !comments->text().isEmpty()))
	{
		QVariant l_value(comments->text());
		m_p_fileMetaInfoItemComment->setValue( l_value );
		if(!m_p_metaInfo->applyChanges())
		{
			MYWARNING
				<<i18n("Unable to apply metainfo changes to %1")
					.arg(m_p_metaInfo->path())
				<<endl;
		}
		delete(m_p_fileMetaInfoItemComment);
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
	
	delete(m_p_metaInfo);
	m_p_metaInfo = new KFileMetaInfo(fileItem.metaInfo());
	
	QStringList l_group_list = m_p_metaInfo->groups();
	m_hasComment=false;

	imagePathLabel->setText(QFileInfo(url.path()).fileName());
	info->clear();

	int col1size = info -> header () -> sectionSize (0);
	MetainfoItem* l_p_current_metainfoItem = 0;
	comments->setText("");
	for ( QStringList::Iterator it = l_group_list.begin(); it != l_group_list.end(); ++it )
	{
		delete(l_p_current_metainfoItem);
		l_p_current_metainfoItem = new MetainfoItem(info, *it);
		
		l_p_current_metainfoItem->setOpen(true);
		KFileMetaInfoGroup l_fileMetaInfoGroup = m_p_metaInfo->group(*it);
		QStringList supportedKeys = l_fileMetaInfoGroup.supportedKeys();
		for ( QStringList::Iterator keys = supportedKeys.begin(); keys != supportedKeys.end(); ++keys )
		{
			KFileMetaInfoItem item = l_fileMetaInfoGroup.item(*keys);
			QString key = item.key();
			QString l_value = item.string();

			if(!key.stripWhiteSpace().isEmpty() && l_value!="---")
			{
				(void)new MetainfoItem(l_p_current_metainfoItem, key, l_value);
			}
			if(key==QString::fromLatin1("Dimensions"))
			{
				m_dimensions = l_value;
			}
			else
			if(key==QString::fromLatin1("Date/time"))
			{
				m_datetime = QDateTime::fromString(l_value, Qt::ISODate);
			}
			else
			if(key==QString::fromLatin1("Comment") && Tools::isImage(url, mimeType))
			{
				const char newline[]={255, 254, 0, 74};
				comments->setTextFormat (Qt::PlainText);
				l_value.replace(newline,"\n");
				comments->setText(l_value);

				comments->setReadOnly(false);
				m_lastComment=l_value;
				m_hasComment=true;
				
				delete(m_p_fileMetaInfoItemComment);
				m_p_fileMetaInfoItemComment = new KFileMetaInfoItem(item);
			}
		}
	}

#ifdef HAVE_LIBEXIV2
	if( 
		Tools::isJPEG(url, mimeType)  &&
		url.isLocalFile()             &&
		QFileInfo(url.path()).exists()
	)
	{
		DMetadata l_exif_data(url.path(), "JPEG");
		QPixmap pix = KPixmapIO().convertToPixmap(l_exif_data.getExifThumbnail(false));
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
#endif /* HAVE_LIBEXIV2 */

	if(!m_hasComment || (!QFileInfo(url.path()).isWritable() && comments->text().isEmpty()))
	{
		m_hasComment=false;
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
	update();
// 	kapp->processEvents();
}

void
ImageMetaInfo::slotClicked(int, int)
{
	if(m_lastComment.isEmpty())
	{
		comments->setTextFormat (Qt::PlainText);
		comments->setText(m_lastComment);
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
		output+=item->text(0)+ ' ' + item->text(1)+'\n';
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









