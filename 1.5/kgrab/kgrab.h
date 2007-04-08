/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KGRAB_H
#define KGRAB_H

#include <QBitmap>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QMouseEvent>
#include <QPixmap>

#include <kglobalsettings.h>
#include <kmainwindow.h>
#include <kurl.h>
#include <kaction.h>

class RegionGrabber;
class KGrabWidget;

#include "kdebug.h"
class KGrabPreview : public QLabel
{
    Q_OBJECT

    public:
        KGrabPreview(QWidget *parent);
        virtual ~KGrabPreview();
        void setPreview(QPixmap pixmap);

    signals:
        void startDrag();
        void callContextMenu( const QPoint& pos );

    protected:
        void mousePressEvent(QMouseEvent * e);
        void mouseMoveEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent * /*e*/);
        QPoint mClickPt;
};

class KGrab : public KMainWindow
{
  Q_OBJECT

public:
    KGrab(QWidget *parent= 0, bool grabCurrent=false);
    ~KGrab();

    enum CaptureMode { FullScreen=0, WindowUnderCursor=1, Region=2, ChildWindow=3 };

    bool save( const QString &filename );
    bool saveEqual( const KUrl& url );
    QString url() const { return filename.url(); }

public slots:
    void slotGrab();
    void slotSave();
    void slotSaveAs();
    void slotCopy();
    void slotPrint();
    void slotOpen();
    void slotMovePointer( int x, int y );
    void setTime(int newTime);
    void setURL(const QString &newURL);
    void setGrabMode( int m );
    void setDelay( int i );
    void setIncludeDecorations( bool b );
    void exit();

protected:
    void reject() { close(); }
    virtual void closeEvent( QCloseEvent * e );
    void resizeEvent(QResizeEvent*);
    bool eventFilter( QObject*, QEvent* );

private slots:
    void grabTimerDone();
    void slotDragSnapshot();
    void updateCaption();
    void updatePreview();
    void slotRegionGrabbed( const QPixmap & );
    void slotWindowGrabbed( const QPixmap & );
    void slotContextMenu( const QPoint& pos );
    void slotSettings( );
    void setPreview( const QPixmap &pm );
    void setMode( int mode );
    int delay();
    bool includeDecorations();
    int mode();
    QPixmap preview();
    int previewWidth();
    int previewHeight();

public:
    int grabMode();
    int timeout();

private:
    int  capturemode;
    int  delay_time;
    bool decoration;
    void createMenu();
    bool save( const KUrl& url );
    void performGrab();
    void autoincFilename();
    QPixmap snapshot;
    QTimer grabTimer;
    QTimer updateTimer;
    QWidget* grabber;
    KUrl filename;
    KGrabWidget *mainWidget;
    RegionGrabber *rgnGrab;
    bool modified;
    KAction *copyAct;
};

#endif // KGRAB_H
