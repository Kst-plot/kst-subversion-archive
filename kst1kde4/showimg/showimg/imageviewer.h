/*****************************************************************************
                          imageviewer.h  -  description
                             -------------------
    begin                : Sat Dec 1 2001
    copyright            : (C) 2001-2005 by Richard Groult
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

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "showimg_export.h"

// QT
#include <qwidget.h>

// KDE
#include <ktrader.h>
#include <kurl.h>

class ImageListView;
class MainWindow;

class KAction;
class KActionCollection;
class KActionMenu;
class KLocale;
class KPixmapIO;
class KPopupMenu;
class KToggleAction;

class QLabel;
class QMenuBar;
class QPopupMenu;
class QWidget;

#define EFFECT_CHARCOAL   8
#define EFFECT_EMBOSS     4
#define EFFECT_EQUALIZE   2
#define EFFECT_GRAYSCALE 10
#define EFFECT_IMPLODE    7
#define EFFECT_INTENSITY  3
#define EFFECT_INVERT     9
#define EFFECT_NORMALIZE  1
#define EFFECT_SPREAD     6
#define EFFECT_SWIRL      5



class SHOWIMGCORE_EXPORT ImageViewer:public QWidget
{
	Q_OBJECT


public:
	enum ImagePosition
	{
		TopLeft = 0,
		TopCentered,
		TopRight,
		RightCentered,
		Centered,
		LeftCentered,
		BottomLeft,
		BottomCentered,
		BottomRight
	};


	/**
	      create the imageViewer
	      @param parent the parent widget
	      @param ImageListView the list of the images
	      @param MainWindow the mainwindow
	      @param name
	      @param wFlags
	*/
	ImageViewer (QWidget * parent = NULL,
		      const QString& name = NULL,
		      int wFlags = 0);

	virtual ~ImageViewer();

	void setMainWindow(MainWindow *a_p_mw);

	void readConfig(KConfig* config, const QString& group);
	void writeConfig(KConfig* config, const QString& group);

	void initMenu(KActionCollection *a_p_actionCollection);
	void initActions(KActionCollection *a_p_actionCollection);


	/**
	load and display an image given the full filaname
	@param fileName the full filaname of the picture to display
	 */
	bool openURL (const KURL& a_url, const QString & a_mimetype=QString::null);

	/**
	      set the grayscale level
	      @param togray between 0 and 100
	*/
	void setToGrayscale(int togray);
	int toGrayscale();


	void reload();

	/**
	      preload in memory an image given the full filaname
	      @param fileName the full filaname of the picture to load
	*/
	bool preopenURL (const KURL& a_url, const QString & a_mimetype=QString::null);

	/**
	      upadate the status_label of the mainwindow
	*/
	void updateStatus ();

	/**
	      @return true if an image is loaded
	*/
	bool hasImage();

	/**
	      flip or miror the diplayed image
	*/
	void mirror (bool horizontal, bool vertical, bool r=true);

	/**
	      rotate to the left
	*/
	void rotateLeft(bool r=true);
	/**
	      rotate to the right
	*/
	void rotateRight(bool r=true);

	/**
	      display the image with its original size (i.e zoom=1)
	*/
	void originalSize();

	/**
	      fit or unfit the image ti the screen
	      @param fit if true, fit the image
	      @keep if true, store this param for te next images
	*/
	void setFit(bool fit, bool keep=false);

	/**
	      set smooth for the next image, required for memory and time
	*/
	void setSmooth(bool s);

	/**
	      zoom into the image given the rate
	*/
	void zoomIn(float rate);

	/**
	      zoom out the image given the rate
	*/
	void zoomOut(float rate);

	/**
	      zoom into the image given the value
	*/
	void setZoomValue(float val);

	/**
	      if true, store the zoom rate
	*/
	void setZoomLock(bool lock);

	/**
	      scroll the image of dx and dy pixels
	*/
	void scroll ( int dx, int dy ) ;

	bool scrollUp ();
	bool scrollDown ();

	void applyFilter(int filter, bool activate);
	void applyFilter();
	void applyFilterPreloaded();

	void setImagePosition(ImagePosition pos);
	ImagePosition getImagePosition();

	/**
	      @return a pixmap of the displayed image
	*/
	QPixmap getPixmap();
	int getImageWidth();
	int getImageHeight();

	/**
	      @return a pixmap of the displayed image
	*/
	QImage* getImage();

	/**
	      @return the fullname of the displayed image
	*/
	QString getFilename();

	/**
	      @param shrink if true, shrink the image if it's larger than the screen
	*/
	void setShrink(bool shrink);

	/**
	      @param enlarge if true, enlarge the image if it's smaller than the screen
	*/
	void setEnlarge( bool enlarge);
	const bool& getEnlarge();

	/**
	      set the background color
	*/
	void setBackgroundColor(const QColor col);

	bool smooth () const;

	void setMessage(const QString& msg);
	void setZoom(float zoom);

	void setFilterList(const QStringList& list);
	QStringList getFilterList();


	void setUseEXIF(bool use);
	bool useEXIF();

	QColor bgColor();

	int getNbImg();

	/* update actions according to the current displayed image*/
	void updateActions();

	void fitWidth(bool setFitWidth=true, bool redraw=true);
	void fitHeight(bool setFitHeight=true, bool redraw=true);

signals:
	void sigSetMessage(const QString&);

	void loaded(const KURL&);

	void askForPreviousImage();
	void askForNextImage();
	void askForFirstImage();
	void askForLastImage();

public slots:
	void wallpaperC();
	void wallpaperM();
	void wallpaperCM();
	void wallpaperCMa();
	void wallpaperA();
	void wallpaperL();

	void wallpaper (int mode=1);

	//
	void slotfitWidth();
	void slotfitHeight();
	void slotZoomIn();
	void slotZoomOut();
	void slotZoom();
	void slotZoomNo();

	void slotShrink();
	void slotEnlarge();
	void slotZoomLock();
	void slotSmooth();

	//
	bool scrolldxR(float coef=10.0);
	bool scrolldyB(float coef=10.0);
	bool scrolldxL(float coef=10.0);
	bool scrolldyT(float coef=10.0);
	bool scrolldxRQuick();
	bool scrolldyBQuick();
	bool scrolldxLQuick();
	bool scrolldyTQuick();
	bool scrolldyB(int dB);
	bool scrolldyT(int dT);

	void slotSaveImage();
	void slotSaveAsImage();
	void slotPrint();

	//
	void selectionChanged(bool selected);

	void slotSetFilter();

	void slotRotateLeft ();
	void slotRotateRight ();
	void slotMirrorH ();
	void slotMirrorV ();

	void slotDisplayExifDialog();

	void removeRedEye();

protected:
	void paintEvent (QPaintEvent *);
	void resizeEvent (QResizeEvent *);

	void wheelEvent (QWheelEvent * e);

	void mousePressEvent (QMouseEvent * e);
	void mouseReleaseEvent (QMouseEvent * e);
	void mouseMoveEvent (QMouseEvent * e);

	void mouseDoubleClickEvent (QMouseEvent * e);

	bool isScrolling();

	MainWindow * getMainWindow() ;

protected slots:
	void movieUpdated(const QRect& area);
	void movieStatus(int a_p_status_label);
	void startMovie();

	void next();
	void previous();
	void last();
	void first();

private:
	bool autoRotate(bool r=false);

	int virtualScreenWidth();
	int virtualScreenHeight();

	int virtualPictureWidth();
	int virtualPictureHeight();

	void setVirtualPosX(double posX);
	void setVirtualPosY(double posY);
	int getVirtualPosX();
	int getVirtualPosY();
	QPoint getVirtualPos();
	void setVirtualPos(const QPoint pos);

	void setPosX(double posX);
	void setPosY(double posY);
	void setPos(const QPoint pos);
	int getPosX();
	int getPosY();
	QPoint getPos();

	bool posXForTopXIsOK(double posX);
	bool posYForTopYIsOK(double posY);

	void placeImage(bool redraw=false);
	void placeImage(ImagePosition pos, bool redraw=false);
	void centerImage(int posX, int posY, bool redraw=false);
	void centerImage(bool redraw=false);
	void centerXImage(bool redraw=false);
	void centerYImage(bool redraw=false);

	void doScale (bool repaint=true);
	void scaleFit();
	void fitSize(bool redraw=true);
	void scalePreload ();
	bool reconvertImage ();

	void initMovie();


private:
	ImageListView
		*m_p_il;

	KURL
		m_loaded_image_url,
		m_pre_loaded_image_url;
	QImage
		* m_p_loaded_image,            // the loaded image
		* m_p_scaled_loaded_image,     // the preloaded scaled image
		* m_p_pre_loaded_image,        // the preloaded image
		* m_p_pre_loaded_scaled_image; // the preloaded image

	QPoint
		* m_p_start_point,
		* m_p_end_point,
		* m_p_last_point; //start, end, and last Points

	KPixmapIO
		* m_p_pixmap_IO;
	QPixmap
		* m_p_bg_pixmap;

	KPopupMenu
		* m_p_popup,
		* m_p_popup_openWith ;

	KActionCollection
		* m_p_actionCollection;

	QMovie
		* m_p_movie;

	QString
		m_imageType;

	int
		imageIndex;

	int
		m_grayscale_value;

	QBrush m_bg_brush;
	QColor m_bg_color;

	float m_scale_value;

	bool m_is_smooth_image; //true if smooth image

	ButtonState button;
	int nbrMU;

	bool
		m_is_fit_all,
		m_is_fit_width,
		m_is_fit_height;
	bool
		m_is_shrink,
		m_is_enlarge,
		m_is_lock_zoom;

	double
		m_real_pos_X, m_real_pos_Y,
		m_drag_start_pos_X, m_drag_start_pos_Y,
		m_top_pos_X, m_top_pos_Y,
		m_diff_top_pos_X, m_diff_top_pos_Y;

	bool
		m_is_scrolling,
		m_has_image,
		m_is_use_EXIF;

	int
		m_total_images,
		m_nbr_images;

	bool
		e_normalize,
		e_equalize,
		e_intensity,
		e_emboss,
		e_swirl,
		e_spread,
		e_implode,
		e_charcoal,
		e_invert,
		e_grayscale;

	ImagePosition
		m_current_image_position;

	KToggleAction
		*aEnlarge, *aShrink,
		*aZoomLock, *aZoomFitWidth, *aZoomFitHeight,
		*aSmooth,

		*aEffect_NORMALIZE,*aEffect_EQUALIZE,*aEffect_INTENSITY, *aEffect_INVERT, *aEffect_GRAYSCALE,
		*aEffect_EMBOSS,*aEffect_SWIRL,*aEffect_SPREAD,*aEffect_IMPLODE,*aEffect_CHARCOAL,
		*aEffect_NONE;
	KAction
		*aPrevious,*aNext,*aFirst,*aLast,
		*aZoomIn,*aZoomOut,*aZoomNo,*aZoomFit,
		*aRotLeft,*aRotRight,*aHMirror,*aVMirror,
		*aPrint, *aSaveImage, *aSaveAsImage,
		*aScrollXR, *aScrollXL, *aScrollYB, *aScrollYT,
		*aScrollXRQuick, *aScrollXLQuick, *aScrollYBQuick, *aScrollYTQuick,
		*aWallpaper_CENTER,*aWallpaper_MOSAIC,*aWallpaper_CENTER_MOSAIC,*aWallpaper_CENTER_MAX,*aWallpaper_ADAPT,*aWallpaper_LOGO,
		*aDisplayExifDialog;

	KActionMenu
		* m_p_actionEffects_menu,
		* m_p_actionOrientation_menu;

	KLocale *m_p_local;
};


#endif //IMAGEVIEWER_H
