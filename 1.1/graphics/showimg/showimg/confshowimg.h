/***************************************************************************
                          confshowimg.h -  description
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

#ifndef CONFSHOWIMG_H
#define CONFSHOWIMG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Local
#include "imageviewer.h"

// KDE
#include <kdialogbase.h>

#ifdef HAVE_KIPI
#include <libkipi/pluginloader.h>
#endif

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSlider;
class QFrame;
class QButtonGroup;
class QSpacerItem;

class KColorButton;
class KListView;
class KLineEdit;
class KFontRequester;

class ConfShowImg : public KDialogBase
{
Q_OBJECT

public:
	ConfShowImg(QWidget *parent);
	virtual ~ConfShowImg();

	void initFiling(int openType, const QString& openDir, bool showSP, bool startFS);
	void initMiscellaneous(bool smooth, bool loadfim, bool sHDir, bool sHFile, bool sDir, bool sAll, bool sprelodim);
	void initColor(const QColor bgcolor, int gray);
	void initSlideshow(int type, int time);
	void initFullscreen(bool showToolbar, bool showStatusbar);
	void initOSD(bool dispOSD, bool onTop, const QFont& font, bool sFilename, bool sFullpath, bool sDimensions, bool sComments, bool sDatetile, bool sExif);
	void initProperties(bool sMeta, bool sHexa);
	void initThumbnails(bool storeth, bool showf, bool useexif, bool wwt, bool showMT, bool showS, bool showD, bool showDim, bool showTooltip);
	void initImagePosition(ImageViewer::ImagePosition pos);
	void initPaths(const QString& cdrom, const QString& gimp, const QString& convert, const QString& jpegtran);

	QColor getColor();
	int getGrayscale();

	int getOpenDirType();
	QString getOpenDir();
	bool checkshowSP();
	bool checkstartFS();

	bool getSmooth();
	bool getShowHiddenDir();
	bool getShowHiddenFile();
	bool getLoadFirstImage();
	bool getShowDir();
	bool getShowAll();
	bool getPreloadIm();

	bool getStoreth();
	bool getShowFrame();
	bool getUseEXIF();
	bool getWordWrapIconText();
	bool getShowMimeType();
	bool getShowSize();
	bool getShowDate();
	bool getShowDimension();
	bool getShowTooltips();

	int getSlideshowTime();
	int getSlideshowType();

	void setLayout(int l);
	int getLayout();

	bool getShowToolbar();
	bool getShowStatusbar();
	bool getShowOSD();
	bool getShowOSDgetOSDOnTop();
	bool getOSDOnTop();
	bool getOSDShowFilename();
	bool getOSDShowFullpath();
	bool getOSDShowDimensions();
	bool getOSDShowComments();
	bool getOSDShowDatetime();
	bool getOSDShowExif();
	QFont getOSDFont();



	bool getShowMeta();
	bool getShowHexa();

	void applyPlugins();

	QString getcdromPath();
	QString getgimpPath();
	QString getconvertPath();
	QString getjpegtranPath();

	ImageViewer::ImagePosition getImagePosition();

protected slots:
	void slotChangeTime(int);
	void setGrayscale(int);
	void chooseDir();

	void chooseDir_cdrom();
	void chooseDir_gimp();
	void chooseDir_convert();
	void chooseDir_jpegtran();

protected:
	void addPage1();
	void addPage2();
	void addPage3();
	void addPage4();
	void addPage5();
	void addPage6();
	void addPage7();
	void addPage8();
	void addPage9();
	void addPage10();
	void addPage11();

	QFrame *page1, *page2, *page3, *page4, *page5, *page6, *page7, *page8 , *page9, *page10, *page11;

	QPixmap *image0	;
	QLineEdit* timeLine;
	QComboBox* ComboBox1;
	QGroupBox* GroupBox18;

    QGroupBox* groupBoxTab;
    QCheckBox* showMeta;
    QCheckBox* showHexa;
    QHBoxLayout* page7Layout;
    QVBoxLayout* layoutTab;
    QGridLayout* groupBoxTabLayout;
    QVBoxLayout* layoutCheckBoxTab;




	QLabel* TextLabel4;

///Colors
    QButtonGroup* colorButtonGroup2;
    QRadioButton* ColorRadioButton5;
    QPushButton* PushButton1;
    QRadioButton* RadioButton4;
    KColorButton* color;
    QGroupBox* colorGroupBox6;
    QCheckBox* grayCheck;
    QSlider* graySlider;
    QLabel* PixmapLabel1;

    QVBoxLayout* ColorsLayout;
    QSpacerItem* colorspacer2;
    QGridLayout* colorButtonGroup2Layout;
    QSpacerItem* colorspacer1;
    QGridLayout* colorGroupBox6Layout;
    QSpacerItem* spacer3;




//Slideshow
    QButtonGroup* ButtonGroup3;
    QRadioButton* forward;
    QRadioButton* backward;
    QRadioButton* random;
    QFrame* Line1;
    QCheckBox* wraparound;
    QGroupBox* GroupBox2;
    QSlider* timeSlide;
    QLabel* timeLabel;

    QVBoxLayout* SlideShowLayout;
    QSpacerItem* spacer16;
    QHBoxLayout* layout9;
    QVBoxLayout* ButtonGroup3Layout;
    QVBoxLayout* GroupBox2Layout;
    QHBoxLayout* layout1;


//Layout
    QButtonGroup* ButtonGroup2;
    QRadioButton* radioButton_4;
    QRadioButton* radioButton_1;
    QRadioButton* radioButton_3;
    QRadioButton* radioButton_2;
    QRadioButton* RadioButton5;
    QVBoxLayout* Form2Layout;
    QSpacerItem* spacer15;
    QHBoxLayout* layout10;
    QSpacerItem* spacer17;
    QGridLayout* ButtonGroup2Layout;
    QSpacerItem* spacer11;
    QSpacerItem* spacer12;
    QSpacerItem* spacer10;
    QSpacerItem* spacer13;
    QSpacerItem* spacer14;

///FullScreen
    QGroupBox* fsGroupBox;
    QCheckBox* sToolbar;
    QCheckBox* sStatusbar;
    QGroupBox* osdGroupBox;
    QCheckBox* sOSDcheckBox;
    KFontRequester* osdFontRequester;
    QGroupBox* osdTimegroupBox;
    QSlider* osdTimeSlider;
    QLabel* osdTimeTextLabel;
    QButtonGroup* osdPosbuttonGroup;
    QRadioButton* osdTopRadioButton;
    QRadioButton* osdBottomRadioButton;
    QGroupBox* osdOptionsGroupBox;
    QCheckBox* osdOptFilenameCheckBox;
    QCheckBox* osdOptFullpathCheckBox;
    QCheckBox* osdOptDimensionsCheckBox;
    QCheckBox* osdOptExifCheckBox;
    QCheckBox* osdOptCommentsCheckBox;
    QCheckBox* osdOptDatetimeCheckBox;

    QVBoxLayout* FullScreenLayoutLayout;
    QSpacerItem* osdSpacer;
    QVBoxLayout* fsGroupBoxLayout;
    QVBoxLayout* osdGroupBoxLayout;
    QHBoxLayout* osdcbfclayout;
    QVBoxLayout* osdTimegroupBoxLayout;
    QHBoxLayout* osdPosbuttonGroupLayout;
    QGridLayout* osdOptionsGroupBoxLayout;




//page1
 protected:
    QButtonGroup* GroupBox13;
    QRadioButton* openHome;
    QRadioButton* openLast;
    QRadioButton* open_custom;
    QLineEdit* LineEdit2;
    QPushButton* chooseButton;
    QCheckBox* showSP;
    QCheckBox* startFS;

protected:
    QVBoxLayout* page1Layout;
    QVBoxLayout* GroupBox8Layout;
    QVBoxLayout* GroupBox13Layout;
    QHBoxLayout* layout1_2;



//page2
protected:
    QGroupBox* groupBox3;
    QCheckBox* smoothCheck;
    QGroupBox* groupBox4;
    QCheckBox* prelodimCheck;
    QCheckBox* loadfirstimCheck;
    QGroupBox* groupBox5;
    QCheckBox* sHiddenDirCheck;
    QCheckBox* sHiddenFileCheck;
    QCheckBox* sAllCheck;
    QCheckBox* sDirCheck;

protected:
    QGridLayout* Form1Layout;
    QVBoxLayout* page2Layout;
    QHBoxLayout* layout6;
    QGridLayout* groupBox3Layout;
    QVBoxLayout* groupBox4Layout;
    QGridLayout* groupBox5Layout;

//page8
protected:
    KListView* pluginList;
    QVBoxLayout* Form1Layoutp8;
#ifdef HAVE_KIPI
    KIPI::ConfigWidget *m_Kipiconfig;
#endif

//thumbail
    QGroupBox* groupboxThumbnails;
    QCheckBox* showFrame;
    QCheckBox* storethCheck;
    QCheckBox* useEXIF;
    QCheckBox* wrapIconText;
    QGroupBox* groupBoxDetails;
    QCheckBox* showMimeType;
    QCheckBox* showSize;
    QCheckBox* showDate;
    QCheckBox* showDimension;
    QGroupBox* tooltipGroupBox;
    QCheckBox* showTooltip;

    QVBoxLayout* thumbConfigWidgetLayout;
    QSpacerItem* thumbnailConfigSpacer;
    QHBoxLayout* layoutThumb;
    QGridLayout* groupboxThumbnailsLayout;
    QHBoxLayout* layoutDetails;
    QGridLayout* groupBoxDetailsLayout;
    QVBoxLayout* tooltipGroupBoxLayout;



//page10
public:
    QGroupBox* cdromgroupBox;
    QLabel* cdromLabel;
    KLineEdit* cdromPath;
    QPushButton* cdromPathBrowser;
    QGroupBox* externalProgramsGroupBox;
    QLabel* gimpLabel;
    KLineEdit* gimpPath;
    QPushButton* gimpPathBrowser;
    QFrame* externalProgramsLine;
    QLabel* convertLabel;
    KLineEdit* convertPath;
    QPushButton* convertPathBrowser;
    QLabel* jpegtranLabel;
    KLineEdit* jpegtranPath;
    QPushButton* jpegtranPathBrowser;

protected:
    QVBoxLayout* ExternalProgramsLayout;
    QVBoxLayout* cdromgroupBoxLayout;
    QHBoxLayout* cdromLayout;
    QGridLayout* externalProgramsGroupBoxLayout;
    QHBoxLayout* gimpLayout;
    QHBoxLayout* convertLayout;
    QHBoxLayout* jpegtranLayout;

//page11
public:
    QButtonGroup* ImagePositionGroup;
    QRadioButton* centeredRadioButton;
    QRadioButton* otherRadioButton;
    QButtonGroup* chosePosButtonGroup;
    QRadioButton* rightBut;
    QRadioButton* bottomLeftBut;
    QRadioButton* topLeftBut;
    QRadioButton* topRightBut;
    QRadioButton* leftBut;
    QRadioButton* bottomRightBut;
    QLabel* imagePreviewPosLabel;
    QRadioButton* topBut;
    QRadioButton* bottomBut;

protected:
    QVBoxLayout* ImagePositionLayout;
    QVBoxLayout* ImagePositionGroupLayout;
    QHBoxLayout* choosePosLayout;
    QGridLayout* chosePosButtonGroupLayout;

};

#endif
