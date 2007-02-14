/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * osd.cpp:   Shows some text in a pretty way independent to the WM
 * begin:     Fre Sep 26 2003
 * copyright: (C) 2004-2005 Christian Muehlhaeuser <chris@chris.de>
 *            (C) 2004 Seb Ruiz <seb100@optusnet.com.au>
 *            (C) 2004, 2005 Max Howell
 */

#include "osd.h"

#include <kdeversion.h>
#include <kapplication.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kdebug.h>

#include <qbitmap.h>
#include <qpainter.h>
#include <qtimer.h>

#define MYDEBUG kdDebug(0)<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

//namespace ShadowEngine
//{
//   QImage makeShadow( const QPixmap &textPixmap, const QColor &bgColor );
//}

OSDWidget::OSDWidget( QWidget *parent, const char *name )
        : QWidget( parent, name, WType_TopLevel | WStyle_Customize | WX11BypassWM | WStyle_StaysOnTop
#if KDE_IS_VERSION(3,3,0)
			| WNoAutoErase
#endif
			)
        , m_duration( 2000 )
        , m_timer( new QTimer( this ) )
        , m_alignment( Middle )
        , m_screen( 0 )
        , m_y( MARGIN )
        , m_drawShadow( true )
{
    setFocusPolicy( NoFocus );
    setBackgroundMode( NoBackground );
    unsetColors();

    connect( m_timer, SIGNAL( timeout() ), SLOT( hide() ) );

    //or crashes, KWin bug I think, crashes in QWidget::icon()
    kapp->setTopWidget( this );
}

void
OSDWidget::show() //virtual
{
    class Grabber : public QWidget {
    public:
        Grabber( const QRect &r, const QColor &color ) : QWidget( 0, 0 ) {
            move( 0, 0 );
            screen = QPixmap::grabWindow( winId(), r.x(), r.y(), r.width(), r.height() );
            KPixmapEffect::fade( screen, 0.80, color );
        }
        KPixmap screen;
    };

    if ( !isEnabled() )
        return;

    const QRect oldGeometry = QRect( pos(), size() );

    determineMetrics();

    const QRect newGeometry = QRect( pos(), size() );

    //TODO handle case when already shown properly
    if( !isShown() ) {
        // obtain snapshot of the screen where we are about to appear
        Grabber g( newGeometry, backgroundColor() );
        m_screenshot = g.screen;

        QWidget::show();
    }
    else
        paintEvent( 0 );

    if( m_duration ) //duration 0 -> stay forever
       m_timer->start( m_duration, true ); //calls hide()
}

void
OSDWidget::determineMetrics()
{
    static const uint HMARGIN = 20;
    static const uint VMARGIN = 10;

    // determine a sensible maximum size, don't cover the whole desktop or cross the screen
    const QSize margin( (HMARGIN + MARGIN) * 2, (VMARGIN + MARGIN) * 2 ); //margins
    const QSize image = m_image.isNull() ? QSize( 0, 0 ) : QSize( 80, 80 ); //80x80 is minimum image size
    const QSize max = QApplication::desktop()->screen( m_screen )->size() - margin;

    // The osd cannot be larger than the screen
    QSize text = max - image;
    QRect rect = fontMetrics().boundingRect( 0, 0, text.width(), text.height(), AlignLeft | WordBreak, m_text );

    if( !m_image.isNull() ) {
        const int availableWidth = max.width() - rect.width(); //WILL be >= 80

        int imageMetric;
        imageMetric = QMIN( availableWidth, rect.height() );
        imageMetric = QMIN( imageMetric, m_image.width() );

        const int widthIncludingImage = rect.width()
                + imageMetric
                + VMARGIN; //margin between text + image

        rect.setWidth( QMIN( widthIncludingImage, max.width() ) );

        m_image = m_image.smoothScale( imageMetric, imageMetric );
    }

    // size and move us
    rect.addCoords( -HMARGIN, -VMARGIN, HMARGIN, VMARGIN );
    reposition( rect.size() );
}

void
OSDWidget::reposition( QSize newSize )
{
    if( !newSize.isValid() ) newSize = size();

    QPoint newPos( MARGIN, m_y );
    const QRect screen = QApplication::desktop()->screenGeometry( m_screen );

    //TODO m_y is the middle of the OSD, and don't exceed screen margins

    switch ( m_alignment )
    {
        case Left:
            break;

        case Right:
            newPos.rx() = screen.width() - MARGIN - newSize.width();
            break;

        case Center:
            newPos.ry() = (screen.height() - newSize.height()) / 2;

            //FALL THROUGH

        case Middle:
            newPos.rx() = (screen.width() - newSize.width()) / 2;
            break;
    }

    //ensure we don't dip below the screen
    if ( newPos.y() + newSize.height() > screen.height() - MARGIN )
        newPos.ry() = screen.height() - MARGIN - newSize.height();

    // correct for screen position
    newPos += screen.topLeft();

    resize( newSize );
    move( newPos );
}

void
OSDWidget::paintEvent( QPaintEvent* )
{
    //TODO double buffer? but is slow...

//    if( !AmarokConfig::osdUseFakeTranslucency() )
       m_screenshot.fill( backgroundColor() );

    bitBlt( this, 0, 0, &m_screenshot );


    //else
    //    fill background with plain colour

    QPainter p;
    QRect crect = rect();
    QImage shadow;
    QFontMetrics metrics = fontMetrics();
    Qt::AlignmentFlags align;

    switch( m_alignment ) {
        case Left:  align = Qt::AlignLeft; break;
        case Right: align = Qt::AlignRight; break;
        default:    align = Qt::AlignHCenter; break;
    }

    crect.addCoords( 20, 10, -20, -10 );

    if( m_drawShadow )
    {
        QRect r = crect;
        QPixmap pixmap( size() );

        pixmap.fill( Qt::black );
        pixmap.setMask( pixmap.createHeuristicMask( true ) );

        p.begin( &pixmap );
        p.setFont( font() );
        p.setPen( Qt::white );
        p.setBrush( Qt::white );

        if( !m_image.isNull() ) {
            p.drawRect( 20, 10, m_image.width(), m_image.height() );
            r.rLeft() += m_image.width() + 10;
        }

        p.drawText( r, align | WordBreak, m_text );
        p.end();

       // int h,s,v;
        //foregroundColor().getHsv( &h, &s, &v );

        //shadow = NULL;FIXME ShadowEngine::makeShadow( pixmap, v > 128 ? Qt::black : Qt::white );
    }

    p.begin( this );
    //p.drawImage( 0, 0, shadow );
    p.setPen( foregroundColor() );
//
    if( !m_image.isNull() ) {
        p.drawImage( 20, 10, m_image );
        crect.rLeft() += m_image.width() + 10;
    }

    p.drawText( crect, align | WordBreak, m_text );
    p.setPen( backgroundColor().dark() );
    p.drawRect( rect() );
    p.end();
}

bool
OSDWidget::event( QEvent *e )
{
    switch( e->type() )
    {
        case QEvent::ApplicationPaletteChange:
//            if ( !AmarokConfig::osdUseCustomColors() )
//                unsetColors(); //use new palette's colours
            return true;
        default:
            return QWidget::event( e );
    }
}

void
OSDWidget::mousePressEvent( QMouseEvent* )
{
    hide();
}

void
OSDWidget::unsetColors()
{
    const QColorGroup c = QApplication::palette().active();

    setPaletteForegroundColor( c.highlightedText() );
    setPaletteBackgroundColor( c.highlight() );
}

void
OSDWidget::setScreen( int screen )
{
    const int n = QApplication::desktop()->numScreens();
    m_screen = (screen >= n) ? n-1 : screen;
    reposition();
}




//////  OSDPreviewWidget below /////////////////////

#include <kcursor.h>         //previewWidget
#include <klocale.h>

OSDPreviewWidget::OSDPreviewWidget( QWidget *parent )
    : OSDWidget( parent, "osdpreview" )
    , m_dragging( false )
{
    m_text = i18n( "OSD Preview - drag to reposition" );
    m_duration    = 0;
}

void OSDPreviewWidget::mousePressEvent( QMouseEvent *event )
{
    m_dragOffset = event->pos();

    if ( event->button() == LeftButton && !m_dragging )
    {
        grabMouse( KCursor::sizeAllCursor() );
        m_dragging = true;
    }
}


void OSDPreviewWidget::mouseReleaseEvent( QMouseEvent * /*event*/ )
{
    if ( m_dragging )
    {
        m_dragging = false;
        releaseMouse();

        // compute current Position && offset
        QDesktopWidget *desktop = QApplication::desktop();
        int currentScreen = desktop->screenNumber( pos() );

        if ( currentScreen != -1 )
        {
            // set new data
            m_screen = currentScreen;
            m_y      = QWidget::y();

            emit positionChanged();
        }
    }
}


void OSDPreviewWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ( m_dragging && this == mouseGrabber() )
    {
        const QRect screen      = QApplication::desktop()->screenGeometry( m_screen );
        const uint  hcenter     = screen.width() / 2;
        const uint  eGlobalPosX = e->globalPos().x() - screen.left();
        const uint  snapZone    = screen.width() / 8;

        QPoint destination = e->globalPos() - m_dragOffset - screen.topLeft();
        int maxY = screen.height() - height() - MARGIN;
        if( destination.y() < MARGIN ) destination.ry() = MARGIN;
        if( destination.y() > maxY ) destination.ry() = maxY;

        if( eGlobalPosX < (hcenter-snapZone) )
        {
            m_alignment = Left;
            destination.rx() = MARGIN;
        }
        else if( eGlobalPosX > (hcenter+snapZone) )
        {
            m_alignment = Right;
            destination.rx() = screen.width() - MARGIN - width();
        }
        else
        {
            const uint eGlobalPosY = e->globalPos().y() - screen.top();
            const uint vcenter     = screen.height()/2;

            destination.rx() = hcenter - width()/2;

            if( eGlobalPosY >= (vcenter-snapZone) && eGlobalPosY <= (vcenter+snapZone) )
            {
                m_alignment = Center;
                destination.ry() = vcenter - height()/2;
            }
            else m_alignment = Middle;
        }

        destination += screen.topLeft();

        move( destination );
    }
}


ShowimgOSD::ShowimgOSD( QWidget *parent )
	: OSDWidget(parent),
	m_dispOSD(true), m_onTop(true), m_sFilename(true), m_sFullpath(true), m_sDimensions(true), m_sComments(true), m_sExif(false)
{
	m_parent = parent;
}

void
ShowimgOSD::show()
{
	QString osdText;
	if(m_sFullpath)
		osdText+=m_fullpath+'/';
	if(m_sFilename)
		osdText+=m_filename+'\n';
	if(m_sFullpath&&!m_sFilename)
		osdText+='\n';
	if(m_sDimensions && !m_dimensions.isEmpty())
		osdText+=m_dimensions+'\n';
	if(m_sDatetime && !m_datetime.isEmpty())
		osdText+=m_datetime+"\n";
	if(m_sComments && !m_comments.isEmpty())
		osdText+=m_comments+'\n';
	if(m_sExif && !m_exif.isEmpty())
		osdText+=m_exif;

	OSDWidget::setText(osdText);
	if(m_dispOSD && ! osdText.isEmpty())
	{
		if(m_onTop)
		{
			setOffset(parentWidget()->mapToGlobal(QPoint(0,0)).y()+10);
		}
		else
		{
			const uint  bottomScreen    = parentWidget()->mapToGlobal(QPoint(0,0)).y() + parentWidget()->height();
			setOffset(bottomScreen - height() - 10 );
		}
		reposition();
		OSDWidget::show();
		repaint();
// 		kapp->processEvents();

	}
	else
		OSDWidget::hide();
}

void
ShowimgOSD::initOSD(bool dispOSD, bool onTop, const QFont& font, bool sFilename, bool sFullpath, bool sDimensions, bool sComments, bool sDatetime, bool sExif)
{
	m_dispOSD = dispOSD;
	m_onTop=onTop;
	setFont(font);

	m_sFilename=sFilename;
	m_sFullpath=sFullpath;
	m_sDimensions=sDimensions;
	m_sComments=sComments;
	m_sDatetime = sDatetime;
	m_sExif=sExif;
}


void
ShowimgOSD::setTexts(const QString& filename, const QString& fullpath, const QString& dimensions, const QString& comments, const QString& datetime, const QString& exif)
{
	m_filename=filename;
	m_fullpath=fullpath;
	m_dimensions=dimensions;
	m_comments=comments;
	m_datetime = datetime;
	m_exif=exif;
}

bool
ShowimgOSD::getShowOSD()
{
	return m_dispOSD;
}
bool
ShowimgOSD::getOSDOnTop()
{
	return m_onTop;
}
bool
ShowimgOSD::getOSDShowFilename()
{
	return m_sFilename;
}
bool
ShowimgOSD::getOSDShowFullpath()
{
	return m_sFullpath;
}
bool
ShowimgOSD::getOSDShowDimensions()
{
	return m_sDimensions;
}
bool
ShowimgOSD::getOSDShowComments()
{
	return m_sComments;
}
bool
ShowimgOSD::getOSDShowDatetime()
{
	return m_sDatetime;
}

bool
ShowimgOSD::getOSDShowExif()
{
	return m_sExif;
}

#ifdef NDEBUG
#warning Please, please compile with --enable-debug=full!
#endif

#include "osd.moc"
