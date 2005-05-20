/***************************************************************************
                          imageviewer.cpp  -  description
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

#include "imageviewer.h"

// Local
#include "imagelistview.h"
#include "mainwindow.h"
#include "fileiconitem.h"
#include "directoryview.h"
#include "printimagedialog.h"
#include "misc/kmagick.h"

#ifdef WANT_DIGIKAMIMAGEEFFECT
#include <digikam/imageiface.h>
#ifdef __GNUC__
#warning WANT_DIGIKAMIMAGEEFFECT enabled
#endif
#else
#ifdef __GNUC__
#warning WANT_DIGIKAMIMAGEEFFECT disabled
#endif
#endif

#include <stdlib.h>
#include <math.h>

// KDE
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <krun.h>
#include <kaction.h>
#include <kpixmapio.h>
#include <kimageio.h>
#include <kaction.h>
#include <klocale.h>
#include <kwin.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kstdaccel.h>
#include <kshortcut.h>
#include <kmimetype.h>
#include <kiconloader.h>
#ifndef Q_WS_WIN //TODO
#include <kprinter.h>
#endif
#ifdef HAVE_LIBKEXIF
	#if LIBKEXIF_VERSION < 020
	#warning LIBKEXIF_VERSION < 020
	#include <libkexif/kexif.h>
	#else
	#warning LIBKEXIF_VERSION >= 020
	#include <libkexif/kexifdialog.h>
	#endif
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
#endif /* HAVE_LIBKEXIF */

// QT
#include <qcanvas.h>
#include <qimage.h>
#include <qpainter.h>
#include <qmovie.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qdatetime.h>

#define ZOOM_MAX 150.0

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

int min(int a, int b)
{
	return a<b?a:b;
}
int max(int a, int b)
{
	return a>b?a:b;
}

static int nbrMU;

ImageViewer::ImageViewer (
			QWidget * parent,
			MainWindow *mw,
			const QString& name,
			int wFlags)
	:QWidget (parent, name.ascii(), WResizeNoErase|WRepaintNoErase|WPaintClever|wFlags),

	mw(NULL),
	il(NULL),

	filename(0),
	image(NULL),
	imageScaled(NULL),
	imageIndex(-1),
	preloadedImage(NULL),
	preloadedScaledImage(NULL),

	m_popup(NULL),


	sp(NULL),
	ep(NULL),

	scale(1),
	ss(true),
	button(NoButton),
	movie(NULL),
	fit(false),
	isFitWidth(false),
	isFitHeight(false),
	shrink(false),
	enlarge(false),
	lock(false),
	realPosX(-1),
	realPosY(-1),
	dragStartPosX(-1),
	dragStartPosY(-1),
	topPosX(0),
	topPosY(0),
	difTopPosX(-1),
	difTopPosY(-1),
	isScrolling_(false),
	hasimage(false),

	total(0),
	nbImg(0),



	aScrollXR(NULL)
{
	setMainWindow(mw);
	setToGrayscale(-1);
	//
	pIO=new KPixmapIO();
	pbgxpm = new QPixmap(locate("appdata", "pics/bgxpm.png"));
	m_kKocale = KGlobal::locale();
	//
	setFocusPolicy (WheelFocus);
	setBackgroundMode(NoBackground);
	//
	KImageIO::registerFormats();
#ifndef Q_WS_WIN //TODO
	kimgio_magick_register(this);
#endif
}

ImageViewer::~ImageViewer()
{
}


void
ImageViewer::setMainWindow(MainWindow *mw)
{
	this->mw = mw;
	this->il = mw?mw->getImageListView():NULL;
}


void
ImageViewer::initActions(KActionCollection *actionCollection)
{
	if(aScrollXR) {kdWarning()<<" initActions already done!"<<endl; return;}
	KShortcut sc_zi(KStdAccel::shortcut(KStdAccel::ZoomIn)); sc_zi.append(KKeySequence((const KKey&)Qt::Key_Plus));
	KShortcut sc_zo(KStdAccel::shortcut(KStdAccel::ZoomOut)); sc_zo.append(KKeySequence((const KKey&)Qt::Key_Minus));

	/////////////
	aScrollXR	=new KAction(i18n("Scroll on the Right"), Key_Right, this, SLOT(scrolldxR()), actionCollection, "ScrollXR");
	aScrollYB	=new KAction(i18n("Scroll at the Bottom"),Key_Down , this, SLOT(scrolldyB()), actionCollection, "ScrollYB");
	aScrollXL	=new KAction(i18n("Scroll on the Left"), Key_Left, this, SLOT(scrolldxL()), actionCollection, "ScrollXL");
	aScrollYT	=new KAction(i18n("Scroll on the Top"), Key_Up, this, SLOT(scrolldyT()), actionCollection, "ScrollYT");

	/////////////
	aZoomIn		=new KAction(i18n("Zoom In"), "viewmag_bis+", sc_zi, this, SLOT(slotZoomIn()), actionCollection, "Zoom in");
	aZoomOut	=new KAction(i18n("Zoom Out"), "viewmag_bis-", sc_zo, this, SLOT(slotZoomOut()), actionCollection, "Zoom out");
	aZoomFit	=new KAction(i18n("Fit to Screen"), "viewmag_full", KShortcut(Key_Slash), this, SLOT(slotZoom()), actionCollection, "Fit to Screen");
	aZoomFitWidth=new KToggleAction(i18n("Fit Width"), "viewmag_w", 0, this, SLOT(slotfitWidth()), actionCollection, "Fit the width");
	aZoomFitHeight=new KToggleAction(i18n("Fit Height"), "viewmag_h", 0, this, SLOT(slotfitHeight()), actionCollection, "Fit the height");
	aZoomNo		=new KAction(i18n("Original Size"), "viewmag_no", KShortcut(Key_Asterisk), this, SLOT(slotZoomNo()), actionCollection, "Originale size");
	aZoomLock	=new KToggleAction(i18n("Lock Zoom"), "viewmag_lock", 0, this, SLOT(slotZoomLock()), actionCollection, "ZoomLock");
	aEnlarge	=new KToggleAction(i18n("Enlarge if Smaller"), "viewmag_enlarge", 0, this, SLOT(slotEnlarge()), actionCollection, "Enlarge");
	aShrink		=new KToggleAction(i18n("Shrink if Bigger"), "viewmag_shrink", 0, this, SLOT(slotShrink()), actionCollection, "Shrink");
	KActionMenu *actionZoom = new KActionMenu( i18n("Zoom"), "viewmag", actionCollection, "view_zoomm" );
	actionZoom->insert(aZoomIn);
	actionZoom->insert(aZoomOut);
	actionZoom->insert(aZoomFit);
	actionZoom->insert(aZoomFitWidth);
	actionZoom->insert(aZoomFitHeight);
	actionZoom->insert(aZoomNo);
	actionZoom->insert(aZoomLock);
	actionZoom->insert(new KActionSeparator());
	actionZoom->insert(aEnlarge);
	actionZoom->insert(aShrink);

#ifndef HAVE_KIPI
	/////////////
	aWallpaper_CENTER	=new KAction(i18n("Centered"), 0, this, SLOT(wallpaperC()), actionCollection, "Center");
	aWallpaper_MOSAIC	=new KAction(i18n("Tiled"), 0, this, SLOT(wallpaperM()), actionCollection, "Mosaic");
	aWallpaper_CENTER_MOSAIC=new KAction(i18n("Center Tiled"), 0, this, SLOT(wallpaperCM()), actionCollection, "Center adapt");
	aWallpaper_CENTER_MAX	=new KAction(i18n("Centered Maxpect"), 0, this, SLOT(wallpaperCMa()), actionCollection, "Center max");
	aWallpaper_ADAPT	=new KAction(i18n("Scaled"), 0, this, SLOT(wallpaperA()), actionCollection, "Adapt");
	aWallpaper_LOGO		=new KAction(i18n("Logo"), 0, this, SLOT(wallpaperL()), actionCollection, "Logo");
#else
	aWallpaper_CENTER=aWallpaper_MOSAIC=aWallpaper_CENTER_MOSAIC=aWallpaper_CENTER_MAX=aWallpaper_ADAPT=aWallpaper_LOGO=NULL;
#endif

/////////////
	aSmooth		=new KToggleAction(i18n("Smooth Scaling"), 0, this, SLOT(slotSmooth()), actionCollection, "Smooth scaling");

	/////////////
	aEffect_GRAYSCALE=new KToggleAction(i18n("Gray Scale"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_grayscale");
	aEffect_NORMALIZE=new KToggleAction(i18n("Normalize"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_normalize");
	aEffect_EQUALIZE=new KToggleAction(i18n("Equalize"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Equalize");
	aEffect_INTENSITY=new KToggleAction(i18n("Intensity"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Intensity");
	aEffect_INVERT=new KToggleAction(i18n("Invert"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Invert");
	aEffect_EMBOSS=new KToggleAction(i18n("Emboss"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Emboss");
	aEffect_SWIRL=new KToggleAction(i18n("Swirl"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Swirl");
	aEffect_SPREAD=new KToggleAction(i18n("Spread"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Spread");
	aEffect_IMPLODE=new KToggleAction(i18n("Implode"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Implode");
	aEffect_CHARCOAL=new KToggleAction(i18n("Charcoal"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_Chaorcoal");
	aEffect_NONE=new KToggleAction(i18n("None"), 0, this, SLOT(slotSetFilter()), actionCollection, "effect_effect_none");
	KActionMenu *actionEffects = new KActionMenu( i18n("Effects"), "kcoloredit", actionCollection, "view_effects" );
	actionEffects->insert(aEffect_NONE);
	actionEffects->insert(new KActionSeparator());
	actionEffects->insert(aEffect_GRAYSCALE);
	actionEffects->insert(aEffect_NORMALIZE);
	actionEffects->insert(aEffect_EQUALIZE);
	actionEffects->insert(aEffect_INTENSITY);
	actionEffects->insert(aEffect_INVERT);
	actionEffects->insert(new KActionSeparator());
	actionEffects->insert(aEffect_EMBOSS);
	actionEffects->insert(aEffect_SWIRL);
	actionEffects->insert(aEffect_SPREAD);
	actionEffects->insert(aEffect_IMPLODE);
	actionEffects->insert(aEffect_CHARCOAL);

	/////////////
	aRotLeft	=new KAction(i18n("Rotate Left"), "rotation_acw", KShortcut(Key_L), this, SLOT(slotRotateLeft()), actionCollection, "Rotate Left");
	aRotRight	=new KAction(i18n("Rotate Right"), "rotation_cw", KShortcut(Key_R), this, SLOT(slotRotateRight()), actionCollection, "Rotate Right");
	aHMirror	=new KAction(i18n("Vertical Flip"), "flip", 0, this, SLOT(slotMirrorH()), actionCollection, "Flip");
	aVMirror	=new KAction(i18n("Horizontal Flip"), "miror", 0, this, SLOT(slotMirrorV()), actionCollection, "Mirror");
	KActionMenu *actionOrientation = new KActionMenu( i18n("Orientation"), "rotation_acw", actionCollection, "view_Orientation" );
		actionOrientation->insert(aRotLeft);
		actionOrientation->insert(aRotRight);
		actionOrientation->insert(aHMirror);
		actionOrientation->insert(aVMirror);

	///////////
	aPrint		=new KAction(i18n("Print..."), "fileprint", KStdAccel::shortcut(KStdAccel::Print), this, SLOT(slotPrint()), actionCollection, "fileprint");

	///////////
	#ifdef HAVE_LIBKEXIF
	aDisplayExifDialog = new KAction(i18n("Exif Information"), 0, this, SLOT(slotDisplayExifDialog()), actionCollection, "display_Exif_Dialog");
	#else
	#ifdef __GNUC__
	#warning no HAVE_LIBKEXIF
	#endif
	aDisplayExifDialog = NULL;
	#endif /* HAVE_LIBKEXIF */

	///////////
	aSaveImage	=new KAction(i18n("Save &As..."), "filesave", KStdAccel::shortcut(KStdAccel::Save), this, SLOT(slotSaveImage()), actionCollection, "filesave" );

	///////////
	aPrevious	=new KAction(i18n("Previous Image"), "1leftarrow", Key_PageUp, this, SLOT(previous()), actionCollection, "Previous Image" );
	aNext		=new KAction(i18n("Next Image"), "1rightarrow", Key_PageDown, this, SLOT(next()), actionCollection, "Next Image");
	aFirst		=new KAction(i18n("First Image"), "top", KStdAccel::shortcut(KStdAccel::Home), this, SLOT(first()), actionCollection, "First Image" );
	aLast		=new KAction(i18n("Last Image"), "bottom", KStdAccel::shortcut(KStdAccel::End), this, SLOT(last()),actionCollection, "Last Image" );

	if(actionCollection->action("action go"))
	{
		((KActionMenu*)actionCollection->action("action go"))->insert(aFirst);
		((KActionMenu*)actionCollection->action("action go"))->insert(aLast);
		((KActionMenu*)actionCollection->action("action go"))->insert(aPrevious);
		((KActionMenu*)actionCollection->action("action go"))->insert(aNext);
	}
}

void
ImageViewer::initMenu(KActionCollection *actionCollection)
{
	if(m_popup)
		m_popup->clear();
	else
		m_popup = new KPopupMenu();

	if(!mw)
	{
		m_popup->insertTitle(i18n("ShowImg Preview"), 1);

		actionCollection->action("action go")->plug(m_popup);
		actionCollection->action("view_zoomm")->plug(m_popup);
		actionCollection->action("view_effects")->plug(m_popup);
		actionCollection->action("view_Orientation")->plug(m_popup);
		if(aDisplayExifDialog) actionCollection->action("display_Exif_Dialog")->plug(m_popup);
		KActionSeparator *sep = new KActionSeparator(); sep->plug(m_popup);
		actionCollection->action("filesave")->plug(m_popup);
		actionCollection->action("fileprint")->plug(m_popup);
	}
	else
	{
#ifndef HAVE_KIPI
		KActionMenu *actionWallp = new KActionMenu( i18n("Set as Wallpaper"), "desktop", actionCollection, "view_wallp" );
		actionWallp->insert(aWallpaper_CENTER);
		actionWallp->insert(aWallpaper_MOSAIC);
		actionWallp->insert(aWallpaper_CENTER_MOSAIC);
		actionWallp->insert(aWallpaper_CENTER_MAX);
		actionWallp->insert(aWallpaper_ADAPT);
		actionWallp->insert(aWallpaper_LOGO);
#endif /* HAVE_KIPI */

		actionCollection->action("FullScreen")->plug(m_popup);

		m_popup->insertSeparator();

		actionCollection->action("view_zoomm")->plug(m_popup);
		actionCollection->action("view_Orientation")->plug(m_popup);
		actionCollection->action("view_effects")->plug(m_popup);

		actionCollection->action("action go")->plug(m_popup);

		m_popup->insertSeparator();

		m_popup->insertSeparator();
		actionCollection->action("filesave")->plug(m_popup);
		actionCollection->action("editcopy")->plug(m_popup);

		m_popup->insertSeparator();
		actionCollection->action("edittrash")->plug(m_popup);
		actionCollection->action("editdelete")->plug(m_popup);

		m_popup->insertSeparator();
		actionCollection->action("Image Info")->plug(m_popup);
		if(aDisplayExifDialog)  actionCollection->action("display_Exif_Dialog")->plug(m_popup);
		actionCollection->action("Properties")->plug(m_popup);
	}
}


bool
ImageViewer::preloadImage (const QString& fileName)
{
	QString _fileName_;
	if(!ListItemView::isImage(fileName))
		_fileName_=QString();
	else
		_fileName_=fileName;

	const QFile::Offset big = 0x501400; // 5MB
	QString imageFormat = QImageIO::imageFormat(_fileName_);
	if( (QFile(_fileName_).size() > big) ||
		( imageFormat == "GIF"))
	{
		kdWarning()<<"ImageViewer::preloadImage (QString fileName): "
			<<"image's too big or is GIF"
			<<endl;
		_fileName_=QString();
	}
	else
	{
	}
	preimageName=_fileName_;
	delete(preloadedImage); preloadedImage=new QImage();
	if(!preloadedImage->load(_fileName_))
	{
		delete(preloadedImage); preloadedImage=NULL;
		delete(preloadedScaledImage);preloadedScaledImage=NULL;

		return false;
	}
	scalePreload();
	return true;
}


void
ImageViewer::reload()
{
	loadImage(filename);
}


bool
ImageViewer::loadImage (const QString& fileName, int index)
{
	QString _fileName_=fileName;
	if(!ListItemView::isImage(_fileName_))
		_fileName_=QString();

	bool ok = false;
	if (!_fileName_.isEmpty())
	{
		imageIndex=index;
		FileIconItem *item =
				NULL;
			(il
			?il->findItem(QFileInfo(_fileName_).filePath(), true)
			 :
			 NULL);
		#ifdef HAVE_LIBKEXIF
		#if KDE_VERSION < 0x30200
		aDisplayExifDialog->setEnabled(KMimeType::findByPath(_fileName_, 0, true)->name() == "image/jpeg");
		#else
		aDisplayExifDialog->setEnabled(KMimeType::findByPath(_fileName_, 0, true)->is("image/jpeg"));
		#endif /* KDE_VERSION < 0x30200 */
		#endif /* HAVE_LIBKEXIF */

		if(item)
		{

			if(!item->isSelected())
			{
				delete (preloadedImage);
				preloadedImage=NULL;
				delete (imageScaled);imageScaled=NULL;
				return false;
			}
		}
// 		else if(il)
// 		{
// 			delete (preloadedImage);preloadedImage=NULL;
// 			delete (imageScaled);imageScaled=NULL;
// 			return false;
// 		}

		setMessage(i18n("Loading image..."));
		KApplication::setOverrideCursor( waitCursor ); // this might take time

		filename = _fileName_;
		nbImg++;
		if (!_fileName_.compare(preimageName) && preloadedImage)
		{
			delete (image);
			image = new QImage(*preloadedImage);
			delete(preloadedImage);
			preloadedImage=NULL;
			delete(imageScaled);
			if(preloadedScaledImage && image)
			{
				imageScaled=preloadedScaledImage;
				reconvertImage();
				preloadedScaledImage=NULL;
				ok=true;
			}
			else
			{
				ok=false;
			}
		}
		if(!ok)
		{
			delete(image); image = new QImage ();
			delete (preloadedImage);preloadedImage=NULL;
			delete (imageScaled);imageScaled=NULL;

			ok=image->load(filename);
			reconvertImage();
		}
	}
	if(ok)
	{
		if(movie)
		{
			movie->disconnectUpdate(this);
			movie->disconnectStatus(this);
			movie->pause();
		}
		if(useEXIF()) autoRotate(false);
		applyFilter();
		doScale(false);

		imageName=_fileName_;
		setZoom(scale);
		//
		imageType=QImageIO::imageFormat(filename);
		if( (imageType == "MNG")  || (imageType == "GIF")   )
		{
			repaint();
			startMovie();
		}
		else
		{
			movie=NULL;
		}
	}
	else
	{
//		filename=strdup("(none)");
		filename="(none)";
		delete(movie);movie=NULL;
		delete(image);image=NULL;
		delete (imageScaled);imageScaled=NULL;
		delete (preloadedImage);preloadedImage=NULL;
		delete (preloadedScaledImage);preloadedScaledImage=NULL;
	}
	updateStatus ();
	hasimage = image!=NULL;

#ifdef WANT_DIGIKAMIMAGEEFFECT
// FIXME try to use Digikam::ImageIface
	if(hasimage)
	{
		MYDEBUG<<endl;
		QImage *img = image;
		int w=0,h=0;
		uint* data=NULL;
		if(img)
		{
			MYDEBUG<<endl;
		    data = (uint *)img->bits();
		    w    = img->width();
    			h    = img->height();
			MYDEBUG<<endl;
		}
		if(data)
		{
			MYDEBUG<<endl;
			Digikam::ImageIface m_ImageIface(0,0);
			MYDEBUG<<endl;
			m_ImageIface.putOriginalData("montest",data,w,h);
			MYDEBUG<<endl;
		}
		MYDEBUG<<endl;
	}
#endif






	setMessage(i18n("Ready"));
	if(!movie)repaint();
	KApplication::restoreOverrideCursor(); // restore original cursor

	//
	KURL imageurl;
	imageurl.setPath(filename);
	emit loaded(imageurl);

	//
	return ok;
}

bool
ImageViewer::hasImage()
{
	return hasimage;
}


bool
ImageViewer::reconvertImage()
{
	if (!image)
		return false;
	if(image->hasAlphaBuffer())
	{
		QPixmap pix(image->size());
		QPainter p;
			p.begin(&pix);
			p.drawTiledPixmap (0, 0, image->width(), image->height(), *pbgxpm);
			p.drawImage(0, 0,*image);
			p.end();
		*image=pix.convertToImage();
	}
	return true;
}


bool
ImageViewer::smooth () const
{
	return ss;
}


void
ImageViewer::scalePreload ()
{
	if(
		(aEffect_NORMALIZE->isChecked() || aEffect_EQUALIZE->isChecked() || aEffect_INTENSITY->isChecked() || aEffect_INVERT->isChecked() || aEffect_EMBOSS->isChecked() || aEffect_SWIRL->isChecked() || aEffect_SPREAD->isChecked() || aEffect_IMPLODE->isChecked() || aEffect_CHARCOAL->isChecked() || aEffect_GRAYSCALE->isChecked())
		||
		(preloadedImage && preloadedImage->hasAlphaBuffer())
	)
	{
		delete(preloadedScaledImage); preloadedScaledImage=NULL;
		delete(preloadedImage); preloadedImage=NULL;
		delete(imageScaled); imageScaled=NULL;

		return;
	}
	float s;
	if ((((double)height()) / preloadedImage->height ()) < (((double) width ()) / preloadedImage->width ()))
		s = ((double)height()) / preloadedImage->height ();
	else
		s = ((double) width ()) / preloadedImage->width ();
	if(!lock)
	{
		if( (s>1 && enlarge) ||  (s<1 && shrink) )
		{
			//
		}
		else
		{
			s=1;
		}
	}
	else
		s=scale;

	QPoint rtp(0, 0);
	QPoint rbp((int)ceil(width()/s),(int)ceil(height()/s));
	QRect redraw(rtp,rbp);
	int
		cx=0,
		cy=0,
		cw=min(preloadedImage->width(),  redraw.width()),
		ch=min(preloadedImage->height(), redraw.height());

	int options=ColorOnly||ThresholdDither||ThresholdAlphaDither||AvoidDither;
	delete(preloadedScaledImage); preloadedScaledImage=new QImage();
	*preloadedScaledImage
		=preloadedImage->copy(cx, cy,
					cw, ch,
					options)
				.smoothScale((int)ceil(cw*s), (int)ceil(ch*s));
}


void
ImageViewer::startMovie()
{
	delete(movie); movie=NULL;
	if (!filename.isEmpty()) // Start a new movie - have it delete when closed.
		initMovie();
}


void
ImageViewer::initMovie()
{
	movie = new QMovie(filename);
	nbrMU=-1;
	QPixmap pix(image->width(), image->height()); pix.fill(bgBrush.color()); *image=pIO->convertToImage(pix);//repaint();
	movie->setBackgroundColor(bgBrush.color());
	movie->connectUpdate(this, SLOT(movieUpdated(const QRect&)));
	movie->connectStatus(this, SLOT(movieStatus(int)));
}

void
ImageViewer::movieUpdated(const QRect& )
{
	if(! movie)
		return;
	nbrMU++;
	if(nbrMU>movie->frameNumber()) /* the gif is not animated */
	{
		movie->disconnectUpdate(this);
		movie->disconnectStatus(this);
		movie->pause();
		movie=0L;
		delete(movie); movie=NULL;
		delete(image); image=new QImage(filename);
		delete (imageScaled);imageScaled=NULL;
		reconvertImage();
		applyFilter();
		doScale(false);
		return;
	}
	else
	{
		*image = pIO->convertToImage(movie->framePixmap());
		if(nbrMU!=0)repaint();
	}
}


void
ImageViewer::movieStatus(int status)
{
	if (movie && status<0)
		KMessageBox::error(this, i18n("Could not play movie \"%1\"").arg(filename));
	if(status==3) nbrMU=-1;
}


void
ImageViewer::mousePressEvent (QMouseEvent * e)
{
	QWidget::mousePressEvent(e);

	button = e->button ();
	if (button==RightButton)
	{
		if(il)
		{
			m_popup->removeItemAt(7);
			m_popup_openWith=il->popupOpenWith();
			m_popup->insertItem(i18n("Open With"), m_popup_openWith, -1, 7);
			il->setSelected(il->currentItem(), true, false);
		}
		if(m_popup)
		{
			m_popup->exec(e->globalPos());
		}
	}
	else
	if (button==LeftButton)
	{
		if(!image) return;
		KApplication::setOverrideCursor (sizeAllCursor);
		dragStartPosX=e->pos().x();
		dragStartPosY=e->pos().y();
		difTopPosX = getVirtualPosX()-dragStartPosX;
		difTopPosY = getVirtualPosY()-dragStartPosY;
	}
	else
	{
		delete(sp); sp = new QPoint(e->pos()) ;
		ep = new QPoint(*sp);
	}
}


void
ImageViewer::mouseReleaseEvent (QMouseEvent * e)
{
	if (e->button()==LeftButton)
	{
		if(!image) return;
		KApplication::restoreOverrideCursor ();
		QWidget::mouseReleaseEvent(e);
		dragStartPosX=-1;
		dragStartPosY=-1;
		repaint();
	}
	else
	if (e->button()==RightButton)
	{
	}
	else
	if(e->button()!=MidButton)
	{
	}
	else
	if(image)
	{
		delete(ep); ep = new QPoint(e->pos()) ;
		if(*sp==*ep && !isScrolling())
		{
			ep=NULL; lp=NULL;
			KApplication::setOverrideCursor (waitCursor);	// this might take time
			doScale(false);placeImage(false);repaint();
			KApplication::restoreOverrideCursor ();	// restore original cursor
			button=NoButton;
			return;
		}
		else if(!isScrolling())
		{
			QRect rectDraged= QRect(
					QPoint(
						max(min(sp->x(),ep->x()), getVirtualPosX()),
						max(min(sp->y(),ep->y()), getVirtualPosY())),
					QPoint(
						min(max(sp->x(),ep->x()), getVirtualPosX()+virtualPictureWidth()),
						min(max(sp->y(),ep->y()), getVirtualPosY()+virtualPictureHeight())));

			QPoint oldCenter=QPoint( (int)(((rectDraged.left()+rectDraged.right())/2 - getVirtualPosX())/scale),
						 (int)(((rectDraged.top()+rectDraged.bottom())/2 - getVirtualPosY())/scale));

			rectDraged.moveBy(-getVirtualPosX(), -getVirtualPosY());
			float
				sx=width()/rectDraged.width(),
				sy=height()/rectDraged.height();
			if(scale*((float)sx+(float)sy)/2 > ZOOM_MAX)
				scale=ZOOM_MAX;
			else
				scale*=((float)sx+(float)sy)/2;

			setZoom(scale);
			oldCenter*=scale;
			ep=NULL; lp=NULL;

			centerImage(oldCenter.x(), oldCenter.y(), true);
		}
		ep=NULL; lp=NULL;
		KApplication::restoreOverrideCursor ();
	}
	ep=NULL; lp=NULL;
	button=NoButton;
	isScrolling_=false;
}


void
ImageViewer::mouseMoveEvent(QMouseEvent * e)
{
	if (button==LeftButton && !isScrolling() )
	{
		QWidget::mouseMoveEvent(e);
		if ( (dragStartPosX+dragStartPosY!=-2.0))
		{
			double
				diffX=e->pos().x()-dragStartPosX,
				diffY=e->pos().y()-dragStartPosY,
				panX=0, panY=0;

			if (virtualPictureWidth()>width())
			{
				if(fabs(diffX)>=scale)
				{
					panX=(int)diffX;
					dragStartPosX+=panX;
					if(!posXForTopXIsOK(difTopPosX+dragStartPosX))
					{
						if( difTopPosX+dragStartPosX>=0 )
						{
							dragStartPosX -= panX;
							panX = - getVirtualPosX();
						}
						else
						{
							dragStartPosX -= panX;
							panX = - (virtualPictureWidth() + getVirtualPosX() - width() );
						}
						dragStartPosX+= panX;
					}
				}
			}
			if (virtualPictureHeight()>height())
			{
				if(fabs(diffY)>=scale)
				{
					panY=diffY;
					dragStartPosY+=panY;
					if(!posYForTopYIsOK(difTopPosY+dragStartPosY))
					{
						if( difTopPosY+dragStartPosY>=0 )
						{
							dragStartPosY -= panY;
							panY = - getVirtualPosY();
						}
						else
						{
							dragStartPosY -= panY;
							panY = - (virtualPictureHeight() + getVirtualPosY() - height() );
						}
						dragStartPosY+= panY;
					}
				}
			}
			if(panX!=0 || panY!=0)
				scroll((int)panX, (int)panY);
		}
		return;
	}
	else
	if(isScrolling())
		return;
	else
	if(movie || !ep )
		return;

	QPainter p(this); p.setPen(QColor("black"));
	lp = new QPoint(*ep);
	ep = new QPoint(e->pos()) ;
	int
		lx = lp->x(),
		ly = lp->y(),
		ex = ep->x(),
		ey = ep->y(),
		sx = sp->x(),
		sy = sp->y();

	int tlx,tly,brx,bry;

		tlx=(sx<ex?sx:ex);
		tly=(ly<ey?ly:ey);
		brx=(sx>ex?sx:ex);
		bry=(ly>ey?ly:ey);
		repaint(QRect(QPoint(tlx ,tly), QPoint(brx ,bry)));

		tlx=(lx<ex?lx:ex);
		tly=(sy<ey?sy:ey);
		brx=(lx>ex?lx:ex);
		bry=(sy>ey?sy:ey);
		repaint(QRect(QPoint(tlx ,tly), QPoint(brx ,bry)));

		tlx=(lx<ex?lx:ex);
		tly=(ly<ey?ly:ey);
		brx=(lx>ex?lx:ex);
		bry=(ly>ey?ly:ey);
		repaint(QRect(QPoint(tlx ,tly), QPoint(brx ,bry)));

		tlx=(lx<sx?lx:sx);
		tly=(sy<ly?sy:ly);
		brx=(lx>sx?lx:sx);
		bry=(sy>ly?sy:ly);
		repaint(QRect(QPoint(tlx ,tly), QPoint(brx ,bry)));

	p.drawRect(QRect(*sp,*ep));
}

void
ImageViewer::mouseDoubleClickEvent ( QMouseEvent * e )
{
	if(e->button () == LeftButton)
	{
		if(mw) mw->slotFullScreen();
	}
}

bool
ImageViewer::scrollUp ()
{
	return scrolldyT(height()/3);
}

bool
ImageViewer::scrollDown ()
{
	return scrolldyB(height()/3);
}

void
ImageViewer::wheelEvent (QWheelEvent * e)
{
	if(e->state() == Qt::ShiftButton)
	{
		if (e->delta () > 0)
			zoomOut(1.5);
		else
			zoomIn(1.5);
	}
	else
	if(button==QMouseEvent::MidButton && !lp)
	{
		isScrolling_=true;
		if (e->delta () > 0)
			scrollUp ();
		else
			scrollDown ();

	}
	else
	if(button==QMouseEvent::MidButton && lp)
	{
		return ;
	}
	else
	{
		if (e->delta () < 0)
			next ();
		else
			previous ();
	}
}

#define CENTER 1
#define MOSAIC 2
#define CENTER_MOSAIC 3
#define CENTER_MAX 4
#define ADAPT 5
#define LOGO 6

void
ImageViewer::wallpaperC(){wallpaper(CENTER);}
void
ImageViewer::wallpaperM(){wallpaper(MOSAIC);}
void
ImageViewer::wallpaperCM(){wallpaper(CENTER_MOSAIC);}
void
ImageViewer::wallpaperCMa(){wallpaper(CENTER_MAX);}
void
ImageViewer::wallpaperA(){wallpaper(ADAPT);}
void
ImageViewer::wallpaperL(){wallpaper(LOGO);}


void
ImageViewer::wallpaper(int mode)
{
	/*
	1:center
	2:mosaic
	3:center mosaic
	4:center max
	5:adapt
	6:logo
	*/
	if(mode>LOGO)
		return;

	setMessage(i18n("Set as Wallpaper"));
	QString com=QString("dcop kdesktop KBackgroundIface setWallpaper '%1' %2 >/dev/null 2>/dev/null")
				.arg(filename)
				.arg(mode);
	KRun::runCommand(com);
	setMessage(i18n("Ready"));
}



void
ImageViewer::setFit(bool fit, bool keep)
{
	if(keep)
		this->fit=fit;
	delete(imageScaled); imageScaled=NULL;
	delete(preloadedImage); preloadedImage=NULL;
	imageScaled=NULL;if(fit)
		fitSize();
	else
		doScale(true);
}

void
ImageViewer::setSmooth(bool s)
{
	ss=s;
	doScale(true);
}


bool ImageViewer::autoRotate(bool r)
{
	KFileMetaInfo metadatas((QString)filename );
	if ( !metadatas.isValid() ) return false;
	KFileMetaInfoItem metaitem = metadatas.item("Orientation");
	if ( !metaitem.isValid() || metaitem.value().isNull()) return false;
        //  Orientation:
        //  1:      normal
        //  2:      flipped horizontally
        //  3:      ROT 180
        //  4:      flipped vertically
        //  5:      ROT 90 -> flip horizontally
        //  6:      ROT 90
        //  7:      ROT 90 -> flip vertically
        //  8:      ROT 270
	switch ( metaitem.value().toInt() )
	{
		case 1:
		default:
			break;
		case 2:
			mirror(true, false, r);
			break;
		case 3:
			mirror(true, true, r);
			break;
		case 4:
			mirror(false, true, r);
			break;
		case 5:
			rotateLeft(r);
			mirror(true, false, r);
			break;
		case 6:
			rotateRight(r);
			break;
		case 7:
			rotateRight(r);
			mirror(false, true, r);
			break;
		case 8:
			rotateLeft(r);
		break;
    }
    return true;
}


void
ImageViewer::scroll( int dx, int dy )
{
	setVirtualPosX(getVirtualPosX()+dx);
	setVirtualPosY(getVirtualPosY()+dy);
	QWidget::scroll(dx,dy);
}


void
ImageViewer::mirror (bool horizontal, bool vertical, bool r)
{
	if(!image) return;
	KApplication::setOverrideCursor (waitCursor);	// this might take time

	QWMatrix matrix;
	if(vertical)
		matrix.scale(1,-1);
	else
	if(horizontal)
		matrix.scale(-1,1);
	*image=image->xForm(matrix);
	delete(imageScaled); imageScaled=NULL;

	centerImage(false);
	placeImage(r);

	KApplication::restoreOverrideCursor ();	// restore original cursor
}


void
ImageViewer::rotateLeft(bool r)
{
	if(!image) return;
	KApplication::setOverrideCursor (waitCursor);	// this might take time
	QWMatrix matrix;
	matrix.rotate(-90);
	*image=image->xForm(matrix);

	delete(imageScaled); imageScaled=NULL;

	centerImage(false);
	placeImage(r);

	KApplication::restoreOverrideCursor ();	// restore original cursor
}


void
ImageViewer::rotateRight(bool r)
{
	if(!image) return;
	KApplication::setOverrideCursor (waitCursor);	// this might take time

	QWMatrix matrix;
	matrix.rotate(90);
	*image=image->xForm(matrix);

	delete(imageScaled); imageScaled=NULL;

	centerImage(false);
	placeImage(r);

	KApplication::restoreOverrideCursor ();	// restore original cursor
}


void
ImageViewer::zoomIn(float rate)
{
	if(scale<ZOOM_MAX)
	{
		KApplication::setOverrideCursor (waitCursor);	// this might take time

		QPoint center(width()/2, height()/2);
		center/=scale;
		center+=QPoint(getPosX(), getPosY());
		if(scale*rate>ZOOM_MAX)
			scale=ZOOM_MAX;
		else
			scale*=rate;
		centerImage((int)(center.x()*scale), (int)(center.y()*scale), true);

		KApplication::restoreOverrideCursor ();	// restore original cursor
		setZoom(scale);

		delete(imageScaled); imageScaled=NULL;
	}
}


void
ImageViewer::zoomOut(float rate)
{
	if(scale>1.0/ZOOM_MAX)
	{
		KApplication::setOverrideCursor (waitCursor);	// this might take time
		QPoint center(width()/2, height()/2);
		center/=scale;
		center+=QPoint(getPosX(), getPosY());
		if(scale/rate<=1.0/ZOOM_MAX)
			scale=float(1.0/ZOOM_MAX);
		else
			scale/=rate;
		centerImage((int)(center.x()*scale), (int)(center.y()*scale), true);
		KApplication::restoreOverrideCursor ();	// restore original cursor
		setZoom(scale);

		delete(imageScaled); imageScaled=NULL;
	}
}

void
ImageViewer::setZoomValue(float val)
{
	if(val>1.0/ZOOM_MAX && val<ZOOM_MAX)
	{
		KApplication::setOverrideCursor (waitCursor);	// this might take time
		QPoint center(width()/2, height()/2);
		center/=scale;
		center+=QPoint(getPosX(), getPosY());
		scale=val;
		centerImage((int)(center.x()*scale), (int)(center.y()*scale), true);
		KApplication::restoreOverrideCursor ();	// restore original cursor
		setZoom(scale);

		delete(imageScaled); imageScaled=NULL;
	}
}


void
ImageViewer::updateStatus ()
{
	if(!mw) return;
	if (!image || (image && image->size() == QSize(0, 0)))
	{
		mw->setZoom((int)(scale*100));
		mw->setImagename(NULL);
		mw->setImagetype(NULL);
		mw->setDim(QSize(0, 0));
		mw->setSize(-1);
		mw->setDate(NULL);
		mw->setImageIndex(-1);
	}
	else
	{
		if (filename!=QString("(none)"))
		{
			mw->setZoom((int)(scale*100));
			//
			QString *fileName=new QString(filename);
			int pos = fileName->findRev ("/");
			mw->setImagename(fileName->right (fileName->length () - pos-1));
			//
			pos = fileName->findRev(".");
			mw->setImagetype(imageType);
			//
			mw->setDim(image->size(), (float)image->dotsPerMeterX ()/1000*25.4) ;
			//
			QFileInfo fi(filename);
			mw->setSize(fi.size());
			//
			if(useEXIF() && imageType=="JPEG")
			{
				KFileMetaInfo meta( filename );
				QString mDate="---";
				if( meta.isValid() )
					mDate = meta.item( "Date/time" ).string( true ).stripWhiteSpace();
				if(mDate!="---")
				{
					QDateTime *mDateTime = new QDateTime(
						m_kKocale->readDate(meta.item( "CreationDate" ).string( true ).stripWhiteSpace()),
						m_kKocale->readTime(meta.item( "CreationTime" ).string( true ).stripWhiteSpace()));
					mw->setDate(mDateTime);
				}
				else
					mw->setDate(new QDateTime(fi.lastModified()));
			}
			else
			{
				mw->setDate(new QDateTime(fi.lastModified()));
			}
			mw->setImageIndex(imageIndex);
		}
		else
		{
			mw->setZoom((int)(scale*100));
			mw->setImagename("(none)");
			mw->setImagetype("");
			mw->setDim(QSize(0, 0));
			mw->setSize(0);
			mw->setDate(NULL);
		}
	}
}

int
ImageViewer::virtualScreenWidth()
{
	return (int)ceil(width()/scale);
}
int
ImageViewer::virtualScreenHeight()
{
	return (int)ceil(height()/scale);
}

int
ImageViewer::virtualPictureWidth()
{
	return (image!=NULL)?(int)ceil(image->width()*scale):0;
}
int
ImageViewer::virtualPictureHeight()
{
	return (image!=NULL)?(int)ceil(image->height()*scale):0;
}

void
ImageViewer::setVirtualPosX(double posX)
{
	topPosX=posX;
	realPosX=-(int)ceil(posX/scale);
}
void
ImageViewer::setVirtualPosY(double posY)
{
	topPosY=posY;
	realPosY=-(int)ceil(posY/scale);
}
int
ImageViewer::getVirtualPosX()
{
	return (int)topPosX;
}
int
ImageViewer::getVirtualPosY()
{
	return (int)topPosY;
}
QPoint ImageViewer::getVirtualPos()
{
	return QPoint(getVirtualPosX(), getVirtualPosY());
}
void
ImageViewer::setVirtualPos(const QPoint pos)
{
	setVirtualPosX(pos.x());setVirtualPosY(pos.y());
}

void
ImageViewer::setPosX(double posX)
{
	realPosX= (int)((posX/fabs(posX)) * ceil(fabs(posX)));
}
void
ImageViewer::setPosY(double posY)
{
	realPosY= (int)((posY/fabs(posY)) * ceil(fabs(posY)));
}
void
ImageViewer::setPos(const QPoint pos)
{
	setPosX(pos.x());
	setPosY(pos.y());
}
int
ImageViewer::getPosX()
{
	return (int)ceil(realPosX);
}
int
ImageViewer::getPosY()
{
	return (int)ceil(realPosY);
}
QPoint
ImageViewer::getPos()
{
	return QPoint(getPosX(), getPosY());
}

void
ImageViewer::placeImage(bool redraw)
{
	if(!posXForTopXIsOK(getVirtualPosX()))
		setVirtualPosX(0);
	if(virtualPictureWidth()<=width())
		centerXImage();
	//
	if(!posYForTopYIsOK(getVirtualPosY()))
		setVirtualPosY(0);
	if(virtualPictureHeight()<=height())
		centerYImage();
	//
	if(redraw )
		repaint();
}

void
ImageViewer::placeImage(ImagePosition pos, bool redraw)
{
	switch(pos)
	{
		case ImageViewer::Centered :
		{
			centerImage(redraw);
			break;
		}
		case ImageViewer::TopLeft :
		{
			if(width()-virtualPictureWidth()<0)
				setVirtualPosX(0);
			else
				centerXImage();
			if(height()-virtualPictureHeight()<0)
				setVirtualPosY(0);
			else
				centerYImage();
			break;
		}
		case ImageViewer::TopCentered :
		{
			centerXImage();
			if(height()-virtualPictureHeight()<0)
				setVirtualPosY(0);
			else
				centerYImage();
			break;
		}
		case ImageViewer::TopRight:
		{
			if(width()-virtualPictureWidth()<0)
				setVirtualPosX(width()-virtualPictureWidth());
			else
				centerXImage();
			if(height()-virtualPictureHeight()<0)
				setVirtualPosY(0);
			else
				centerYImage();
			break;
		}
		case ImageViewer::LeftCentered :
		{
			if(width()-virtualPictureWidth()<0)
				setVirtualPosX(0);
			else
				centerXImage();
			centerYImage();
			break;
		}
		case ImageViewer::RightCentered :
		{
			if(width()-virtualPictureWidth()<0)
				setVirtualPosX(width()-virtualPictureWidth());
			else
				centerXImage();
			centerYImage();
			break;
		}
		case ImageViewer::BottomLeft :
		{
			if(width()-virtualPictureWidth()<0)
				setVirtualPosX(0);
			else
				centerXImage();
			if(height()-virtualPictureHeight()<0)
				setVirtualPosY(height()-virtualPictureHeight());
			else
				centerYImage();
			break;
		}
		case ImageViewer::BottomCentered:
		{
			centerXImage();
			if(height()-virtualPictureHeight()<0)
				setVirtualPosY(height()-virtualPictureHeight());
			else
				centerYImage();
			break;
		}
		case ImageViewer::BottomRight:
		{
			if(width()-virtualPictureWidth()<0)
				setVirtualPosX(width()-virtualPictureWidth());
			else
				centerXImage();
			if(height()-virtualPictureHeight()<0)
				setVirtualPosY(height()-virtualPictureHeight());
			else
				centerYImage();
			break;
		}
	}
}


void
ImageViewer::centerImage(int posX, int posY, bool redraw)
{
	int
		oldVPX=getVirtualPosX(),
		oldVPY=getVirtualPosY();

	if (virtualPictureWidth()>width())
	{
		int possibleX = width()/2 - posX;
		if(posXForTopXIsOK(possibleX))
		{
			setVirtualPosX(possibleX);
		}
		else
		{
			posX-=getVirtualPosX();
			if ( (posX>width()/2) &&
				(virtualPictureWidth()+getVirtualPosX()-width()<abs(width()-posX)))
			{
				posX=width()-virtualPictureWidth();
				setVirtualPosX(posX);
			}
			else
			{
				posX=0;
				setVirtualPosX(posX);
			}
		}
	}
	else
		centerXImage();
	//
	if(virtualPictureHeight()>height())
	{
		int possibleY = height()/2 - posY;

		if(posYForTopYIsOK(possibleY))
		{
			setVirtualPosY(possibleY);
		}
		else
		{
			posY-=getVirtualPosY();
			if ((posY>height()/2) &&
				(virtualPictureHeight()+getVirtualPosY()-height()<abs(height()-posY)))
			{
				posY=height()-virtualPictureHeight();
				setVirtualPosY(posY);
			}
			else
			{
				posY=0;
				setVirtualPosY(posY);
			}
		}
	}
	else
		centerYImage();

	if(redraw &&
		((oldVPX!=getVirtualPosX()) ||
		 (oldVPY!=getVirtualPosY())))
	{
		repaint();
	}
}

void
ImageViewer::centerImage(bool redraw)
{
	centerXImage();
	centerYImage();
	if(redraw)
		repaint();
}

void
ImageViewer::centerXImage(bool redraw)
{
	int oldVPX=getVirtualPosX();
	setVirtualPosX((width () - virtualPictureWidth()) / 2);

	if(redraw &&
		oldVPX!=getVirtualPosX())
	{
		repaint();
	}
}

void
ImageViewer::centerYImage(bool redraw)
{
	int oldVPY=getVirtualPosX();
	setVirtualPosY((height() - virtualPictureHeight()) / 2);

	if(redraw &&
		oldVPY!=getVirtualPosY())
	{
		repaint();
	}
}


void
ImageViewer::originalSize()
{
	scale=1;
	placeImage(false);
	setZoom(scale);

	delete(imageScaled); imageScaled=NULL;
	delete(preloadedImage); preloadedImage=NULL;
	repaint();
}


void
ImageViewer::fitSize(bool redraw)
{
	if (!image)
		return;
	if (image->isNull ())
		return;

	float s;
	if ((((double)height()) / image->height ()) < (((double) width ()) / image->width ()))
		s = ((double)height()) / image->height ();
	else
		s = ((double) width ()) / image->width ();

	scale=s;
	placeImage(false);
	setZoom(scale);

	if(redraw)
		repaint();
}

void
ImageViewer::fitWidth(bool setFitWidth, bool redraw)
{
	isFitWidth=setFitWidth;
	isFitHeight=false;
	if(!isFitWidth)return;

	if (!image)
		return;
	if (image->isNull ())
		return;
	scale=((double)width()) / image->width ();
	placeImage(false);
	setZoom(scale);

	delete(imageScaled); imageScaled=NULL;
	delete(preloadedImage); preloadedImage=NULL;
	if(redraw)
		repaint();
}

void
ImageViewer::fitHeight(bool setFitHeight, bool redraw)
{
	isFitHeight=setFitHeight;
	isFitWidth=false;
	if(!isFitHeight)return;

	if (!image)
		return;
	if (image->isNull ())
		return;
	scale=((double)height()) / image->height();
	placeImage(false);
	setZoom(scale);

	delete(imageScaled); imageScaled=NULL;
	delete(preloadedImage); preloadedImage=NULL;
	if(redraw)
		repaint();
}

void
ImageViewer::doScale(bool repaint)
{
	if(!image)return;
	if (image->size () == QSize (0, 0))
		return;

	float s;
	if ((((double) height ()) / image->height ()) < (((double) width ()) / image->width ()))
		s = ((double) height ()) / image->height ();
	else
		s = ((double) width ()) / image->width ();

	if(isFitWidth)
		fitWidth(true, false);
	else
	if(isFitHeight)
		fitHeight(true, false);
	else
	if(!lock)
	{
		if(s>1 && enlarge)
			scaleFit();
		else
		if(s<1 && shrink)
			scaleFit();
		else
			scale=1;
	}
	placeImage(getImagePosition(), repaint);
}


void
ImageViewer::scaleFit()
{
	fitSize(false);
}

bool
ImageViewer::posXForTopXIsOK(double posX)
{
	int maxX=width();
	return !(
			(
				(posX+virtualPictureWidth()>maxX) &&
				(posX>=0)
			)
		||
			(
				(posX<0) &&
				(posX+virtualPictureWidth()<maxX)
			)
		);
}

bool
ImageViewer::posYForTopYIsOK(double posY)
{
	int maxY=height();
	return !(
			(
				(posY+virtualPictureHeight()>maxY) &&
				(posY>=0)
			)
		||
			(
				(posY<0) &&
				(posY+virtualPictureHeight()<maxY)
			)
		);
}


void
ImageViewer::resizeEvent (QResizeEvent *e)
{
	QWidget::resizeEvent(e);

	if(
		posXForTopXIsOK(getVirtualPosX()+1) &&  posYForTopYIsOK(getVirtualPosY()+1)
		&&
		getVirtualPosX()+virtualPictureWidth()>=width() && getVirtualPosY()+virtualPictureHeight()>=height()
	)
	{
	}
	else
	{
		delete(imageScaled); imageScaled=NULL;
		doScale(true);
	}
	delete(preloadedImage); preloadedImage=NULL;
}


void
ImageViewer::setBackgroundColor(const QColor col)
{
	bgBrush = QBrush(col);
	bgColor_ = col;

	QWidget::setBackgroundColor(col);
	setBackgroundMode(NoBackground);
	repaint();
}

void
ImageViewer::paintEvent (QPaintEvent * e)
{
	if(!((QWidget*)parent())->isUpdatesEnabled()) return;
	if(image)
	{
	#ifdef WANT_DIGIKAMIMAGEEFFECT
// 	FIXME try to use Digikam::ImageIface
//		Digikam::ImageIface m_ImageIface(0,0);
//		delete(image);
//		image = new QImage((uchar*)m_ImageIface.getOriginalData(), m_ImageIface.originalWidth(), m_ImageIface.originalHeight(), 32, 0, 32, QImage::IgnoreEndian);
//	Digikam::ImageIface m_ImageIface(0,0);
//	QPixmap pixtmp(m_ImageIface.originalWidth(), m_ImageIface.originalHeight());
//	m_ImageIface.paint(&pixtmp, 0, 0, m_ImageIface.originalWidth(), m_ImageIface.originalHeight());
//	*image = pixtmp.convertToImage();

	#endif
		int options=ColorOnly||ThresholdDither||ThresholdAlphaDither||AvoidDither;
		if(dragStartPosX + dragStartPosY != -2)
		{
			setVirtualPosX(difTopPosX+dragStartPosX);
			setVirtualPosY(difTopPosY+dragStartPosY);
		}

		//////////////////
		QPoint rtp((int)ceil(e->rect().topLeft().x()/scale),
				(int)ceil(e->rect().topLeft().y()/scale));
		QPoint rbp((int)ceil(e->rect().bottomRight().x()/scale),
				(int)ceil(e->rect().bottomRight().y()/scale));
		QRect redraw(rtp,rbp);
		redraw.moveBy(getPosX(), getPosY());

		int
			cx=max(0,redraw.topLeft().x()),
			cy=max(0,redraw.topLeft().y()),
			cw=min(image->width(),  redraw.width() +min(0,redraw.topLeft().x())+1),
			ch=min(image->height(), redraw.height()+min(0,redraw.topLeft().y())+1);
		if(image->hasAlphaBuffer()){cw++; ch++;}

		int
			x= e->rect().topLeft().x()-min(0, (int)(ceil(redraw.topLeft().x()*scale))),
			y= e->rect().topLeft().y()-min(0, (int)(ceil(redraw.topLeft().y()*scale)));

		int
			newW=((int)ceil(cw*scale)),
			newH=((int)ceil(ch*scale));

		////////////////
		QPainter painter;
		painter.begin(this);

		if(cw>0 && ch>0)
		{
			if(cx==0 && cy==0 && imageScaled)
			{
				painter.drawImage(x,y,*imageScaled);
			}
			else
			if( (!smooth()) ||
				(scale==1.0)  ||
				(dragStartPosX + dragStartPosY != -2) ||
				(ep!=NULL))
			{
				QImage copiedImage=image->copy(cx, cy, cw, ch, options);
				QPixmap scaleCopy(newW, newH);
				QPainter pC(&scaleCopy);
					pC.scale(scale, scale);
					pC.drawImage(0, 0, copiedImage);
					pC.end();
				painter.drawPixmap(x, y, scaleCopy);
			}
			else
			{
				painter.drawImage (x,y, image->copy(cx, cy, cw, ch, options).smoothScale(newW, newH));
			}
		}

		if(getVirtualPosX()>0)
		{
			painter.fillRect(0, 0,
					x, height(),
					bgBrush);
			painter.flush();
		}
		if(getVirtualPosX()+virtualPictureWidth()<width())
		{
			painter.fillRect(getVirtualPosX()+virtualPictureWidth(), 0,
					width()-(getVirtualPosX()+virtualPictureWidth()), height(),
					bgBrush);
			painter.flush();
		}
		if(getVirtualPosY()>0)
		{
			painter.fillRect(0, 0,
					width(), y,
					bgBrush);
			painter.flush();
		}
		if(getVirtualPosY()+virtualPictureHeight()<height())
		{
			painter.fillRect(0, getVirtualPosY()+virtualPictureHeight(),
					width(), height()-(getVirtualPosY()+virtualPictureHeight()),
					bgBrush);
			painter.flush();
		}
		painter.flush();
		painter.end();
	}
	else
	{
		QPainter painter;
		painter.begin(this);
		painter.fillRect(0, 0,width(), height(), bgBrush);
		painter.end();
	}
}

void ImageViewer::setShrink(bool shrink)
{
	this->shrink=shrink;
	delete(imageScaled); imageScaled=NULL;
	delete(preloadedImage); preloadedImage=NULL;
	if(shrink)
	{
		doScale(true);
	}
}

/** Read property of bool enlarge. */
const bool&
ImageViewer::getEnlarge()
{
	return enlarge;
}

/** Write property of bool enlarge. */
void
ImageViewer::setEnlarge(bool enlarge)
{
	this->enlarge = enlarge;
	delete(imageScaled); imageScaled=NULL;
	delete(preloadedImage); preloadedImage=NULL;
	if(enlarge)
	{
		doScale(true);
	}
}

void
ImageViewer::setZoomLock(bool lock)
{
	this->lock=lock;
}

void
ImageViewer::applyFilter(int filter, bool activate)
{
	switch(filter)
	{
		case EFFECT_GRAYSCALE : aEffect_GRAYSCALE->setChecked(activate);break;

		case EFFECT_NORMALIZE : aEffect_NORMALIZE->setChecked(activate);break;
		case EFFECT_EQUALIZE : aEffect_EQUALIZE->setChecked(activate);break;
		case EFFECT_INTENSITY : aEffect_INTENSITY->setChecked(activate);break;
		case EFFECT_INVERT : aEffect_INVERT->setChecked(activate);break;

		case EFFECT_EMBOSS : aEffect_EMBOSS->setChecked(activate);break;
		case EFFECT_SWIRL : aEffect_SWIRL->setChecked(activate);break;
		case EFFECT_SPREAD : aEffect_SPREAD->setChecked(activate);break;
		case EFFECT_IMPLODE : aEffect_IMPLODE->setChecked(activate);break;
		case EFFECT_CHARCOAL : aEffect_CHARCOAL->setChecked(activate);break;
	}
}

QStringList
ImageViewer::getFilterList()
{
	QStringList list;
	if(aEffect_GRAYSCALE->isChecked()) list+="e_grayscale";
	if(aEffect_NORMALIZE->isChecked())list+="e_normalize";
	if(aEffect_EQUALIZE->isChecked())list+="e_equalize";
	if(aEffect_INTENSITY->isChecked())list+="e_intensity";
	if(aEffect_INVERT->isChecked())list+="e_invert";
	if(aEffect_EMBOSS->isChecked())list+="e_emboss";
	if(aEffect_SWIRL->isChecked())list+="e_swirl";
	if(aEffect_SPREAD->isChecked())list+="e_spread";
	if(aEffect_IMPLODE->isChecked())list+="e_implode";
	if(aEffect_CHARCOAL->isChecked())list+="e_charcoal";
	return list;
}


void
ImageViewer::setFilterList(const QStringList& list)
{
	QStringList _list_=list;
	for ( QStringList::Iterator it = _list_.begin(); it != _list_.end(); ++it )
	{
		if(*it=="e_grayscale") aEffect_GRAYSCALE->setChecked(true);
		else if(*it=="e_normalize")aEffect_NORMALIZE->setChecked(true);
		else if(*it=="e_equalize")aEffect_EQUALIZE->setChecked(true);
		else if(*it=="e_intensity")aEffect_INTENSITY->setChecked(true);
		else if(*it=="e_invert")aEffect_INVERT->setChecked(true);
		else if(*it=="e_emboss")aEffect_EMBOSS->setChecked(true);
		else if(*it=="e_swirl")aEffect_SWIRL->setChecked(true);
		else if(*it=="e_spread")aEffect_SPREAD->setChecked(true);
		else if(*it=="e_implode")aEffect_IMPLODE->setChecked(true);
		else if(*it=="e_charcoal")aEffect_CHARCOAL->setChecked(true);
	}
}

void
ImageViewer::applyFilter()
{
	if(!image) return;
	if(image->size() == QSize(0, 0)) return;

	if(aEffect_NORMALIZE->isChecked())
		KImageEffect::normalize(*image);
	if(aEffect_EQUALIZE->isChecked())
		KImageEffect::equalize(*image);
	if(aEffect_INTENSITY->isChecked())
		*image=KImageEffect::intensity(*image, 0.5);
	if(aEffect_INVERT->isChecked())
		image->invertPixels(false);
	if(aEffect_EMBOSS->isChecked())
		*image=KImageEffect::emboss(*image);
	if(aEffect_SWIRL->isChecked())
		*image=KImageEffect::swirl(*image);
	if(aEffect_SPREAD->isChecked())
		*image=KImageEffect::spread(*image);
	if(aEffect_IMPLODE->isChecked())
		*image=KImageEffect::implode(*image);
	if(aEffect_CHARCOAL->isChecked())
		*image=KImageEffect::charcoal(*image);
	if(aEffect_GRAYSCALE->isChecked())
		*image=KImageEffect::desaturate(*image, toGrayscale()/100.0);


}


void
ImageViewer::applyFilterPreloaded()
{
}


bool
ImageViewer::scrolldyB(int dB)
{
	if (virtualPictureHeight()<=height())
		return false;
	dragStartPosX=0;
	dragStartPosY=-ceil((double)dB);
	difTopPosX = getVirtualPosX();
	difTopPosY = getVirtualPosY();
	if(!posYForTopYIsOK(difTopPosY+dragStartPosY))
		dragStartPosY = -(virtualPictureHeight() + getVirtualPosY() - height());
	bool hasDraged=(dragStartPosY!=0);
	if(dragStartPosY!=0)
		scroll((int)(dragStartPosX), (int)(dragStartPosY));
	dragStartPosX=-1;
	dragStartPosY=-1;
	return hasDraged;
}


bool
ImageViewer::scrolldyT(int dT)
{
	if (virtualPictureHeight()<=height())
		return false;
	dragStartPosX=0;
	dragStartPosY=ceil((double)dT);
	difTopPosX = getVirtualPosX();
	difTopPosY = getVirtualPosY();
	if(!posYForTopYIsOK(difTopPosY+dragStartPosY))
		dragStartPosY =  - getVirtualPosY() ;
	bool hasDraged=(dragStartPosY!=0);
	if(dragStartPosY!=0)
		scroll((int)(dragStartPosX), (int)(dragStartPosY));
	dragStartPosX=-1;
	dragStartPosY=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldyB()
{
	if (virtualPictureHeight()<=height())
		return false;
	dragStartPosX=0;
	dragStartPosY=-ceil(10*scale);
	difTopPosX = getVirtualPosX();
	difTopPosY = getVirtualPosY();
	if(!posYForTopYIsOK(difTopPosY+dragStartPosY))
		dragStartPosY = -(virtualPictureHeight() + getVirtualPosY() - height());
	bool hasDraged=(dragStartPosY!=0);
	if(dragStartPosY!=0)
		scroll((int)(dragStartPosX), (int)(dragStartPosY));
	dragStartPosX=-1;
	dragStartPosY=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldyT()
{
	if (virtualPictureHeight()<=height())
		return false;
	dragStartPosX=0;
	dragStartPosY=ceil(10*scale);
	difTopPosX = getVirtualPosX();
	difTopPosY = getVirtualPosY();
	if(!posYForTopYIsOK(difTopPosY+dragStartPosY))
		dragStartPosY =  - getVirtualPosY() ;
	bool hasDraged=(dragStartPosY!=0);
	if(dragStartPosY!=0)
		scroll((int)(dragStartPosX), (int)(dragStartPosY));
	dragStartPosX=-1;
	dragStartPosY=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldxL()
{
	if (virtualPictureWidth()<width())
		return false;
	dragStartPosX=ceil(10*scale);
	dragStartPosY=0;
	difTopPosX = getVirtualPosX();
	difTopPosY = getVirtualPosY();
	if(!posXForTopXIsOK(difTopPosX+dragStartPosX))
		dragStartPosX = -  getVirtualPosX() ;
	bool hasDraged=(dragStartPosX!=0);
	if(dragStartPosX!=0)
		scroll((int)(dragStartPosX), (int)(dragStartPosY));
	dragStartPosX=-1;
	dragStartPosY=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldxR()
{
	if (virtualPictureWidth()<width())
		return false;
	dragStartPosX=-ceil(10*scale);
	dragStartPosY=0;
	difTopPosX = getVirtualPosX();
	difTopPosY = getVirtualPosY();
	if(!posXForTopXIsOK(difTopPosX+dragStartPosX))
		dragStartPosX = - (virtualPictureWidth() + getVirtualPosX() - width());
	bool hasDraged=(dragStartPosX!=0);
	if(dragStartPosX!=0)
		scroll((int)(dragStartPosX), (int)(dragStartPosY));
	dragStartPosX=-1;
	dragStartPosY=-1;
	return hasDraged;
}

void
ImageViewer::slotSaveImage()
{
	QString path;
	if(mw) if(mw->getLastDestDir().isEmpty()) path=mw->getCurrentDir();

	QString destDir=KFileDialog::getSaveFileName(path,
							"*.png *.jpg *.gif *.bmp",
							this,
							i18n("Save File As"));
	if(destDir.isEmpty()) return;


	setMessage(i18n("Saving image...")); kapp->processEvents();
	KApplication::setOverrideCursor (waitCursor);
	QString ext=QFileInfo(destDir).extension().upper();
	if(ext.isEmpty())
	{
		destDir+=".png";
		ext="PNG";
	}
	else
	if(ext=="JPG")
	{
		ext="JPEG";
	}
	if(!image->save(destDir, ext.local8Bit(), 100))
	{
		KApplication::restoreOverrideCursor ();
		KMessageBox::error(this, i18n("Error saving image."));
	}
	else
	{
		KApplication::restoreOverrideCursor ();
	}


	setMessage(i18n("Ready"));
	if(mw) mw->setLastDestDir(destDir);
}

void
ImageViewer::setMessage(const QString& msg)
{
	if(mw)
		mw->setMessage(msg);
	else
		emit sigSetMessage(msg);
}

void
ImageViewer::setZoom(float zoom)
{
	if(mw) mw->setZoom(zoom*100);
}


QColor
ImageViewer::bgColor()
{
	return bgColor_;
}


void
ImageViewer::readConfig(KConfig* config, const QString& group)
{
	config->setGroup(group);
	setSmooth(config->readBoolEntry("smooth", true));
	setBackgroundColor(config->readColorEntry("bgcolor", new QColor("black")));
	setToGrayscale(config->readNumEntry("grayscale", 100));
	setFilterList(config->readListEntry("filterList"));

	//
	config->setGroup("Options");
	nbImg=config->readNumEntry("nbImg", 0);
	setImagePosition((ImageViewer::ImagePosition)config->readNumEntry("imagePosition", (int)Centered));
	setUseEXIF(config->readBoolEntry("useEXIF", TRUE));

	config->setGroup("Zoom");
	aShrink->setChecked(config->readBoolEntry("shrink", true));
	aEnlarge->setChecked(config->readBoolEntry("enlarge", false));
	aZoomLock->setChecked(config->readBoolEntry("lock", false));

	aZoomFitWidth->setChecked(config->readBoolEntry( "fit width", false ));
	aZoomFitHeight->setChecked(config->readBoolEntry( "fit height", false ));

	//
	if(aZoomFitWidth->isChecked()) slotfitWidth();
	else if(aZoomFitHeight->isChecked()) slotfitHeight();

	slotShrink();
	slotEnlarge();
	slotZoomLock();
}


void
ImageViewer::writeConfig(KConfig* config, const QString& group)
{
	config->setGroup(group);
	config->writeEntry( "smooth", smooth() );
	config->writeEntry( "bgcolor", bgColor() );
	config->writeEntry( "grayscale", toGrayscale() );
	config->writeEntry( "filterList", getFilterList() );

	//
	config->setGroup("Options");
	config->writeEntry( "nbImg", nbImg );
	config->writeEntry( "imagePosition", getImagePosition() );

	config->setGroup("Zoom");
	config->writeEntry( "shrink", aShrink->isChecked() );
	config->writeEntry( "enlarge", aEnlarge->isChecked() );
	config->writeEntry( "lock", aZoomLock->isChecked() );
	config->writeEntry( "fit width", aZoomFitWidth->isChecked() );
	config->writeEntry( "fit height", aZoomFitHeight->isChecked() );

	config->setGroup("confirm");
	config->writeEntry( "useEXIF",  useEXIF());

	config->sync();
}


void
ImageViewer::slotfitWidth()
{
	if(aZoomFitWidth->isChecked())
	{
		aZoomFitHeight->setChecked(false);
		aEnlarge->setChecked(false);
		aShrink->setChecked(false);

	}
	fitWidth(aZoomFitWidth->isChecked(), true);
}


void
ImageViewer::slotfitHeight()
{
	if(aZoomFitHeight->isChecked())
	{
		aZoomFitWidth->setChecked(false);
		aEnlarge->setChecked(false);
		aShrink->setChecked(false);

	}
	fitHeight(aZoomFitHeight->isChecked(), true);
}

void
ImageViewer::slotZoomIn()
{
	setMessage(i18n("Zooming In..."));
	zoomIn(1.5);
	setMessage(i18n("Ready"));
}



void
ImageViewer::slotZoomOut()
{
	setMessage(i18n("Zooming Out..."));
	zoomOut(1.5);
	setMessage(i18n("Ready"));
}


void
ImageViewer::slotZoom()
{
    setMessage(i18n("Toggle fit to screen..."));
	setFit(true);
	setMessage(i18n("Ready"));
}


void
ImageViewer::slotZoomNo()
{
	setMessage(i18n("Original size..."));
	originalSize();
	setMessage(i18n("Ready"));
}

void
ImageViewer::slotShrink()
{
	if(aShrink->isChecked())
	{
		aZoomLock->setChecked(false);
		aZoomFitWidth->setChecked(false);
		aZoomFitHeight->setChecked(false);

		slotfitHeight();
		slotfitWidth();
		slotZoomLock();
	}
	setShrink(aShrink->isChecked());
	//
	if(!image || !aEnlarge->isChecked()) {repaint(); return;};
	float s;
	if ((((double) height ()) / image->height ()) < (((double) width ()) / image->width ()))
		s = ((double) height ()) / image->height ();
	else
		s = ((double) width ()) / image->width ();
	if(s<1) slotZoom();
}

void
ImageViewer::slotEnlarge()
{
	if(aEnlarge->isChecked())
	{
		aZoomLock->setChecked(false);
		aZoomFitWidth->setChecked(false);
		aZoomFitHeight->setChecked(false);

		slotfitHeight();
		slotfitWidth();
		slotZoomLock();
	}
	setEnlarge(aEnlarge->isChecked());
	//
	if(!image || !aEnlarge->isChecked()) {repaint(); return;};
	float s;
	if ((((double) height ()) / image->height ()) < (((double) width ()) / image->width ()))
		s = ((double) height ()) / image->height ();
	else
		s = ((double) width ()) / image->width ();
	if(s>1) slotZoom();
}

void
ImageViewer::slotZoomLock()
{
	if(aZoomLock->isChecked()) {aEnlarge->setChecked(false);slotEnlarge();aShrink->setChecked(false);slotShrink();}
	setZoomLock(aZoomLock->isChecked());

}


void
ImageViewer::slotSmooth ()
{
	setSmooth(aSmooth->isChecked());
}

//
void
ImageViewer::selectionChanged(bool selected)
{
	aZoomIn->setEnabled(selected);
	aZoomOut->setEnabled(selected);
	aZoomFit->setEnabled(selected);
	aZoomFitWidth->setEnabled(selected);
	aZoomFitHeight->setEnabled(selected);
	aZoomNo->setEnabled(selected);
	aZoomLock ->setEnabled(selected);

	aRotLeft->setEnabled(selected);
	aRotRight->setEnabled(selected);
	aHMirror->setEnabled(selected);
	aVMirror->setEnabled(selected);

	aPrint->setEnabled(selected);
}

void
ImageViewer::slotSetFilter()
{
	setMessage(i18n("Applying filter(s)"));
	KApplication::setOverrideCursor (waitCursor);
	if(aEffect_NONE->isChecked())
	{
		aEffect_GRAYSCALE->setChecked(false);
		aEffect_NORMALIZE->setChecked(false);
		aEffect_EQUALIZE->setChecked(false);
		aEffect_INTENSITY->setChecked(false);
		aEffect_INVERT->setChecked(false);
		aEffect_EMBOSS->setChecked(false);
		aEffect_SWIRL->setChecked(false);
		aEffect_SPREAD->setChecked(false);
		aEffect_IMPLODE->setChecked(false);
		aEffect_CHARCOAL->setChecked(false);

		aEffect_NONE->setChecked(false);
	}

	applyFilter(EFFECT_GRAYSCALE, aEffect_GRAYSCALE->isChecked());
	applyFilter(EFFECT_NORMALIZE, aEffect_NORMALIZE->isChecked());
	applyFilter(EFFECT_EQUALIZE, aEffect_EQUALIZE->isChecked());
	applyFilter(EFFECT_INTENSITY, aEffect_INTENSITY->isChecked());
	applyFilter(EFFECT_INVERT, aEffect_INVERT->isChecked());
	applyFilter(EFFECT_EMBOSS, aEffect_EMBOSS->isChecked());
	applyFilter(EFFECT_SWIRL, aEffect_SWIRL->isChecked());
	applyFilter(EFFECT_SPREAD, aEffect_SPREAD->isChecked());
	applyFilter(EFFECT_IMPLODE, aEffect_IMPLODE->isChecked());
	applyFilter(EFFECT_CHARCOAL, aEffect_CHARCOAL->isChecked());

	reload();
	setMessage(i18n("Ready"));

	KApplication::restoreOverrideCursor ();
}

void
ImageViewer::slotRotateLeft ()
{
	setMessage(i18n("Rotating..."));
	rotateLeft(false);
	doScale(); repaint();
	setMessage(i18n("Ready"));
}

void
ImageViewer::slotRotateRight ()
{
	setMessage(i18n("Rotating..."));
	rotateRight(false);
	doScale(); repaint();
	setMessage(i18n("Ready"));
}

void
ImageViewer::slotMirrorH ()
{
	setMessage(i18n("Flip..."));
	mirror (false, true);
	setMessage(i18n("Ready"));
}


void
ImageViewer::slotMirrorV ()
{
	setMessage(i18n("Flip..."));
	mirror (true, false);
	setMessage(i18n("Ready"));
}


void
ImageViewer::slotPrint()
{
#ifndef Q_WS_WIN //TODO
	int res;

	KPrinter printer;
	printer.setFullPage( true );
	do {
		if (!printer.setup(this))
			return;
		printImageDialog printDlg(this, getPixmap(), getFilename(), &printer);
		res=printDlg.exec();
	}
	while(res==2);
#endif
}

void
ImageViewer::setImagePosition(ImageViewer::ImagePosition pos)
{
	currentImagePosition_=pos;
}

ImageViewer::ImagePosition
ImageViewer::getImagePosition()
{
	return currentImagePosition_;
}

void
ImageViewer::slotDisplayExifDialog()
{
#ifdef HAVE_LIBKEXIF
#if LIBKEXIF_VERSION < 020
	KExif kexif(this);
#else
	KExifDialog kexif(this);
#endif
	if (kexif.loadFile(filename))
		kexif.exec();
	else
		KMessageBox::sorry(this,
						   i18n("This item has no Exif Information."));
#else
#ifdef __GNUC__
#warning no HAVE_LIBKEXIF
#endif
#endif /* HAVE_LIBKEXIF */
}

bool
ImageViewer::isScrolling()
{
	return isScrolling_;
}

QPixmap
ImageViewer::getPixmap()
{
	if(image)
		return pIO->convertToPixmap(*image);
	else
		return QPixmap(0,0);
}

QImage*
ImageViewer::getImage()
{
	return image;
}

int
ImageViewer::getImageWidth()
{
	if(image)
		return image->width();
	else
		return 0;
}

int
ImageViewer::getImageHeight()
{
	if(image)
		return image->height();
	else
		return 0;
}

QString
ImageViewer::getFilename()
{
	return imageName;
}

void
ImageViewer::setToGrayscale(int togray)
{
	grayscale_=togray;
}

int
ImageViewer::toGrayscale()
{
	return grayscale_;
}

bool
ImageViewer::useEXIF()
{
	return _useEXIF_;
}

void
ImageViewer::setUseEXIF(bool use)
{
	_useEXIF_ = use;
}

void
ImageViewer::previous()
{
	emit askForPreviousImage();
}
void
ImageViewer::next()
{
	emit askForNextImage();
}
void
ImageViewer::first()
{
	emit askForFirstImage();
}
void
ImageViewer::last()
{
	emit askForLastImage();
}

int
ImageViewer::getNbImg()
{
	return nbImg;
}

#include "imageviewer.moc"
