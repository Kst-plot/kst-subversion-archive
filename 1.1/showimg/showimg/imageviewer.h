/***************************************************************************
                          imageviewer.h  -  description
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
class KPixmapIO;
class KPopupMenu;
class KToggleAction;
class KLocale;

class QLabel;
class QMenuBar;
class QPopupMenu;
class QWidget;

#define EFFECT_NORMALIZE 1
#define EFFECT_EQUALIZE 2
#define EFFECT_INTENSITY 3
#define EFFECT_INVERT 9
#define EFFECT_GRAYSCALE 10

#define EFFECT_EMBOSS 4
#define EFFECT_SWIRL 5
#define EFFECT_SPREAD 6
#define EFFECT_IMPLODE 7
#define EFFECT_CHARCOAL 8



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
		      MainWindow *mw = NULL,
		      const QString& name = NULL,
		      int wFlags = 0);

	virtual ~ImageViewer();

	void setMainWindow(MainWindow *mw);

	void readConfig(KConfig* config, const QString& group);
	void writeConfig(KConfig* config, const QString& group);

	void initMenu(KActionCollection *actionCollection);
	void initActions(KActionCollection *actionCollection);


	/**
	load and display an image given the full filaname
	@param fileName the full filaname of the picture to display
	 */
	bool loadImage (const QString& fileName="", int index=-1);

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
	bool preloadImage (const QString& fileName);

	/**
	      upadate the status of the mainwindow
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

	void fitHeight(bool setFitHeight=true, bool redraw=true);
	void fitWidth(bool setFitWidth=true, bool redraw=true);

	///
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

	///
	bool scrolldxR();
	bool scrolldyB();
	bool scrolldxL();
	bool scrolldyT();
	bool scrolldyB(int dB);
	bool scrolldyT(int dT);

	void slotSaveImage();
	void slotPrint();

	//
	void selectionChanged(bool selected);

	void slotSetFilter();

	void slotRotateLeft ();
	void slotRotateRight ();
	void slotMirrorH ();
	void slotMirrorV ();

	void slotDisplayExifDialog();

protected:
	void paintEvent (QPaintEvent *);
	void resizeEvent (QResizeEvent *);

	void wheelEvent (QWheelEvent * e);

	void mousePressEvent (QMouseEvent * e);
	void mouseReleaseEvent (QMouseEvent * e);
	void mouseMoveEvent (QMouseEvent * e);

	void mouseDoubleClickEvent (QMouseEvent * e);

	bool isScrolling();

protected slots:
	void movieUpdated(const QRect& area);
	void movieStatus(int status);
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

private:
	MainWindow *mw;
	ImageListView *il;

	QString filename;
	QImage *image;// the loaded image
	QImage *imageScaled;  // the preloaded scaled image
	QString imageName;
	QString imageType;
	int imageIndex;
	KPixmapIO *pIO;

	QString preimageName;
	QImage *preloadedImage;       // the preloaded image
	QImage *preloadedScaledImage; // the preloaded image

	KActionCollection *actionCollection;
	QPopupMenu *file;
	KPopupMenu *m_popup, *m_popup_openWith ;

	int grayscale_;

	QPoint *sp, *ep, *lp; //start, end, and last Points
	QBrush bgBrush;
	QColor bgColor_;
	QPixmap *pbgxpm;

	float scale;

	bool ss; //true if smooth image

	ButtonState button;
	QMovie* movie;
	void initMovie();

	bool fit, isFitWidth, isFitHeight;
	bool shrink, enlarge, lock;
	double realPosX, realPosY;
	double dragStartPosX, dragStartPosY;
	double topPosX, topPosY;
	double difTopPosX, difTopPosY;

	bool isScrolling_;
	bool hasimage;

	QLabel *status;
	int total;
	int nbImg;


	bool _useEXIF_;

	bool e_normalize, e_equalize,  e_intensity, e_emboss, e_swirl, e_spread, e_implode, e_charcoal, e_invert, e_grayscale;

	ImagePosition currentImagePosition_;

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
		*aPrint, *aSaveImage,
		*aScrollXR, *aScrollXL, *aScrollYB, *aScrollYT,
		*aWallpaper_CENTER,*aWallpaper_MOSAIC,*aWallpaper_CENTER_MOSAIC,*aWallpaper_CENTER_MAX,*aWallpaper_ADAPT,*aWallpaper_LOGO,
		*aDisplayExifDialog;

	KLocale *m_kKocale;
};


#endif //IMAGEVIEWER_H
