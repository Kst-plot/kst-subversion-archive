/*
 *  Copyright (C) 1997-2002 Richard J. Moore		(ksnapshot)
 *  Copyright (C) 2000 Matthias Ettrich			(ksnapshot)
 *  Copyright (C) 2002 Aaron J. Seigo			(ksnapshot)
 *  Copyright (C) 2003 Nadeem Hasan			(ksnapshot)
 *  Copyright (C) 2004 Bernd Brandstetter		(ksnapshot)
 *  Copyright (C) 2006 Urs Wolfer <uwolfer @ fwo.ch>    (ksnapshot)
 *  Copyright (C) 2006 Marcus Hufgard <Marcus.Hufgard@hufgard.de>
 *
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

#include <QClipboard>
#include <QPainter>
#include <QShortcut>
#include <QVBoxLayout>
#include <QX11Info>

#include <klocale.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kimagefilepreview.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kio/netaccess.h>
#include <ksavefile.h>
#include <ktemporaryfile.h>
#include <knotification.h>
#include <kmenu.h>
#include <kstartupinfo.h>
#include <kvbox.h>
#include <kglobal.h>
#include <kicon.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kedittoolbar.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>

#include "kgrab.h"
#include "regiongrabber.h"
#include "windowgrabber.h"
#include "ui_kgrabwidget.h"
#include "kgrab_settings.h"
#include "openwith.h"


#include <X11/Xlib.h>
#include <X11/Xatom.h>


// ----------------    KGrabWidget ---------------------------------

class KGrabWidget : public QWidget, public Ui::KGrabWidget
{
    public:
        KGrabWidget(QWidget *parent = 0)
        : QWidget(parent)
        {
            setupUi(this);
        }
};

// ----------------    KGrabPreview ---------------------------------

KGrabPreview::KGrabPreview(QWidget *parent)
: QLabel(parent)
{
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    setCursor( Qt::OpenHandCursor );
}

KGrabPreview::~KGrabPreview()
{
}

void 
KGrabPreview::setPreview(QPixmap pixmap)
{
    // if this looks convoluted, that's because it is. drawing a PE_SizeGrip
     // does unexpected things when painting directly onto the pixmap
    QPixmap handle(15, 15);
    QBitmap mask(15, 15);
    mask.clear();
    QStyleOption o;
    o.rect = QRect(0, 0, 15, 15);
    {
        QPainter p(&mask);
        style()->drawControl(QStyle::CE_SizeGrip, &o, &p);
        p.end();
        handle.setMask(mask);
    }
    {
        QPainter p(&handle);
        style()->drawControl(QStyle::CE_SizeGrip, &o, &p);
        p.end();
    }
    o.rect = QRect(pixmap.width() - 16, pixmap.height() - 16, 15, 15);
    QPainter p(&pixmap);
    p.drawPixmap(o.rect, handle);
    p.end();
   // hooray for making things like setPixmap not virtual! *sigh*
   setPixmap(pixmap);
}

void 
KGrabPreview::mousePressEvent(QMouseEvent * e)
{
    if ( e->button() == Qt::LeftButton )
        mClickPt = e->pos();
    if ( e->button() == Qt::RightButton ) 
    {
        emit callContextMenu( e->globalPos() );
    }
}

void 
KGrabPreview::mouseMoveEvent(QMouseEvent * e)
{
   if (mClickPt != QPoint(0, 0) &&
       (e->pos() - mClickPt).manhattanLength() > KGlobalSettings::dndEventDelay())
   {
       mClickPt = QPoint(0, 0);
       emit startDrag();
   }
}

void 
KGrabPreview::mouseReleaseEvent(QMouseEvent * /*e*/)
{
    mClickPt = QPoint(0, 0);
}

// ----------------    KGrab ---------------------------------

KGrab::KGrab(QWidget *parent, bool grabCurrent)
  : KMainWindow(parent)
{
    setObjectName( "toplevel" );
    setCaption( "" );
    createMenu();
    setupGUI();


    KStartupInfo::appStarted();

    KVBox *vbox = new KVBox( this );
    vbox->setSpacing( KDialog::spacingHint() );
    setCentralWidget(vbox);
    mainWidget = new KGrabWidget( vbox );

    connect( mainWidget->lblImage , SIGNAL( startDrag() ), SLOT( slotDragSnapshot() ) );
    connect( mainWidget->lblImage , SIGNAL( callContextMenu( const QPoint& ) ) , 
             SLOT( slotContextMenu( const QPoint& ) ) );

    grabber = new QWidget( 0,  Qt::X11BypassWindowManagerHint );
    grabber->move( -1000, -1000 );
    grabber->installEventFilter( this );
 
    grabber->show();
    grabber->grabMouse( Qt::WaitCursor );

    if ( !grabCurrent )
        snapshot = QPixmap::grabWindow( QX11Info::appRootWindow() );
    else {
        setMode( WindowUnderCursor );
        setIncludeDecorations( true );
        performGrab();
    }

    grabber->releaseMouse();
    grabber->hide();

    KSharedConfig::Ptr conf=KGlobal::config();
    conf->setGroup("GENERAL");
    setDelay( conf->readEntry("delay", 0) );
    setMode( conf->readEntry("mode", 0) );
    setIncludeDecorations(conf->readEntry("includeDecorations",true));
    filename = KUrl( conf->readPathEntry( "filename", QDir::currentPath()+'/'+"snapshot"+"1.png" ));

    // Make sure the name is not already being used
    while(KIO::NetAccess::exists( filename, false, this )) {
        autoincFilename();
    }

    connect( &grabTimer, SIGNAL( timeout() ), this, SLOT(  grabTimerDone() ) );
    connect( &updateTimer, SIGNAL( timeout() ), this, SLOT(  updatePreview() ) );
    QTimer::singleShot( 0, this, SLOT( updateCaption() ) );
}


KGrab::~KGrab()
{
    delete grabber;
    delete mainWidget;
}


void
KGrab::createMenu()
{
    // Menu File

    KActionCollection *coll= actionCollection();
    
    KAction *newAct = new KAction( KIcon("filenew"), i18n("&New Snapshot"), this );
    newAct->setShortcut( i18n("Ctrl+N") );
    newAct->setStatusTip( i18n("Make new Snapshot") );
    connect( newAct, SIGNAL(triggered()), this, SLOT( slotGrab() ) );
    coll->addAction( "new_snapshot", newAct );

    coll->addAction( "file_saveas", 
                     KStandardAction::saveAs( this, SLOT( slotSaveAs() ), actionCollection() ) );

    KAction *owAct = new KAction( KIcon("project_open"), i18n("Open &With..."), this ); 
    owAct->setShortcut( i18n("Ctrl+W") );
    owAct->setStatusTip( i18n("Open the Snapshot with an other application") );
    connect( owAct, SIGNAL(triggered()), this, SLOT( slotOpen() ) );
    coll->addAction( "file_openwith", owAct );

    coll->addAction( "file_print", KStandardAction::print( this, SLOT( slotPrint() ), actionCollection() ) );

    coll->addAction( "file_quit", KStandardAction::quit( this, SLOT( exit() ), actionCollection() ) );

   
    // Edit Menu

    copyAct = new KAction( KIcon("editcopy"), i18n("&Copy"), this );
    copyAct->setShortcut( i18n("Ctrl+C") );
    copyAct->setStatusTip( i18n("Copy the Snapshot to clipboard") );
    connect( copyAct, SIGNAL(triggered()), this, SLOT( slotCopy() ) );
    coll->addAction( "edit_copy", copyAct );

    // Menu Settings

    KStandardAction::preferences( this, SLOT( slotSettings() ), actionCollection() );
    KStandardAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
}


void KGrab::resizeEvent( QResizeEvent * )
{
    updateTimer.setSingleShot( true );
    updateTimer.start( 200 );
}

bool KGrab::save( const QString &filename )
{
    return save( KUrl( filename ));
}

bool KGrab::save( const KUrl& url )
{
    if ( KIO::NetAccess::exists( url, false, this ) ) {
        const QString title = i18n( "File Exists" );
        const QString text = i18n( "<qt>Do you really want to overwrite <b>%1</b>?</qt>" , url.prettyUrl());
        if (KMessageBox::Continue != KMessageBox::warningContinueCancel( this, text, title, 					     KStandardGuiItem::cont(), i18n("Overwrite") ) )
        {
            return false;
        }
    }
    return saveEqual( url );
}

bool KGrab::saveEqual( const KUrl& url )
{
    QByteArray type = "PNG";
    QString mime = KMimeType::findByUrl( url.fileName(), 0, url.isLocalFile(), true )->name();
    QStringList types = KImageIO::typeForMime(mime);
    if ( !types.isEmpty() )
        type = types.first().toLatin1();

    bool ok = false;

    if ( url.isLocalFile() ) 
    {
        KSaveFile saveFile( url.path() );
        if ( saveFile.open()) 
        {
            if ( snapshot.save( &saveFile, type ) )
                ok = saveFile.finalize();
        }
    }
    else 
    {
        KTemporaryFile tmpFile;
        if ( tmpFile.open() ) 
        {
            if ( snapshot.save( &tmpFile, type ) ) 
                ok = KIO::NetAccess::upload( tmpFile.fileName(), url, this );
        }
    }

    QApplication::restoreOverrideCursor();
    if ( !ok ) {
        kWarning() << "KGrab was unable to save the snapshot" << endl;

        QString caption = i18n("Unable to save image");
        QString text = i18n("KGrab was unable to save the image to\n%1.", url.prettyUrl());
        KMessageBox::error(this, text, caption);
    }

    return ok;
}

void KGrab::slotSave()
{
    if ( save(filename) ) {
        modified = false;
        autoincFilename();
    }
}

void KGrab::slotSaveAs()
{
    QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Writing );
    KFileDialog dlg( filename.url(), mimetypes.join(" "), this);

    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setCaption( i18n("Save As") );

    KImageFilePreview *ip = new KImageFilePreview( &dlg );
    dlg.setPreviewWidget( ip );

    if ( !dlg.exec() )
        return;

    KUrl url = dlg.selectedUrl();
    if ( !url.isValid() )
        return;

    if ( save(url) ) {
        filename = url;
        modified = false;
        autoincFilename();
    }
}

void KGrab::slotCopy()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setPixmap( snapshot );
}

void KGrab::slotDragSnapshot()
{
    QDrag *drag = new QDrag(this);

    drag->setMimeData(new QMimeData);
    drag->mimeData()->setImageData(snapshot);
    drag->setPixmap(preview());
    drag->start();
}

void KGrab::slotGrab()
{
    hide();

    if ( delay() ) {
        grabTimer.setSingleShot( true );
        grabTimer.start( delay() * 1000 );
    }
    else {
        if ( mode() == Region ) {
            rgnGrab = new RegionGrabber();
            connect( rgnGrab, SIGNAL( regionGrabbed( const QPixmap & ) ),
                              SLOT( slotRegionGrabbed( const QPixmap & ) ) );
        }
        else {
            grabber->show();
            grabber->grabMouse( Qt::CrossCursor );
        }
    }
}

void KGrab::slotPrint()
{
    KPrinter printer;
    if (snapshot.width() > snapshot.height())
        printer.setOrientation(KPrinter::Landscape);
    else
        printer.setOrientation(KPrinter::Portrait);

    qApp->processEvents();

    if (printer.setup(this, i18n("Print Screenshot")))
    {
        qApp->processEvents();

        QPainter painter(&printer);

        float w = snapshot.width();
        float dw = w - printer.width();
        float h = snapshot.height();
        float dh = h - printer.height();
        bool scale = false;

        if ( (dw > 0.0) || (dh > 0.0) )
            scale = true;

        if ( scale ) {

            QImage img = snapshot.toImage();
            qApp->processEvents();

            float newh, neww;
            if ( dw > dh ) {
                neww = w-dw;
                newh = neww/w*h;
            }
            else {
                newh = h-dh;
                neww = newh/h*w;
            }

            img = img.scaled( int(neww), int(newh), Qt::KeepAspectRatio, Qt::SmoothTransformation );
            qApp->processEvents();

            int x = (printer.width()-img.width())/2;
            int y = (printer.height()-img.height())/2;

            painter.drawImage( x, y, img);
        }
        else {
            int x = (printer.width()-snapshot.width())/2;
            int y = (printer.height()-snapshot.height())/2;
            painter.drawPixmap( x, y, snapshot );
        }
    }

    qApp->processEvents();
}

void KGrab::slotOpen()
{
    QString fileopen = KStandardDirs::locateLocal( "tmp", filename.fileName() );

    if( !saveEqual( fileopen ) )
        return;
    OpenWith* ow = new OpenWith( QString(  "image/png" ), fileopen, this );
    ow->exec();
}

void KGrab::slotRegionGrabbed( const QPixmap &pix )
{
  if ( !pix.isNull() )
  {
    snapshot = pix;
    updatePreview();
    modified = true;
    updateCaption();
  }

  delete rgnGrab;
  QApplication::restoreOverrideCursor();
  show();
}

void KGrab::slotWindowGrabbed( const QPixmap &pix )
{
    if ( !pix.isNull() )
    {
        snapshot = pix;
        updatePreview();
        modified = true;
        updateCaption();
    }

    QApplication::restoreOverrideCursor();
    show();
}

void KGrab::slotSettings( )
{
    KGrabSettings settings( this );
    settings.slotModeChanged( capturemode );
    settings.setDelay( delay_time );
    settings.setDecorations( decoration );
    settings.exec();
}

void KGrab::closeEvent( QCloseEvent * e )
{
    KSharedConfig::Ptr conf=KGlobal::config();
    conf->setGroup("GENERAL");
    conf->writeEntry("delay",delay());
    conf->writeEntry("mode",mode());
    conf->writeEntry("includeDecorations",includeDecorations());
    KUrl url = filename;
    url.setPass( QString::null );
    conf->writePathEntry("filename",url.url());
    e->accept();
}

bool KGrab::eventFilter( QObject* o, QEvent* e)
{
    if ( o == grabber && e->type() == QEvent::MouseButtonPress ) {
        QMouseEvent* me = (QMouseEvent*) e;
        if ( QWidget::mouseGrabber() != grabber )
            return false;
        if ( me->button() == Qt::LeftButton )
            performGrab();
    }
    return false;
}

void KGrab::autoincFilename()
{
    // Extract the filename from the path
    QString name= filename.fileName();

    // If the name contains a number then increment it
    QRegExp numSearch("[0-9]+");

    // Does it have a number?
    int start = numSearch.indexIn(name);
    if (start != -1) {
        // It has a number, increment it
        int len = numSearch.matchedLength();
        QString numAsStr= name.mid(start, len);
        QString number = QString::number(numAsStr.toInt() + 1);
        number = number.rightJustified( len, '0');
        name.replace(start, len, number );
    }
    else {
        // no number
        start = name.lastIndexOf('.');
        if (start != -1) {
            // has a . somewhere, e.g. it has an extension
            name.insert(start, '1');
        }
        else {
            // no extension, just tack it on to the end
            name += '1';
        }
    }

    //Rebuild the path
    KUrl newURL = filename;
    newURL.setFileName( name );
    setURL( newURL.url() );
}

void KGrab::updatePreview()
{
    setPreview( snapshot );
}

void KGrab::grabTimerDone()
{
    if ( mode() == Region ) {
        rgnGrab = new RegionGrabber();
        connect( rgnGrab, SIGNAL( regionGrabbed( const QPixmap & ) ),
                          SLOT( slotRegionGrabbed( const QPixmap & ) ) );
    }
    else {
        performGrab();
    }
    KNotification::beep(i18n("The screen has been successfully grabbed."));
}

void KGrab::performGrab()
{
    grabber->releaseMouse();
    grabber->hide();
    grabTimer.stop();
    if ( mode() == ChildWindow ) {
        WindowGrabber wndGrab;
        connect( &wndGrab, SIGNAL( windowGrabbed( const QPixmap & ) ),
                           SLOT( slotWindowGrabbed( const QPixmap & ) ) );
        wndGrab.exec();
    }
    else if ( mode() == WindowUnderCursor ) {
        snapshot = WindowGrabber::grabCurrent( includeDecorations() );
    }
    else {
        snapshot = QPixmap::grabWindow( QX11Info::appRootWindow() );
    }
    updatePreview();
    QApplication::restoreOverrideCursor();
    modified = true;
    updateCaption();
    show();
}

void KGrab::setTime(int newTime)
{
    setDelay(newTime);
}

int KGrab::timeout()
{
    return delay();
}

void KGrab::setURL( const QString &url )
{
    KUrl newURL = KUrl( url );
    if ( newURL == filename )
        return;

    filename = newURL;
    updateCaption();
}

void 
KGrab::setGrabMode( int m )
{
    setMode( m );
}

int 
KGrab::grabMode()
{
    return mode();
}

void 
KGrab::updateCaption()
{
    setCaption( filename.fileName(), true );
}

void 
KGrab::slotMovePointer(int x, int y)
{
    QCursor::setPos( x, y );
}

void 
KGrab::slotContextMenu( const QPoint& pos )
{
    QMenu *menuCon = new QMenu();
    menuCon->addAction( copyAct );

    menuCon->popup( pos );
}


void KGrab::exit()
{
    reject();
}

void KGrab::setPreview( const QPixmap &pm )
{
    QPixmap pmScaled = pm.scaled( previewWidth(), previewHeight(), Qt::KeepAspectRatio, Qt::SmoothTransformation );

    mainWidget->lblImage->setToolTip(
        i18n( "Preview of the snapshot image (%1 x %2)" ,
          pm.width(), pm.height() ) );

    mainWidget->lblImage->setPreview( pmScaled );
    mainWidget->lblImage->adjustSize();
}

void KGrab::setDelay( int i )
{
    delay_time = i;
}

void KGrab::setIncludeDecorations( bool b )
{
    decoration = b;
}

void KGrab::setMode( int mode )
{
    capturemode = mode;
}

int KGrab::delay()
{
    return delay_time;
}

bool KGrab::includeDecorations()
{
    return decoration;
}

int KGrab::mode()
{
    return capturemode;
}

QPixmap KGrab::preview()
{
    return *mainWidget->lblImage->pixmap();
}

int KGrab::previewWidth()
{
    return mainWidget->lblImage->width();
}

int KGrab::previewHeight()
{
    return mainWidget->lblImage->height();
}

#include "kgrab.moc"
