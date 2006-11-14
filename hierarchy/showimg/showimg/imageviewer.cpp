/*****************************************************************************
                          imageviewer.cpp  -  description
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

#include "imageviewer.h"

//-----------------------------------------------------------------------------
#define IMAGEVIEWER_DEBUG    0
//-----------------------------------------------------------------------------

// Local
#include "directoryview.h"
#include "fileiconitem.h"
#include "imagelistview.h"
#include "mainwindow.h"
#include "misc/kmagick.h"
#include "printimagedialog.h"
#include "showimg_common.h"
#include "tools.h"

// System
#include <math.h>
#include <stdlib.h>

// KDE
#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kimageio.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kpixmapio.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <kwin.h>

#ifndef Q_WS_WIN //TODO
#include <kprinter.h>
#endif

#ifdef HAVE_LIBEXIF2
#include <exiv2/exif.hpp>
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIF2
#endif
#endif /* HAVE_LIBEXIF2 */

// QT
#include <qcanvas.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qmovie.h>
#include <qpainter.h>
#include <qpopupmenu.h>

#define ZOOM_MAX 150.0

int min(int a, int b)
{
	return a<b?a:b;
}
int max(int a, int b)
{
	return a>b?a:b;
}

ImageViewer::ImageViewer (
			QWidget * parent,
			const QString& name,
			int wFlags)
	:QWidget (parent, name.ascii(), WResizeNoErase|WRepaintNoErase|WPaintClever|wFlags),

	m_p_il(NULL),

	m_loaded_image_url(0),
	m_p_loaded_image(NULL),
	m_p_scaled_loaded_image(NULL),
	imageIndex(-1),
	m_p_pre_loaded_image(NULL),
	m_p_pre_loaded_scaled_image(NULL),

	m_p_popup(NULL),


	m_p_start_point(NULL),
	m_p_end_point(NULL),

	m_scale_value(1),
	m_is_smooth_image(true),
	button(NoButton),
	m_p_movie(NULL),
	m_is_fit_all(false),
	m_is_fit_width(false),
	m_is_fit_height(false),
	m_is_shrink(false),
	m_is_enlarge(false),
	m_is_lock_zoom(false),
	m_real_pos_X(-1),
	m_real_pos_Y(-1),
	m_drag_start_pos_X(-1),
	m_drag_start_pos_Y(-1),
	m_top_pos_X(0),
	m_top_pos_Y(0),
	m_diff_top_pos_X(-1),
	m_diff_top_pos_Y(-1),
	m_is_scrolling(false),
	m_has_image(false),

	m_total_images(0),
	m_nbr_images(0),



	aScrollXR(NULL)
{
	setToGrayscale(-1);
	//
	m_p_pixmap_IO=new KPixmapIO();
	if(KStandardDirs::exists(locate("appdata", "pics/bgxpm.png")))
		m_p_bg_pixmap = new QPixmap(locate("appdata", "pics/bgxpm.png"));
	else
	{
		m_p_bg_pixmap = new QPixmap(1,1);
		m_p_bg_pixmap->fill(Qt::black);
	}
	m_p_local = KGlobal::locale();
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
ImageViewer::setMainWindow(MainWindow *)
{
	m_p_il = getMainWindow()?getMainWindow()->getImageListView():NULL;
}


void
ImageViewer::initActions(KActionCollection *a_p_actionCollection)
{
	m_p_actionCollection = a_p_actionCollection;

	if(aScrollXR)
	{
		MYWARNING<<" initActions already done!"<<endl;
		return;
	}
	KShortcut sc_zi(KStdAccel::shortcut(KStdAccel::ZoomIn)); sc_zi.append(KKeySequence((const KKey&)Qt::Key_Plus));
	KShortcut sc_zo(KStdAccel::shortcut(KStdAccel::ZoomOut)); sc_zo.append(KKeySequence((const KKey&)Qt::Key_Minus));

	//--------------------------------------------------------------------------//
	aScrollXR	=new KAction(i18n("Scroll on the Right"), Key_Right, this, SLOT(scrolldxR()), m_p_actionCollection, "ScrollXR");
	aScrollYB	=new KAction(i18n("Scroll at the Bottom"),Key_Down , this, SLOT(scrolldyB()), m_p_actionCollection, "ScrollYB");
	aScrollXL	=new KAction(i18n("Scroll on the Left"), Key_Left, this, SLOT(scrolldxL()), m_p_actionCollection, "ScrollXL");
	aScrollYT	=new KAction(i18n("Scroll on the Top"), Key_Up, this, SLOT(scrolldyT()), m_p_actionCollection, "ScrollYT");

	aScrollXRQuick	=new KAction(i18n("Scroll on the Right Quickly"),  KShortcut(SHIFT+Key_Right), this, SLOT(scrolldxRQuick()), m_p_actionCollection, "ScrollXR Quickly");
	aScrollYBQuick	=new KAction(i18n("Scroll at the Bottom Quickly"), KShortcut(SHIFT+Key_Down) , this, SLOT(scrolldyBQuick()), m_p_actionCollection, "ScrollYB Quickly");
	aScrollXLQuick	=new KAction(i18n("Scroll on the Left Quickly"),  KShortcut(SHIFT+Key_Left), this, SLOT(scrolldxLQuick()), m_p_actionCollection, "ScrollXL Quickly");
	aScrollYTQuick	=new KAction(i18n("Scroll on the Top Quickly"),  KShortcut(SHIFT+Key_Up), this, SLOT(scrolldyTQuick()), m_p_actionCollection, "ScrollYT Quickly");

	//--------------------------------------------------------------------------//
	aZoomIn		=
		new KAction(i18n("Zoom In"), "viewmag_bis+", sc_zi,
		this, SLOT(slotZoomIn()), m_p_actionCollection, "Zoom in");
	aZoomOut	=
		new KAction(i18n("Zoom Out"), "viewmag_bis-", sc_zo,
		this, SLOT(slotZoomOut()), m_p_actionCollection, "Zoom out");
	aZoomFit	=
		new KAction(i18n("Fit to Screen"), "viewmag_full", KShortcut(Key_Slash),
		this, SLOT(slotZoom()), m_p_actionCollection, "Fit to Screen");
	aZoomFitWidth =
		new KToggleAction(i18n("Fit Width"), "viewmag_w", 0,
		this, SLOT(slotfitWidth()), m_p_actionCollection, "Fit the width");
	aZoomFitHeight =
		new KToggleAction(i18n("Fit Height"), "viewmag_h", 0,
		this, SLOT(slotfitHeight()), m_p_actionCollection, "Fit the height");
	aZoomNo =
		new KAction(i18n("Original Size"), "viewmag_no", KShortcut(Key_Asterisk),
		this, SLOT(slotZoomNo()), m_p_actionCollection, "Originale size");
	aZoomLock =
		new KToggleAction(i18n("Lock Zoom"), "viewmag_lock", 0,
		this, SLOT(slotZoomLock()), m_p_actionCollection, "ZoomLock");
	aEnlarge =
		new KToggleAction(i18n("Enlarge if Smaller"), "viewmag_enlarge", 0,
		this, SLOT(slotEnlarge()), m_p_actionCollection, "Enlarge");
	aShrink =
		new KToggleAction(i18n("Shrink if Bigger"), "viewmag_shrink", 0,
		this, SLOT(slotShrink()), m_p_actionCollection, "Shrink");

	KActionMenu *actionZoom = new KActionMenu( i18n("Zoom"), "viewmag", m_p_actionCollection, "view_zoomm" );
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
	//--------------------------------------------------------------------------//
	aWallpaper_CENTER	=new KAction(i18n("Centered"), 0, this, SLOT(wallpaperC()), m_p_actionCollection, "Center");
	aWallpaper_MOSAIC	=new KAction(i18n("Tiled"), 0, this, SLOT(wallpaperM()), m_p_actionCollection, "Mosaic");
	aWallpaper_CENTER_MOSAIC=new KAction(i18n("Center Tiled"), 0, this, SLOT(wallpaperCM()), m_p_actionCollection, "Center adapt");
	aWallpaper_CENTER_MAX	=new KAction(i18n("Centered Maxpect"), 0, this, SLOT(wallpaperCMa()), m_p_actionCollection, "Center max");
	aWallpaper_ADAPT	=new KAction(i18n("Scaled"), 0, this, SLOT(wallpaperA()), m_p_actionCollection, "Adapt");
	aWallpaper_LOGO		=new KAction(i18n("Logo"), 0, this, SLOT(wallpaperL()), m_p_actionCollection, "Logo");
#else
	aWallpaper_CENTER=aWallpaper_MOSAIC=aWallpaper_CENTER_MOSAIC=aWallpaper_CENTER_MAX=aWallpaper_ADAPT=aWallpaper_LOGO=NULL;
#endif

//--------------------------------------------------------------------------//
	aSmooth		=new KToggleAction(i18n("Smooth Scaling"), 0, this, SLOT(slotSmooth()), m_p_actionCollection, "Smooth scaling");

	//--------------------------------------------------------------------------//
	KAction *aEffect_REDEYES = new KAction(i18n("Remove Red Eyes"), 0, this, SLOT(removeRedEye()), m_p_actionCollection, "effect_remove red_eyes");

	aEffect_GRAYSCALE=new KToggleAction(i18n("Gray Scale"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_grayscale");
	aEffect_NORMALIZE=new KToggleAction(i18n("Normalize"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_normalize");
	aEffect_EQUALIZE=new KToggleAction(i18n("Equalize"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Equalize");
	aEffect_INTENSITY=new KToggleAction(i18n("Intensity"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Intensity");
	aEffect_INVERT=new KToggleAction(i18n("Invert"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Invert");
	aEffect_EMBOSS=new KToggleAction(i18n("Emboss"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Emboss");
	aEffect_SWIRL=new KToggleAction(i18n("Swirl"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Swirl");
	aEffect_SPREAD=new KToggleAction(i18n("Spread"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Spread");
	aEffect_IMPLODE=new KToggleAction(i18n("Implode"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Implode");
	aEffect_CHARCOAL=new KToggleAction(i18n("Charcoal"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_Chaorcoal");
	aEffect_NONE=new KToggleAction(i18n("None"), 0, this, SLOT(slotSetFilter()), m_p_actionCollection, "effect_effect_none");
	m_p_actionEffects_menu = new KActionMenu( i18n("Effects"), "kcoloredit", m_p_actionCollection, "view_effects" );
	m_p_actionEffects_menu->insert(aEffect_NONE);
	m_p_actionEffects_menu->insert(new KActionSeparator());
	m_p_actionEffects_menu->insert(aEffect_REDEYES);
	m_p_actionEffects_menu->insert(new KActionSeparator());
	m_p_actionEffects_menu->insert(aEffect_GRAYSCALE);
	m_p_actionEffects_menu->insert(aEffect_NORMALIZE);
	m_p_actionEffects_menu->insert(aEffect_EQUALIZE);
	m_p_actionEffects_menu->insert(aEffect_INTENSITY);
	m_p_actionEffects_menu->insert(aEffect_INVERT);
	m_p_actionEffects_menu->insert(new KActionSeparator());
	m_p_actionEffects_menu->insert(aEffect_EMBOSS);
	m_p_actionEffects_menu->insert(aEffect_SWIRL);
	m_p_actionEffects_menu->insert(aEffect_SPREAD);
	m_p_actionEffects_menu->insert(aEffect_IMPLODE);
	m_p_actionEffects_menu->insert(aEffect_CHARCOAL);

	//--------------------------------------------------------------------------//
	aRotLeft	=new KAction(i18n("Rotate Left"), "rotation_acw", KShortcut(Key_L), this, SLOT(slotRotateLeft()), m_p_actionCollection, "Rotate Left");
	aRotRight	=new KAction(i18n("Rotate Right"), "rotation_cw", KShortcut(Key_R), this, SLOT(slotRotateRight()), m_p_actionCollection, "Rotate Right");
	aHMirror	=new KAction(i18n("Vertical Flip"), "flip", 0, this, SLOT(slotMirrorH()), m_p_actionCollection, "Flip");
	aVMirror	=new KAction(i18n("Horizontal Flip"), "miror", 0, this, SLOT(slotMirrorV()), m_p_actionCollection, "Mirror");
	m_p_actionOrientation_menu = new KActionMenu( i18n("Orientation"), "rotation_acw", m_p_actionCollection, "view_Orientation" );
		m_p_actionOrientation_menu->insert(aRotLeft);
		m_p_actionOrientation_menu->insert(aRotRight);
		m_p_actionOrientation_menu->insert(aHMirror);
		m_p_actionOrientation_menu->insert(aVMirror);

	//--------------------------------------------------------------------------
	aPrint		=new KAction(i18n("Print..."), "fileprint", KStdAccel::shortcut(KStdAccel::Print), this, SLOT(slotPrint()), m_p_actionCollection, "fileprint");

	//--------------------------------------------------------------------------
	#ifdef HAVE_LIBEXIV2
	aDisplayExifDialog = new KAction(i18n("Exif Information"), 0, this, SLOT(slotDisplayExifDialog()), m_p_actionCollection, "display_Exif_Dialog");
	#else
	aDisplayExifDialog = NULL;
	#endif /* HAVE_LIBEXIV2 */

	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	aSaveImage	=new KAction(i18n("&Save"), "filesave", KStdAccel::shortcut(KStdAccel::Save), this, SLOT(slotSaveImage()), m_p_actionCollection, "filesave" );
	aSaveAsImage	=new KAction(i18n("Save &As..."), "filesaveas", 0, this, SLOT(slotSaveAsImage()), m_p_actionCollection, "filesaveas" );

	//--------------------------------------------------------------------------
	QAccel* accel=m_p_actionCollection->accel();
	if(accel)
	{
		aPrevious	=new KAction(i18n("Previous Image"), "1leftarrow", 0,
					this, SLOT(previous()), m_p_actionCollection, "Previous Image" );
		aNext		=new KAction(i18n("Next Image"), "1rightarrow", 0,
					this, SLOT(next()), m_p_actionCollection, "Next Image");

		accel->connectItem(accel->insertItem(Key_PageUp), this, SLOT(previous()));
		accel->connectItem(accel->insertItem(Key_PageDown), this, SLOT(next()));
	}
	else
	{
		aPrevious	=new KAction(i18n("Previous Image"), "1leftarrow", Key_PageUp,
					this, SLOT(previous()), m_p_actionCollection, "Previous Image" );
		aNext		=new KAction(i18n("Next Image"), "1rightarrow", Key_PageDown,
					this, SLOT(next()), m_p_actionCollection, "Next Image");
	}

	aFirst		=new KAction(i18n("First Image"), "top", KStdAccel::shortcut(KStdAccel::Home), this, SLOT(first()), m_p_actionCollection, "First Image" );
	aLast		=new KAction(i18n("Last Image"), "bottom", KStdAccel::shortcut(KStdAccel::End), this, SLOT(last()),m_p_actionCollection, "Last Image" );

	if(m_p_actionCollection->action("action go"))
	{
		(dynamic_cast<KActionMenu*>(m_p_actionCollection->action("action go")))->insert(aFirst);
		(dynamic_cast<KActionMenu*>(m_p_actionCollection->action("action go")))->insert(aLast);
		(dynamic_cast<KActionMenu*>(m_p_actionCollection->action("action go")))->insert(aPrevious);
		(dynamic_cast<KActionMenu*>(m_p_actionCollection->action("action go")))->insert(aNext);
	}
}

void
ImageViewer::initMenu(KActionCollection *a_p_actionCollection)
{
	m_p_actionCollection = a_p_actionCollection;
	if(m_p_popup)
		m_p_popup->clear();
	else
		m_p_popup = new KPopupMenu();

	if(!getMainWindow())
	{
		m_p_popup->insertTitle(i18n("ShowImg Preview"), 1);
		if(m_p_actionCollection->action("Simple Interface Switch to interface"))
		{
			m_p_actionCollection->action("Simple Interface Switch to interface")->plug(m_p_popup);
			(new KActionSeparator())->plug(m_p_popup);
		}
		m_p_actionCollection->action("action go")->plug(m_p_popup);
		m_p_actionCollection->action("view_zoomm")->plug(m_p_popup);
		m_p_actionCollection->action("view_effects")->plug(m_p_popup);
		m_p_actionCollection->action("view_Orientation")->plug(m_p_popup);
		if(aDisplayExifDialog) m_p_actionCollection->action("display_Exif_Dialog")->plug(m_p_popup);
		KActionSeparator *sep = new KActionSeparator(); sep->plug(m_p_popup);
		m_p_actionCollection->action("filesaveas")->plug(m_p_popup);
		m_p_actionCollection->action("fileprint")->plug(m_p_popup);
		if(m_p_actionCollection->action("Simple Interface Quit"))
		{
			(new KActionSeparator())->plug(m_p_popup);
			m_p_actionCollection->action("Simple Interface Quit")->plug(m_p_popup);
		}
	}
	else
	{
#ifndef HAVE_KIPI
		KActionMenu *actionWallp = new KActionMenu( i18n("Set as Wallpaper"), "desktop", m_p_actionCollection, "view_wallp" );
		actionWallp->insert(aWallpaper_CENTER);
		actionWallp->insert(aWallpaper_MOSAIC);
		actionWallp->insert(aWallpaper_CENTER_MOSAIC);
		actionWallp->insert(aWallpaper_CENTER_MAX);
		actionWallp->insert(aWallpaper_ADAPT);
		actionWallp->insert(aWallpaper_LOGO);
#endif /* HAVE_KIPI */

		m_p_actionCollection->action("FullScreen")->plug(m_p_popup);

		m_p_popup->insertSeparator();

		m_p_actionCollection->action("view_zoomm")->plug(m_p_popup);
		m_p_actionCollection->action("view_Orientation")->plug(m_p_popup);
		m_p_actionCollection->action("view_effects")->plug(m_p_popup);

		m_p_actionCollection->action("action go")->plug(m_p_popup);

		m_p_popup->insertSeparator();

		m_p_popup->insertSeparator();
		m_p_actionCollection->action("filesaveas")->plug(m_p_popup);
		m_p_actionCollection->action("editcopy")->plug(m_p_popup);

		m_p_popup->insertSeparator();
		m_p_actionCollection->action("edittrash")->plug(m_p_popup);
		m_p_actionCollection->action("editdelete")->plug(m_p_popup);

		m_p_popup->insertSeparator();
		m_p_actionCollection->action("Image Info")->plug(m_p_popup);
		if(aDisplayExifDialog)  m_p_actionCollection->action("display_Exif_Dialog")->plug(m_p_popup);
		m_p_actionCollection->action("Properties")->plug(m_p_popup);
	}
}

void
ImageViewer::updateActions()
{
	aScrollXR->setEnabled(hasImage());
	aScrollYB->setEnabled(hasImage());
	aScrollXL->setEnabled(hasImage());
	aScrollYT->setEnabled(hasImage());

	aRotLeft->setEnabled(hasImage());
	aRotRight->setEnabled(hasImage());
	aHMirror->setEnabled(hasImage());
	aVMirror->setEnabled(hasImage());

	aZoomIn->setEnabled(hasImage());
	aZoomOut->setEnabled(hasImage());
	aZoomFit->setEnabled(hasImage());
	aZoomNo->setEnabled(hasImage());

	m_p_actionEffects_menu->setEnabled(hasImage());
	m_p_actionOrientation_menu->setEnabled(hasImage());

	aPrint->setEnabled(hasImage());
	aSaveImage->setEnabled(hasImage());
	aSaveAsImage->setEnabled(hasImage());
}

bool
ImageViewer::preopenURL (const KURL& a_url, const QString & a_mimetype/*=QString::null*/)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	KURL l_url;
	if(!Tools::isImage(a_url))
		l_url=KURL();
	else
		l_url=a_url;

	const QFile::Offset big = 0x501400; // 5MB
	QString imageFormat = QImageIO::imageFormat(l_url.path());
	if( (QFile(l_url.path()).size() > big) ||
		( imageFormat == QString::fromLatin1("GIF")))
	{
		MYWARNING
			<<"m_p_loaded_image's too big or is GIF"
			<<endl;
		l_url=KURL();
	}
	else
	{
	}
	m_pre_loaded_image_url=l_url;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=new QImage();
	if(!m_p_pre_loaded_image->load(l_url.path()))
	{
		delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
		delete(m_p_pre_loaded_scaled_image);m_p_pre_loaded_scaled_image=NULL;

		return false;
	}
	scalePreload();
	return true;
}


void
ImageViewer::reload()
{
	openURL(m_loaded_image_url);
}


bool
ImageViewer::openURL (const KURL& a_url, const QString & a_mimetype/*=QString::null*/)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<"BEGIN a_url="<<a_url.url()<<endl;
#endif
	KURL l_image_url = a_url;
	if(!Tools::isImage(l_image_url))
		l_image_url=KURL();

	bool l_is_ok = false;
	if (!l_image_url.isEmpty())
	{
		FileIconItem *l_p_item =
			(m_p_il
			?m_p_il->findItem(l_image_url.path(), true)
			:NULL);
		if(l_p_item)
		{
			if(!l_p_item->isSelected())
			{
				delete (m_p_pre_loaded_image);    m_p_pre_loaded_image=NULL;
				delete (m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
				return false;
			}
		}
 		else
		{
			if(m_p_il)
	 		{
	 			delete (m_p_pre_loaded_image);    m_p_pre_loaded_image=NULL;
	 			delete (m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	 			return false;
	 		}
		}
		//--------------------------------------------------------------------------///
#ifdef HAVE_LIBEXIV2
		aDisplayExifDialog->setEnabled(Tools::isJPEG(l_image_url));
#endif /* HAVE_LIBEXIV2 */

		setMessage(i18n("Loading image..."));
		KApplication::setOverrideCursor( waitCursor );

		m_loaded_image_url = l_image_url;
		m_nbr_images++;
		if (
			l_image_url != m_pre_loaded_image_url && 
			m_p_pre_loaded_image)
		{
#if IMAGEVIEWER_DEBUG > 0
			MYDEBUG<<"l_image_url != m_pre_loaded_image_url && m_p_pre_loaded_image"<<endl;
#endif
			delete (m_p_loaded_image);
			m_p_loaded_image = new QImage(*m_p_pre_loaded_image);
			
			delete(m_p_pre_loaded_image);
			m_p_pre_loaded_image=NULL;
			
			delete(m_p_scaled_loaded_image);
			if(
				m_p_pre_loaded_scaled_image && 
				m_p_loaded_image
			)
			{
#if IMAGEVIEWER_DEBUG > 0
				MYDEBUG<<"m_p_pre_loaded_scaled_image && m_p_loaded_image"<<endl;
#endif
				m_p_scaled_loaded_image=m_p_pre_loaded_scaled_image;
				reconvertImage();
				m_p_pre_loaded_scaled_image=NULL;
				l_is_ok=true;
			}
			else
			{
				l_is_ok=false;
			}
		}
		if(!l_is_ok)
		{
#if IMAGEVIEWER_DEBUG > 0
			MYDEBUG<<"l_is_ok"<<endl;
#endif
			delete(m_p_loaded_image);         m_p_loaded_image = new QImage ();
			delete (m_p_pre_loaded_image);    m_p_pre_loaded_image=NULL;
			delete (m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;

			QString l_image_path;
			if(m_loaded_image_url.isLocalFile())
			{
#if IMAGEVIEWER_DEBUG > 0
				MYDEBUG<<"m_loaded_image_url.isLocalFile()"<<endl;
#endif
				l_image_path = m_loaded_image_url.path();
			}
			else
			{
#if IMAGEVIEWER_DEBUG > 0
				MYDEBUG<<"! m_loaded_image_url.isLocalFile()"<<endl;
#endif
				l_image_path = locateLocal("tmp", "showimg-net/");
				l_image_path += m_loaded_image_url.filename();
				if( KIO::NetAccess::download( m_loaded_image_url, l_image_path, getMainWindow() ) )
				{
#if IMAGEVIEWER_DEBUG > 0
					MYDEBUG<<"KIO::NetAccess::download( l_url, l_image_path="<< l_image_path<<", getMainWindow() ) "<<endl;
#endif
				}
			}
			l_is_ok = m_p_loaded_image->load(l_image_path);
			if(!m_loaded_image_url.isLocalFile())
			{
#if IMAGEVIEWER_DEBUG > 0
				MYDEBUG<<"!m_loaded_image_url.isLocalFile()"<<endl;
#endif
				KIO::NetAccess::removeTempFile(l_image_path);
			}
			reconvertImage();
		}
	}
	if(l_is_ok)
	{
		if(m_p_movie)
		{
			m_p_movie->disconnectUpdate(this);
			m_p_movie->disconnectStatus(this);
			m_p_movie->pause();
		}
		if(useEXIF()) autoRotate(false);
		applyFilter();
		doScale(false);

		setZoom(m_scale_value);
		//
		m_imageType = QImageIO::imageFormat(m_loaded_image_url.path());
		if( 
			m_imageType == QString::fromLatin1("MNG")  || 
			m_imageType == QString::fromLatin1("GIF")
		)
		{
			repaint();
			startMovie();
		}
		else
		{
			m_p_movie=NULL;
		}
	}
	else
	{
		m_loaded_image_url="(none)";
		delete(m_p_movie);m_p_movie=NULL;
		delete(m_p_loaded_image);m_p_loaded_image=NULL;
		delete (m_p_scaled_loaded_image);m_p_scaled_loaded_image=NULL;
		delete (m_p_pre_loaded_image);m_p_pre_loaded_image=NULL;
		delete (m_p_pre_loaded_scaled_image);m_p_pre_loaded_scaled_image=NULL;
	}
	updateStatus ();
	m_has_image = m_p_loaded_image!=NULL;

	setMessage(i18n("Ready"));
	if(!m_p_movie)repaint();
	KApplication::restoreOverrideCursor();

	//
	emit loaded(m_loaded_image_url);
	updateActions();

	//
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<"END"<<endl;
#endif
	return l_is_ok;
}

bool
ImageViewer::hasImage()
{
	return m_has_image;
}


bool
ImageViewer::reconvertImage()
{
	if (!m_p_loaded_image)
		return false;
	if(m_p_loaded_image->hasAlphaBuffer())
	{
		QPixmap pix(m_p_loaded_image->size());
		QPainter p;
			p.begin(&pix);
			p.drawTiledPixmap (0, 0, m_p_loaded_image->width(), m_p_loaded_image->height(), *m_p_bg_pixmap);
			p.drawImage(0, 0,*m_p_loaded_image);
			p.end();
		*m_p_loaded_image=pix.convertToImage();
	}
	return true;
}


bool
ImageViewer::smooth () const
{
	return m_is_smooth_image;
}


void
ImageViewer::scalePreload ()
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	if(
		(aEffect_NORMALIZE->isChecked() || aEffect_EQUALIZE->isChecked() || aEffect_INTENSITY->isChecked() || aEffect_INVERT->isChecked() || aEffect_EMBOSS->isChecked() || aEffect_SWIRL->isChecked() || aEffect_SPREAD->isChecked() || aEffect_IMPLODE->isChecked() || aEffect_CHARCOAL->isChecked() || aEffect_GRAYSCALE->isChecked())
		||
		(m_p_pre_loaded_image && m_p_pre_loaded_image->hasAlphaBuffer())
	)
	{
		delete(m_p_pre_loaded_scaled_image); m_p_pre_loaded_scaled_image=NULL;
		delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
		delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;

		return;
	}
	float s;
	if ((((double)height()) / m_p_pre_loaded_image->height ()) < (((double) width ()) / m_p_pre_loaded_image->width ()))
		s = ((double)height()) / m_p_pre_loaded_image->height ();
	else
		s = ((double) width ()) / m_p_pre_loaded_image->width ();
	if(!m_is_lock_zoom)
	{
		if( (s>1 && m_is_enlarge) ||  (s<1 && m_is_shrink) )
		{
			//
		}
		else
		{
			s=1;
		}
	}
	else
		s=m_scale_value;

	QPoint rtp(0, 0);
	QPoint rbp((int)ceil(width()/s),(int)ceil(height()/s));
	QRect redraw(rtp,rbp);
	int
		cx=0,
		cy=0,
		cw=min(m_p_pre_loaded_image->width(),  redraw.width()),
		ch=min(m_p_pre_loaded_image->height(), redraw.height());

	int options=ColorOnly|ThresholdDither|ThresholdAlphaDither|AvoidDither;
	delete(m_p_pre_loaded_scaled_image); m_p_pre_loaded_scaled_image=new QImage();
	*m_p_pre_loaded_scaled_image
		=m_p_pre_loaded_image->copy(cx, cy,
					cw, ch,
					options)
				.smoothScale((int)ceil(cw*s), (int)ceil(ch*s));
}


void
ImageViewer::startMovie()
{
	delete(m_p_movie); m_p_movie=NULL;
	if (!m_loaded_image_url.isEmpty()) // Start a new m_p_movie - have it delete when closed.
		initMovie();
}


void
ImageViewer::initMovie()
{
	m_p_movie = new QMovie(m_loaded_image_url.path());
	nbrMU=-1;
	QPixmap pix(m_p_loaded_image->width(), m_p_loaded_image->height()); pix.fill(m_bg_brush.color()); *m_p_loaded_image=m_p_pixmap_IO->convertToImage(pix);//repaint();
	m_p_movie->setBackgroundColor(m_bg_brush.color());
	m_p_movie->connectUpdate(this, SLOT(movieUpdated(const QRect&)));
	m_p_movie->connectStatus(this, SLOT(movieStatus(int)));
}

void
ImageViewer::movieUpdated(const QRect& )
{
	if(! m_p_movie)
		return;
	nbrMU++;
	if(nbrMU>m_p_movie->frameNumber()) /* the gif is not animated */
	{
		m_p_movie->disconnectUpdate(this);
		m_p_movie->disconnectStatus(this);
		m_p_movie->pause();
		m_p_movie=0L;
		delete(m_p_movie); m_p_movie=NULL;
		delete(m_p_loaded_image); m_p_loaded_image=new QImage(m_loaded_image_url.path());
		delete (m_p_scaled_loaded_image);m_p_scaled_loaded_image=NULL;
		reconvertImage();
		applyFilter();
		doScale(false);
		return;
	}
	else
	{
		*m_p_loaded_image = m_p_pixmap_IO->convertToImage(m_p_movie->framePixmap());
		if(nbrMU!=0)repaint();
	}
}


void
ImageViewer::movieStatus(int a_status)
{
	if (m_p_movie && a_status<0)
		KMessageBox::error(this, i18n("Could not play m_p_movie \"%1\"").arg(m_loaded_image_url.url()));
	if(a_status==3) nbrMU=-1;
}


void
ImageViewer::mousePressEvent (QMouseEvent * e)
{
	QWidget::mousePressEvent(e);

	button = e->button ();
	if (button==RightButton)
	{
		if(m_p_il)
		{
			m_p_popup->removeItemAt(7);
			m_p_popup_openWith=m_p_il->popupOpenWith();
			m_p_popup->insertItem(i18n("Open With"), m_p_popup_openWith, -1, 7);
			m_p_il->setSelected(m_p_il->currentItem(), true, false);
		}
		if(m_p_popup)
		{
			m_p_popup->exec(e->globalPos());
		}
	}
	else
	if (button==LeftButton)
	{
		if(!m_p_loaded_image) return;
		KApplication::setOverrideCursor (sizeAllCursor);
		m_drag_start_pos_X=e->pos().x();
		m_drag_start_pos_Y=e->pos().y();
		m_diff_top_pos_X = getVirtualPosX()-m_drag_start_pos_X;
		m_diff_top_pos_Y = getVirtualPosY()-m_drag_start_pos_Y;
	}
	else
	{
		delete(m_p_start_point); m_p_start_point = new QPoint(e->pos()) ;
		m_p_end_point = new QPoint(*m_p_start_point);
	}
}


void
ImageViewer::mouseReleaseEvent (QMouseEvent * e)
{
	if (e->button()==LeftButton)
	{
		if(!m_p_loaded_image) return;
		KApplication::restoreOverrideCursor ();
		QWidget::mouseReleaseEvent(e);
		m_drag_start_pos_X=-1;
		m_drag_start_pos_Y=-1;
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
	if(m_p_loaded_image)
	{
		delete(m_p_end_point); m_p_end_point = new QPoint(e->pos()) ;
		if(*m_p_start_point==*m_p_end_point && !isScrolling())
		{
			m_p_end_point=NULL; m_p_last_point=NULL;
			KApplication::setOverrideCursor (waitCursor);
			doScale(false);placeImage(false);repaint();
			KApplication::restoreOverrideCursor ();
			button=NoButton;
			return;
		}
		else if(!isScrolling())
		{
			QRect rectDraged= QRect(
					QPoint(
						max(min(m_p_start_point->x(),m_p_end_point->x()), getVirtualPosX()),
						max(min(m_p_start_point->y(),m_p_end_point->y()), getVirtualPosY())),
					QPoint(
						min(max(m_p_start_point->x(),m_p_end_point->x()), getVirtualPosX()+virtualPictureWidth()),
						min(max(m_p_start_point->y(),m_p_end_point->y()), getVirtualPosY()+virtualPictureHeight())));

			QPoint oldCenter=QPoint( (int)(((rectDraged.left()+rectDraged.right())/2 - getVirtualPosX())/m_scale_value),
						 (int)(((rectDraged.top()+rectDraged.bottom())/2 - getVirtualPosY())/m_scale_value));

			rectDraged.moveBy(-getVirtualPosX(), -getVirtualPosY());
			float
				sx=width()/rectDraged.width(),
				sy=height()/rectDraged.height();
			if(m_scale_value*((float)sx+(float)sy)/2 > ZOOM_MAX)
				m_scale_value=ZOOM_MAX;
			else
				m_scale_value*=((float)sx+(float)sy)/2;

			setZoom(m_scale_value);
			oldCenter*=m_scale_value;
			m_p_end_point=NULL; m_p_last_point=NULL;

			centerImage(oldCenter.x(), oldCenter.y(), true);
		}
		m_p_end_point=NULL; m_p_last_point=NULL;
		KApplication::restoreOverrideCursor ();
	}
	m_p_end_point=NULL; m_p_last_point=NULL;
	button=NoButton;
	m_is_scrolling=false;
}


void
ImageViewer::mouseMoveEvent(QMouseEvent * e)
{
	if (button==LeftButton && !isScrolling() )
	{
		QWidget::mouseMoveEvent(e);
		if ( (m_drag_start_pos_X+m_drag_start_pos_Y!=-2.0))
		{
			double
				diffX=e->pos().x()-m_drag_start_pos_X,
				diffY=e->pos().y()-m_drag_start_pos_Y,
				panX=0, panY=0;

			if (virtualPictureWidth()>width())
			{
				if(fabs(diffX)>=m_scale_value)
				{
					panX=(int)diffX;
					m_drag_start_pos_X+=panX;
					if(!posXForTopXIsOK(m_diff_top_pos_X+m_drag_start_pos_X))
					{
						if( m_diff_top_pos_X+m_drag_start_pos_X>=0 )
						{
							m_drag_start_pos_X -= panX;
							panX = - getVirtualPosX();
						}
						else
						{
							m_drag_start_pos_X -= panX;
							panX = - (virtualPictureWidth() + getVirtualPosX() - width() );
						}
						m_drag_start_pos_X+= panX;
					}
				}
			}
			if (virtualPictureHeight()>height())
			{
				if(fabs(diffY)>=m_scale_value)
				{
					panY=diffY;
					m_drag_start_pos_Y+=panY;
					if(!posYForTopYIsOK(m_diff_top_pos_Y+m_drag_start_pos_Y))
					{
						if( m_diff_top_pos_Y+m_drag_start_pos_Y>=0 )
						{
							m_drag_start_pos_Y -= panY;
							panY = - getVirtualPosY();
						}
						else
						{
							m_drag_start_pos_Y -= panY;
							panY = - (virtualPictureHeight() + getVirtualPosY() - height() );
						}
						m_drag_start_pos_Y+= panY;
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
	if(m_p_movie || !m_p_end_point )
		return;

	QPainter p(this); p.setPen(QColor("black"));
	m_p_last_point = new QPoint(*m_p_end_point);
	m_p_end_point = new QPoint(e->pos()) ;
	int
		lx = m_p_last_point->x(),
		ly = m_p_last_point->y(),
		ex = m_p_end_point->x(),
		ey = m_p_end_point->y(),
		sx = m_p_start_point->x(),
		sy = m_p_start_point->y();

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

	p.drawRect(QRect(*m_p_start_point,*m_p_end_point));
}

void
ImageViewer::mouseDoubleClickEvent ( QMouseEvent * e )
{
	if(e->button () == LeftButton && getMainWindow() )
		getMainWindow()->slotFullScreen();
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
	if(button==QMouseEvent::MidButton && !m_p_last_point)
	{
		m_is_scrolling=true;
		if (e->delta () > 0)
			scrollUp ();
		else
			scrollDown ();

	}
	else
	if(button==QMouseEvent::MidButton && m_p_last_point)
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
				.arg(m_loaded_image_url.url())
				.arg(mode);
	KRun::runCommand(com);
	setMessage(i18n("Ready"));
}



void
ImageViewer::setFit(bool a_is_fit_all, bool a_keep)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	if(a_keep)
		m_is_fit_all=a_is_fit_all;
	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;

	m_p_scaled_loaded_image=NULL;
	if(m_is_fit_all)
		fitSize();
	else
		doScale(true);
}

void
ImageViewer::setSmooth(bool s)
{
	m_is_smooth_image=s;
	doScale(true);
}


bool ImageViewer::autoRotate(bool r)
{
	KFileMetaInfo metadatas(m_loaded_image_url.path() );
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
	if(!m_p_loaded_image)
		return;

	KApplication::setOverrideCursor (waitCursor);

	QWMatrix matrix;
	if(vertical)
		matrix.scale(1,-1);
	else
	if(horizontal)
		matrix.scale(-1,1);
	*m_p_loaded_image=m_p_loaded_image->xForm(matrix);
	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;

	centerImage(false);
	placeImage(r);

	KApplication::restoreOverrideCursor ();	// restore original cursor
}


void
ImageViewer::rotateLeft(bool r)
{
	if(!m_p_loaded_image) return;
	KApplication::setOverrideCursor (waitCursor);
	QWMatrix matrix;
	matrix.rotate(-90);
	*m_p_loaded_image=m_p_loaded_image->xForm(matrix);

	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;

	centerImage(false);
	placeImage(r);

	KApplication::restoreOverrideCursor ();	// restore original cursor
}


void
ImageViewer::rotateRight(bool r)
{
	if(!m_p_loaded_image) return;
	KApplication::setOverrideCursor (waitCursor);

	QWMatrix matrix;
	matrix.rotate(90);
	*m_p_loaded_image=m_p_loaded_image->xForm(matrix);

	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;

	centerImage(false);
	placeImage(r);

	KApplication::restoreOverrideCursor ();	// restore original cursor
}


void
ImageViewer::zoomIn(float rate)
{
	if(m_scale_value<ZOOM_MAX)
	{
		KApplication::setOverrideCursor (waitCursor);

		QPoint center(width()/2, height()/2);
		center/=m_scale_value;
		center+=QPoint(getPosX(), getPosY());
		if(m_scale_value*rate>ZOOM_MAX)
			m_scale_value=ZOOM_MAX;
		else
			m_scale_value*=rate;
		centerImage((int)(center.x()*m_scale_value), (int)(center.y()*m_scale_value), true);

		KApplication::restoreOverrideCursor ();	// restore original cursor
		setZoom(m_scale_value);

		delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	}
}


void
ImageViewer::zoomOut(float rate)
{
	if(m_scale_value>1.0/ZOOM_MAX)
	{
		KApplication::setOverrideCursor (waitCursor);
		QPoint center(width()/2, height()/2);
		center/=m_scale_value;
		center+=QPoint(getPosX(), getPosY());
		if(m_scale_value/rate<=1.0/ZOOM_MAX)
			m_scale_value=float(1.0/ZOOM_MAX);
		else
			m_scale_value/=rate;
		centerImage((int)(center.x()*m_scale_value), (int)(center.y()*m_scale_value), true);
		KApplication::restoreOverrideCursor ();	// restore original cursor
		setZoom(m_scale_value);

		delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	}
}

void
ImageViewer::setZoomValue(float val)
{
	if(val>1.0/ZOOM_MAX && val<ZOOM_MAX)
	{
		KApplication::setOverrideCursor (waitCursor);
		QPoint center(width()/2, height()/2);
		center/=m_scale_value;
		center+=QPoint(getPosX(), getPosY());
		m_scale_value=val;
		centerImage((int)(center.x()*m_scale_value), (int)(center.y()*m_scale_value), true);
		KApplication::restoreOverrideCursor ();	// restore original cursor
		setZoom(m_scale_value);

		delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	}
}


void
ImageViewer::updateStatus ()
{
	if(!getMainWindow()) return;
	if (!m_p_loaded_image || (m_p_loaded_image && m_p_loaded_image->size() == QSize(0, 0)))
	{
		getMainWindow()->setZoom((int)(m_scale_value*100));
		getMainWindow()->setImagename(NULL);
		getMainWindow()->setImagetype(NULL);
		getMainWindow()->setDim(QSize(0, 0));
		getMainWindow()->setSize(-1);
		getMainWindow()->setDate(NULL);
		getMainWindow()->setImageIndex(-1);
	}
	else
	{
		if (m_loaded_image_url!=QString("(none)"))
		{
			getMainWindow()->setZoom((int)(m_scale_value*100));
			//
			QString l_fileName(m_loaded_image_url.path());
			int pos = l_fileName.findRev ("/");
			getMainWindow()->setImagename(l_fileName.right (l_fileName.length () - pos-1));
			//
			pos = l_fileName.findRev(".");
			getMainWindow()->setImagetype(m_imageType);
			//
			getMainWindow()->setDim(m_p_loaded_image->size(), (float)m_p_loaded_image->dotsPerMeterX ()/1000*25.4) ;
			
			//			
			QFileInfo fi(m_loaded_image_url.path());
			getMainWindow()->setSize(fi.size());
			
			//
			QDateTime l_dateTime;
			if(
				useEXIF() //&& 
				//m_imageType == QString::fromLatin1("JPEG")
			)
			{
				KFileMetaInfo meta( m_loaded_image_url );
				QString mDate("---");
				if( meta.isValid() )
					mDate = meta.item( "Date/time" ).string( true ).stripWhiteSpace();
				if(mDate!="---") //FIXME pointer
					l_dateTime = QDateTime(
						m_p_local->readDate(meta.item( "CreationDate" ).string( true ).stripWhiteSpace()),
						m_p_local->readTime(meta.item( "CreationTime" ).string( true ).stripWhiteSpace()));
				else
					l_dateTime = QDateTime(fi.lastModified());
			}
			else
			{
				l_dateTime = QDateTime(fi.lastModified());
			}
			getMainWindow()->setDate(l_dateTime);
			getMainWindow()->setImageIndex(imageIndex);
		}
		else
		{
			getMainWindow()->setZoom((int)(m_scale_value*100));
			getMainWindow()->setImagename("(none)");
			getMainWindow()->setImagetype("");
			getMainWindow()->setDim(QSize(0, 0));
			getMainWindow()->setSize(0);
			getMainWindow()->setDate(NULL);
		}
	}
}

int
ImageViewer::virtualScreenWidth()
{
	return (int)ceil(width()/m_scale_value);
}
int
ImageViewer::virtualScreenHeight()
{
	return (int)ceil(height()/m_scale_value);
}

int
ImageViewer::virtualPictureWidth()
{
	return (m_p_loaded_image!=NULL)?(int)ceil(m_p_loaded_image->width()*m_scale_value):0;
}
int
ImageViewer::virtualPictureHeight()
{
	return (m_p_loaded_image!=NULL)?(int)ceil(m_p_loaded_image->height()*m_scale_value):0;
}

void
ImageViewer::setVirtualPosX(double posX)
{
	m_top_pos_X=posX;
	m_real_pos_X=-(int)ceil(posX/m_scale_value);
}
void
ImageViewer::setVirtualPosY(double posY)
{
	m_top_pos_Y=posY;
	m_real_pos_Y=-(int)ceil(posY/m_scale_value);
}
int
ImageViewer::getVirtualPosX()
{
	return (int)m_top_pos_X;
}
int
ImageViewer::getVirtualPosY()
{
	return (int)m_top_pos_Y;
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
	m_real_pos_X= (int)((posX/fabs(posX)) * ceil(fabs(posX)));
}
void
ImageViewer::setPosY(double posY)
{
	m_real_pos_Y= (int)((posY/fabs(posY)) * ceil(fabs(posY)));
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
	return (int)ceil(m_real_pos_X);
}
int
ImageViewer::getPosY()
{
	return (int)ceil(m_real_pos_Y);
}
QPoint
ImageViewer::getPos()
{
	return QPoint(getPosX(), getPosY());
}

void
ImageViewer::placeImage(bool redraw)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
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
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
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
	m_scale_value=1;
	placeImage(false);
	setZoom(m_scale_value);

	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
	repaint();
}


void
ImageViewer::fitSize(bool redraw)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	if (!m_p_loaded_image)
		return;
	if (m_p_loaded_image->isNull ())
		return;

	float l_s;
	if ((((double)height()) / m_p_loaded_image->height ()) < (((double) width ()) / m_p_loaded_image->width ()))
		l_s = ((double)height()) / m_p_loaded_image->height ();
	else
		l_s = ((double) width ()) / m_p_loaded_image->width ();

	m_scale_value = l_s;
	placeImage(false);
	setZoom(m_scale_value);

	if(redraw)
		repaint();
}

void
ImageViewer::fitWidth(bool setFitWidth, bool redraw)
{
	m_is_fit_width=setFitWidth;
	m_is_fit_height=false;
	if(!m_is_fit_width)return;

	if (!m_p_loaded_image)
		return;
	if (m_p_loaded_image->isNull ())
		return;
	m_scale_value=((double)width()) / m_p_loaded_image->width ();
	placeImage(false);
	setZoom(m_scale_value);

	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
	if(redraw)
		repaint();
}

void
ImageViewer::fitHeight(bool setFitHeight, bool redraw)
{
	m_is_fit_height=setFitHeight;
	m_is_fit_width=false;
	if(!m_is_fit_height)return;

	if (!m_p_loaded_image)
		return;
	if (m_p_loaded_image->isNull ())
		return;
	m_scale_value=((double)height()) / m_p_loaded_image->height();
	placeImage(false);
	setZoom(m_scale_value);

	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
	if(redraw)
		repaint();
}

void
ImageViewer::doScale(bool repaint)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	if(!m_p_loaded_image)return;
	if (m_p_loaded_image->size () == QSize (0, 0))
		return;

	float l_s;
	if ((((double) height ()) / m_p_loaded_image->height ()) < (((double) width ()) / m_p_loaded_image->width ()))
		l_s = ((double) height ()) / m_p_loaded_image->height ();
	else
		l_s = ((double) width ()) / m_p_loaded_image->width ();

	if(m_is_fit_width)
		fitWidth(true, false);
	else
	if(m_is_fit_height)
		fitHeight(true, false);
	else
	if(!m_is_lock_zoom)
	{
		if(l_s>1 && m_is_enlarge)
			scaleFit();
		else
		if(l_s<1 && m_is_shrink)
			scaleFit();
		else
			m_scale_value=1;
	}
	placeImage(getImagePosition(), repaint);
}


void
ImageViewer::scaleFit()
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
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
		delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
		doScale(true);
	}
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
}


void
ImageViewer::setBackgroundColor(const QColor a_col)
{
	m_bg_color = a_col;
	m_bg_brush = QBrush(m_bg_color);

	QWidget::setBackgroundColor(m_bg_color);
	setBackgroundMode(NoBackground);
	repaint();
}

void
ImageViewer::paintEvent (QPaintEvent * e)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	if(!dynamic_cast<QWidget*>(parent())->isUpdatesEnabled()) return;
	if(m_p_loaded_image)
	{
		int options=ColorOnly|ThresholdDither|ThresholdAlphaDither|AvoidDither;
		if(m_drag_start_pos_X + m_drag_start_pos_Y != -2)
		{
			setVirtualPosX(m_diff_top_pos_X+m_drag_start_pos_X);
			setVirtualPosY(m_diff_top_pos_Y+m_drag_start_pos_Y);
		}

		//--------------------------------------------------------------------------///////
		QPoint rtp((int)ceil(e->rect().topLeft().x()/m_scale_value),
				(int)ceil(e->rect().topLeft().y()/m_scale_value));
		QPoint rbp((int)ceil(e->rect().bottomRight().x()/m_scale_value),
				(int)ceil(e->rect().bottomRight().y()/m_scale_value));
		QRect redraw(rtp,rbp);
		redraw.moveBy(getPosX(), getPosY());

		int
			cx=max(0,redraw.topLeft().x()),
			cy=max(0,redraw.topLeft().y()),
			cw=min(m_p_loaded_image->width(),  redraw.width() +min(0,redraw.topLeft().x())+1),
			ch=min(m_p_loaded_image->height(), redraw.height()+min(0,redraw.topLeft().y())+1);
		if(m_p_loaded_image->hasAlphaBuffer()){cw++; ch++;}

		int
			x= e->rect().topLeft().x()-min(0, (int)(ceil(redraw.topLeft().x()*m_scale_value))),
			y= e->rect().topLeft().y()-min(0, (int)(ceil(redraw.topLeft().y()*m_scale_value)));

		int
			newW=((int)ceil(cw*m_scale_value)),
			newH=((int)ceil(ch*m_scale_value));

		//--------------------------------------------------------------------------/////
		QPainter painter;
		painter.begin(this);

		if(cw>0 && ch>0)
		{
			if(cx==0 && cy==0 && m_p_scaled_loaded_image)
			{
// 				MYDEBUG<<endl;
				painter.drawImage(x,y,*m_p_scaled_loaded_image);
			}
			else
			if( (!smooth()) ||
				(m_scale_value==1.0)  ||
				(m_drag_start_pos_X + m_drag_start_pos_Y != -2) ||
				(m_p_end_point!=NULL))
			{
// 				MYDEBUG<<endl;
				QImage copiedImage=m_p_loaded_image->copy(cx, cy, cw, ch, options);
				QPixmap scaleCopy(newW, newH);
				QPainter pC(&scaleCopy);
					pC.scale(m_scale_value, m_scale_value);
					pC.drawImage(0, 0, copiedImage);
					pC.end();
				painter.drawPixmap(x, y, scaleCopy);
			}
			else
			{
// 				MYDEBUG<<endl;
				painter.drawImage (x,y, m_p_loaded_image->copy(cx, cy, cw, ch, options).smoothScale(newW, newH));
			}
		}

		if(getVirtualPosX()>0)
		{
			painter.fillRect(0, 0,
					x, height(),
					m_bg_brush);
			painter.flush();
		}
		if(getVirtualPosX()+virtualPictureWidth()<width())
		{
			painter.fillRect(getVirtualPosX()+virtualPictureWidth(), 0,
					width()-(getVirtualPosX()+virtualPictureWidth()), height(),
					m_bg_brush);
			painter.flush();
		}
		if(getVirtualPosY()>0)
		{
			painter.fillRect(0, 0,
					width(), y,
					m_bg_brush);
			painter.flush();
		}
		if(getVirtualPosY()+virtualPictureHeight()<height())
		{
			painter.fillRect(0, getVirtualPosY()+virtualPictureHeight(),
					width(), height()-(getVirtualPosY()+virtualPictureHeight()),
					m_bg_brush);
			painter.flush();
		}
		painter.flush();
		painter.end();
	}
	else
	{
		QPainter painter;
		painter.begin(this);
		painter.fillRect(0, 0,width(), height(), m_bg_brush);
		painter.end();
	}
}

void ImageViewer::setShrink(bool a_is_shrink)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	m_is_shrink = a_is_shrink;

	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
	if(m_is_shrink)
	{
		doScale(true);
	}
}

/** Read property of bool m_is_enlarge. */
const bool&
ImageViewer::getEnlarge()
{
	return m_is_enlarge;
}

/** Write property of bool m_is_enlarge. */
void
ImageViewer::setEnlarge(bool a_is_enlarge)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	m_is_enlarge = a_is_enlarge;
	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	delete(m_p_pre_loaded_image); m_p_pre_loaded_image=NULL;
	if(m_is_enlarge)
	{
		doScale(true);
	}
}

void
ImageViewer::setZoomLock(bool a_is_lock_zoom)
{
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
	m_is_lock_zoom = a_is_lock_zoom;
}

void
ImageViewer::applyFilter(int a_filter, bool activate)
{
	switch(a_filter)
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
		if(*it==QString::fromLatin1("e_grayscale")) aEffect_GRAYSCALE->setChecked(true);
		else if(*it==QString::fromLatin1("e_normalize"))aEffect_NORMALIZE->setChecked(true);
		else if(*it==QString::fromLatin1("e_equalize"))aEffect_EQUALIZE->setChecked(true);
		else if(*it==QString::fromLatin1("e_intensity"))aEffect_INTENSITY->setChecked(true);
		else if(*it==QString::fromLatin1("e_invert"))aEffect_INVERT->setChecked(true);
		else if(*it==QString::fromLatin1("e_emboss"))aEffect_EMBOSS->setChecked(true);
		else if(*it==QString::fromLatin1("e_swirl"))aEffect_SWIRL->setChecked(true);
		else if(*it==QString::fromLatin1("e_spread"))aEffect_SPREAD->setChecked(true);
		else if(*it==QString::fromLatin1("e_implode"))aEffect_IMPLODE->setChecked(true);
		else if(*it==QString::fromLatin1("e_charcoal"))aEffect_CHARCOAL->setChecked(true);
	}
}

void
ImageViewer::applyFilter()
{
	if(!m_p_loaded_image) return;
	if(m_p_loaded_image->size() == QSize(0, 0)) return;

	if(aEffect_NORMALIZE->isChecked())
		KImageEffect::normalize(*m_p_loaded_image);
	if(aEffect_EQUALIZE->isChecked())
		KImageEffect::equalize(*m_p_loaded_image);
	if(aEffect_INTENSITY->isChecked())
		*m_p_loaded_image=KImageEffect::intensity(*m_p_loaded_image, 0.5);
	if(aEffect_INVERT->isChecked())
		m_p_loaded_image->invertPixels(false);
	if(aEffect_EMBOSS->isChecked())
		*m_p_loaded_image=KImageEffect::emboss(*m_p_loaded_image);
	if(aEffect_SWIRL->isChecked())
		*m_p_loaded_image=KImageEffect::swirl(*m_p_loaded_image);
	if(aEffect_SPREAD->isChecked())
		*m_p_loaded_image=KImageEffect::spread(*m_p_loaded_image);
	if(aEffect_IMPLODE->isChecked())
		*m_p_loaded_image=KImageEffect::implode(*m_p_loaded_image);
	if(aEffect_CHARCOAL->isChecked())
		*m_p_loaded_image=KImageEffect::charcoal(*m_p_loaded_image);
	if(aEffect_GRAYSCALE->isChecked())
		*m_p_loaded_image=KImageEffect::desaturate(*m_p_loaded_image, toGrayscale()/100.0);


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
	m_drag_start_pos_X=0;
	m_drag_start_pos_Y=-ceil((double)dB);
	m_diff_top_pos_X = getVirtualPosX();
	m_diff_top_pos_Y = getVirtualPosY();
	if(!posYForTopYIsOK(m_diff_top_pos_Y+m_drag_start_pos_Y))
		m_drag_start_pos_Y = -(virtualPictureHeight() + getVirtualPosY() - height());
	bool hasDraged=(m_drag_start_pos_Y!=0);
	if(m_drag_start_pos_Y!=0)
		scroll((int)(m_drag_start_pos_X), (int)(m_drag_start_pos_Y));
	m_drag_start_pos_X=-1;
	m_drag_start_pos_Y=-1;
	return hasDraged;
}


bool
ImageViewer::scrolldyT(int dT)
{
	if (virtualPictureHeight()<=height())
		return false;
	m_drag_start_pos_X=0;
	m_drag_start_pos_Y=ceil((double)dT);
	m_diff_top_pos_X = getVirtualPosX();
	m_diff_top_pos_Y = getVirtualPosY();
	if(!posYForTopYIsOK(m_diff_top_pos_Y+m_drag_start_pos_Y))
		m_drag_start_pos_Y =  - getVirtualPosY() ;
	bool hasDraged=(m_drag_start_pos_Y!=0);
	if(m_drag_start_pos_Y!=0)
		scroll((int)(m_drag_start_pos_X), (int)(m_drag_start_pos_Y));
	m_drag_start_pos_X=-1;
	m_drag_start_pos_Y=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldyB(float coef)
{
	if (virtualPictureHeight()<=height())
		return false;
	m_drag_start_pos_X=0;
	m_drag_start_pos_Y=-ceil(coef*m_scale_value);
	m_diff_top_pos_X = getVirtualPosX();
	m_diff_top_pos_Y = getVirtualPosY();
	if(!posYForTopYIsOK(m_diff_top_pos_Y+m_drag_start_pos_Y))
		m_drag_start_pos_Y = -(virtualPictureHeight() + getVirtualPosY() - height());
	bool hasDraged=(m_drag_start_pos_Y!=0);
	if(m_drag_start_pos_Y!=0)
		scroll((int)(m_drag_start_pos_X), (int)(m_drag_start_pos_Y));
	m_drag_start_pos_X=-1;
	m_drag_start_pos_Y=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldyT(float coef)
{
	if (virtualPictureHeight()<=height())
		return false;
	m_drag_start_pos_X=0;
	m_drag_start_pos_Y=ceil(coef*m_scale_value);
	m_diff_top_pos_X = getVirtualPosX();
	m_diff_top_pos_Y = getVirtualPosY();
	if(!posYForTopYIsOK(m_diff_top_pos_Y+m_drag_start_pos_Y))
		m_drag_start_pos_Y =  - getVirtualPosY() ;
	bool hasDraged=(m_drag_start_pos_Y!=0);
	if(m_drag_start_pos_Y!=0)
		scroll((int)(m_drag_start_pos_X), (int)(m_drag_start_pos_Y));
	m_drag_start_pos_X=-1;
	m_drag_start_pos_Y=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldxL(float coef)
{
	if (virtualPictureWidth()<width())
		return false;
	m_drag_start_pos_X=ceil(coef*m_scale_value);
	m_drag_start_pos_Y=0;
	m_diff_top_pos_X = getVirtualPosX();
	m_diff_top_pos_Y = getVirtualPosY();
	if(!posXForTopXIsOK(m_diff_top_pos_X+m_drag_start_pos_X))
		m_drag_start_pos_X = -  getVirtualPosX() ;
	bool hasDraged=(m_drag_start_pos_X!=0);
	if(m_drag_start_pos_X!=0)
		scroll((int)(m_drag_start_pos_X), (int)(m_drag_start_pos_Y));
	m_drag_start_pos_X=-1;
	m_drag_start_pos_Y=-1;
	return hasDraged;
}

bool
ImageViewer::scrolldxR(float coef)
{
	if (virtualPictureWidth()<width())
		return false;
	m_drag_start_pos_X=-ceil(coef*m_scale_value);
	m_drag_start_pos_Y=0;
	m_diff_top_pos_X = getVirtualPosX();
	m_diff_top_pos_Y = getVirtualPosY();
	if(!posXForTopXIsOK(m_diff_top_pos_X+m_drag_start_pos_X))
		m_drag_start_pos_X = - (virtualPictureWidth() + getVirtualPosX() - width());
	bool hasDraged=(m_drag_start_pos_X!=0);
	if(m_drag_start_pos_X!=0)
		scroll((int)(m_drag_start_pos_X), (int)(m_drag_start_pos_Y));
	m_drag_start_pos_X=-1;
	m_drag_start_pos_Y=-1;
	return hasDraged;
}


bool ImageViewer::scrolldxRQuick()
{
	return scrolldxR(200);
}
bool ImageViewer::scrolldyBQuick()
{
	return scrolldyB(200);
}
bool ImageViewer::scrolldxLQuick()
{
	return scrolldxL(200);
}
bool ImageViewer::scrolldyTQuick()
{
	return scrolldyT(200);
}


void
ImageViewer::slotSaveImage()
{
	setMessage(i18n("Saving m_p_loaded_image..."));
	//kapp->processEvents();
	KApplication::setOverrideCursor (waitCursor);
	if(! Tools::saveAs(m_p_loaded_image, getFilename(), getFilename()) )
	{
		KApplication::restoreOverrideCursor ();
		KMessageBox::error(this, i18n("Error saving m_p_loaded_image."));
	}
	else
	{
		KApplication::restoreOverrideCursor ();
	}
	setMessage(i18n("Ready"));
}



void
ImageViewer::slotSaveAsImage()
{
	QString path;
	if(getMainWindow()) 
		if(getMainWindow()->getLastDestURL().isEmpty()) 
			path=getMainWindow()->getCurrentDir();

	QString destDir = KFileDialog::getSaveFileName(path + QFileInfo(getFilename()).fileName(),
							"*.png *.jpg *.gif *.bmp",
							this,
							i18n("Save File As"));
	if(destDir.isEmpty()) return;


	setMessage(i18n("Saving m_p_loaded_image..."));
	//kapp->processEvents();
	KApplication::setOverrideCursor (waitCursor);
	QString ext = QFileInfo(destDir).extension().upper();
	if(ext.isEmpty())
	{
		destDir+=".png";
		ext="PNG";
	}
	else
	if(ext == QString::fromLatin1("JPG"))
		ext="JPEG";

	if(! Tools::saveAs(m_p_loaded_image, getFilename(), destDir)/*m_p_loaded_image->save(destDir, ext.local8Bit(), 100)*/ )
	{
		KApplication::restoreOverrideCursor ();
		KMessageBox::error(this, i18n("Error Saving Image."));
	}
	else
	{
		KApplication::restoreOverrideCursor ();
	}

	setMessage(i18n("Ready"));
	if(getMainWindow())
	{
		KURL l_url;
		l_url.setPath(destDir);
		getMainWindow()->setLastDestURL(l_url);
	}
}

void
ImageViewer::setMessage(const QString& msg)
{
	emit sigSetMessage(msg);
}

void
ImageViewer::setZoom(float zoom)
{
	if(getMainWindow()) getMainWindow()->setZoom(zoom*100);
}


QColor
ImageViewer::bgColor()
{
	return m_bg_color;
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
	m_nbr_images=config->readNumEntry("m_nbr_images", 0);
	setImagePosition((ImageViewer::ImagePosition)config->readNumEntry("imagePosition", (int)Centered));
	setUseEXIF(config->readBoolEntry("useEXIF", true));

	config->setGroup("Zoom");
	aShrink->setChecked(config->readBoolEntry("m_is_shrink", true));
	aEnlarge->setChecked(config->readBoolEntry("m_is_enlarge", false));
	aZoomLock->setChecked(config->readBoolEntry("m_is_lock_zoom", false));

	aZoomFitWidth->setChecked(config->readBoolEntry( "m_is_fit_all width", false ));
	aZoomFitHeight->setChecked(config->readBoolEntry( "m_is_fit_all height", false ));

	//
	if(aZoomFitWidth->isChecked())
		slotfitWidth();
	else
	if(aZoomFitHeight->isChecked())
		slotfitHeight();

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
	config->writeEntry( "m_nbr_images", m_nbr_images );
	config->writeEntry( "imagePosition", getImagePosition() );

	config->setGroup("Zoom");
	config->writeEntry( "m_is_shrink", aShrink->isChecked() );
	config->writeEntry( "m_is_enlarge", aEnlarge->isChecked() );
	config->writeEntry( "m_is_lock_zoom", aZoomLock->isChecked() );
	config->writeEntry( "m_is_fit_all width", aZoomFitWidth->isChecked() );
	config->writeEntry( "m_is_fit_all height", aZoomFitHeight->isChecked() );

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
#if IMAGEVIEWER_DEBUG > 0
	MYDEBUG<<endl;
#endif
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
	if(
		!m_p_loaded_image ||
		!aEnlarge->isChecked())
	{
		repaint();
		return;
	};

	float l_s;
	if ((((double) height ()) / m_p_loaded_image->height ()) < (((double) width ()) / m_p_loaded_image->width ()))
		l_s = ((double) height ()) / m_p_loaded_image->height ();
	else
		l_s = ((double) width ()) / m_p_loaded_image->width ();
	if(l_s<1)
		slotZoom();
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
	if(
		!m_p_loaded_image ||
		!aEnlarge->isChecked())
	{
		repaint();
		return;
	};
	float l_s;
	if ((((double) height ()) / m_p_loaded_image->height ()) < (((double) width ()) / m_p_loaded_image->width ()))
		l_s = ((double) height ()) / m_p_loaded_image->height ();
	else
		l_s = ((double) width ()) / m_p_loaded_image->width ();
	if(l_s>1)
		slotZoom();
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

/**
  * Formula from digikam plugins
**/
void
ImageViewer::removeRedEye()
{
	int   r,g,b,a;

	struct channel {
		float red_gain;
		float green_gain;
		float blue_gain;
	};
	channel red_chan, green_chan, blue_chan;

	red_chan.red_gain   = 0.1;
	red_chan.green_gain = 0.6 ;
	red_chan.blue_gain  = 0.3 ;

	green_chan.red_gain   = 0.0;
	green_chan.green_gain = 1.0;
	green_chan.blue_gain  = 0.0;

	blue_chan.red_gain   = 0.0;
	blue_chan.green_gain = 0.0;
	blue_chan.blue_gain  = 1.0;

	float red_norm, green_norm, blue_norm;

	red_norm = 1.0/(red_chan.red_gain + red_chan.green_gain + red_chan.blue_gain);
	green_norm = 1.0/(green_chan.red_gain + green_chan.green_gain + green_chan.blue_gain);
	blue_norm = 1.0/(blue_chan.red_gain + blue_chan.green_gain + blue_chan.blue_gain);

	int r1, g1, b1;

	int   w    = QMIN((int)(getPosX() + width()/m_scale_value), m_p_loaded_image->width());
	int   h    = QMIN((int)(getPosY() + height()/m_scale_value), m_p_loaded_image->height());

	for (int yi = QMAX(getPosY(), 0); yi<h; yi++) for (int xi = QMAX(getPosX(), 0); xi<w; xi++)
	{
		uint* ptr  = (uint * )m_p_loaded_image->scanLine(yi) + xi;

		a = (*ptr >> 24) & 0xff;
		r = (*ptr >> 16) & 0xff;
		g = (*ptr >> 8)  & 0xff;
		b = (*ptr)       & 0xff;

		if ( r >= ( 2 * g) )
		{
			r1 = (int) QMIN(255, red_norm * (red_chan.red_gain   * r +
									red_chan.green_gain * g +
									red_chan.blue_gain  * b));
			b1 = (int) QMIN(255, green_norm * (green_chan.red_gain   * r +
										green_chan.green_gain * g +
										green_chan.blue_gain  * b));
			g1 = (int) QMIN(255, blue_norm * (blue_chan.red_gain   * r +
										blue_chan.green_gain * g +
										blue_chan.blue_gain  * b));
			g1=(int)(g1*0.65);

 			*ptr = QMIN(int((r-g)/150.0*255.0),255) << 24 | r1 << 16 | g1 << 8 | b1;
		}
	}
	delete(m_p_scaled_loaded_image); m_p_scaled_loaded_image=NULL;
	repaint();
}



void
ImageViewer::setImagePosition(ImageViewer::ImagePosition pos)
{
	m_current_image_position=pos;
}

ImageViewer::ImagePosition
ImageViewer::getImagePosition()
{
	return m_current_image_position;
}

void
ImageViewer::slotDisplayExifDialog()
{
#ifdef HAVE_LIBEXIV2
/*
	KExifDialog kexif(this);
	if (kexif.loadFile(m_loaded_image_url.path()))
		kexif.exec();
	else
		KMessageBox::sorry(this,
						   i18n("This item has no Exif Information."));
*/
#endif /* HAVE_LIBEXIV2 */
}

bool
ImageViewer::isScrolling()
{
	return m_is_scrolling;
}

QPixmap
ImageViewer::getPixmap()
{
	if(m_p_loaded_image)
		return m_p_pixmap_IO->convertToPixmap(*m_p_loaded_image);
	else
		return QPixmap(0,0);
}

QImage*
ImageViewer::getImage()
{
	return m_p_loaded_image;
}

int
ImageViewer::getImageWidth()
{
	if(m_p_loaded_image)
		return m_p_loaded_image->width();
	else
		return 0;
}

int
ImageViewer::getImageHeight()
{
	if(m_p_loaded_image)
		return m_p_loaded_image->height();
	else
		return 0;
}

QString
ImageViewer::getFilename()
{
	//return imageName;
	return m_loaded_image_url.url();
}

void
ImageViewer::setToGrayscale(int togray)
{
	m_grayscale_value=togray;
}

int
ImageViewer::toGrayscale()
{
	return m_grayscale_value;
}

bool
ImageViewer::useEXIF()
{
	return m_is_use_EXIF;
}

void
ImageViewer::setUseEXIF(bool use)
{
	m_is_use_EXIF = use;
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
	return m_nbr_images;
}

MainWindow * ImageViewer::getMainWindow()
{
	return &MAINWINDOW;
};


#include "imageviewer.moc"
