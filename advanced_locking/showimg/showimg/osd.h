/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  osd.h   -  Provides an interface to a plain QWidget, which is independent of KDE (bypassed to X11)
  begin:     Fre Sep 26 2003
  copyright: (C) 2003-2005 by Christian Muehlhaeuser
  email:     chris@chris.de
*/

#ifndef AMAROK_OSD_H
#define AMAROK_OSD_H

#include <kpixmap.h>
#include <qimage.h>
#include <qvaluelist.h>
#include <qwidget.h> //baseclass


class OSDWidget : public QWidget
{
    Q_OBJECT

    public:
        enum Alignment { Left, Middle, Center, Right };

        OSDWidget( QWidget *parent, const char *name = "osd" );

        /** resets the colours to defaults */
        void unsetColors();

      public slots:
        /** calls setText() then show() */
        void show( const QString &text ) { setText( text ); show(); }

        /** reimplemented, shows the OSD */
        virtual void show();

        /**
         * For the sake of somplicity, when these settings are
         * changed they do not take effect until the next time
         * the OSD is shown!
         *
         * To force an update call show();
         */
        void setDuration( int ms ) { m_duration = ms; }
        void setTextColor( const QColor &color ) { setPaletteForegroundColor( color ); }
        void setBackgroundColor(const QColor &color ) { setPaletteBackgroundColor( color ); }
        void setOffset( int y ) { m_y = y; }
        void setAlignment( Alignment alignment ) { m_alignment = alignment; }
        void setImage( const QImage &image ) { m_image = image; }
        void setScreen( int screen );
        void setText( const QString &text ) { m_text = text;  determineMetrics();}
        void setDrawShadow( bool b ) { m_drawShadow = b; }

    protected:
        /** determine new size and position */
        void determineMetrics();

        /** call to reposition a new OSD text or when position attributes change */
        void reposition( QSize newSize = QSize() );

        /** reimplemented */
        virtual void mousePressEvent( QMouseEvent* );
        virtual void paintEvent( QPaintEvent* );
        virtual bool event( QEvent* );

        /** distance from screen edge */
        static const int MARGIN = 15;

        /** padding inside the OSD frame */
        static const int HPADDING = 20;
        static const int VPADDING = 10;

        int         m_duration;
        QTimer     *m_timer;
        Alignment   m_alignment;
        int         m_screen;
        uint        m_y;
        bool        m_drawShadow;
        QString     m_text;
        QImage      m_image;
        KPixmap     m_screenshot;
};



class OSDPreviewWidget : public OSDWidget
{
    Q_OBJECT

public:
    OSDPreviewWidget( QWidget *parent );

    int screen()    { return m_screen; }
    int alignment() { return m_alignment; }
    int y()         { return m_y; }

public slots:
    void setTextColor( const QColor &color ) { OSDWidget::setTextColor( color ); doUpdate(); }
    void setBackgroundColor(const QColor &color ) { OSDWidget::setBackgroundColor( color ); doUpdate(); }
    void setDrawShadow( bool b ) { OSDWidget::setDrawShadow( b ); doUpdate(); }
    void setFont( const QFont &font ) { OSDWidget::setFont( font ); doUpdate(); }

private:
    inline void doUpdate() { if( isShown() ) show(); }

signals:
    void positionChanged();

protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );

private:
    bool   m_dragging;
    QPoint m_dragOffset;
};


class ShowimgOSD : public OSDWidget
{
	Q_OBJECT

	public:
		ShowimgOSD( QWidget *parent );

		void initOSD(bool dispOSD, bool onTop, const QFont& font, bool sFilename, bool sFullpath, bool sDimensions, bool sComments, bool sDatetime, bool sExif);

		void setTexts(const QString& filename, const QString& fullpath, const QString& dimensions, const QString& comments, const QString& datetime, const QString& exif);

		bool getShowOSD();
		bool getOSDOnTop();
		bool getOSDShowFilename();
		bool getOSDShowFullpath();
		bool getOSDShowDimensions();
		bool getOSDShowComments();
		bool getOSDShowDatetime();
		bool getOSDShowExif();

	public slots:
		/** reimplemented, shows the OSD */
		virtual void show();

	protected:
		bool m_dispOSD, m_onTop, m_sFilename, m_sFullpath, m_sDimensions, m_sComments, m_sDatetime, m_sExif;
		QString m_filename, m_fullpath, m_dimensions, m_comments, m_datetime, m_exif;

		QWidget *m_parent;

};




#endif /*AMAROK_OSD_H*/
