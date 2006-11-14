/*****************************************************************************
                          tools.cpp  -  description
                             -------------------
    begin                : Jan 22 2006
    copyright            : (C) 2005 by Richard Groult
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

#include "tools.h"

//-----------------------------------------------------------------------------
#define TOOLS_DEBUG           0
//-----------------------------------------------------------------------------

// Local
#include "directoryview.h"
#include "displaycompare.h"
#include "fileiconitem.h"
#include "formatconversion.h"
#include "imageloader.h"
#include "mainwindow.h"
#include "renameseries.h"
#include "showimg_common.h"

// System
#include <stdlib.h>
#include <math.h>

#ifdef HAVE_LIBEXIF
#include "misc/jpeg-data.h"

#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#else
#ifdef __GNUC__
#warning no HAVE_LIBEXIF
#endif /* __GNUC__ */
#endif /* HAVE_LIBEXIF */

// KDE
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kprogress.h>
#include <kscan.h>
#include <kstandarddirs.h>

#if KDE_IS_VERSION(3,3,0)
#include <kactioncollection.h>
#endif

// QT
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qptrvector.h>

const char* CONFIG_BATCHRENAME =       "batch rename";

QString Tools::m_convertPath  = "convert";
QString Tools::m_jpegtranPath = "jpegtran";

QStringList Tools::m_dir_mime_type_list;
QStringList Tools::m_image_mime_type_list;
QStringList Tools::m_video_mime_type_list;
QStringList Tools::m_svg_mime_type_list;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define  PAS 32

/**
	a class to store datas to look for similaties of 2 images
 */
class ImageSimilarityData
{
	public:
		ImageSimilarityData()
		{
			avg_r=(uchar*)malloc(PAS*PAS*sizeof(uchar));
			avg_g=(uchar*)malloc(PAS*PAS*sizeof(uchar));
			avg_b=(uchar*)malloc(PAS*PAS*sizeof(uchar));
		}

		~ImageSimilarityData()
		{
			delete(avg_r);
			delete(avg_g);
			delete(avg_b);
		}

		QString filename;

		uchar *avg_r;
		uchar *avg_g;
		uchar *avg_b;

		int filled;
		float ratio;
};


Tools::Tools( ):
	m_p_formatConver(NULL),
	m_p_renameS(NULL)
{
	setName("Tools");

	connect(this, SIGNAL(sigSetMessage(const QString&)),
		getMainWindow(), SLOT(slotSetStatusBarText(const QString&)));
}

Tools::~Tools()
{
}

void
Tools::initActions(KActionCollection *actionCollection)
{
	aRenameSeries =new KAction(i18n("&Rename Series..."), "item_rename", 0, this, SLOT(renameSeries()), actionCollection, "Rename series");

	//------------------------------------------------------------------------------
	aConvert =new KAction(i18n("Format Conversion..."), 0, this, SLOT(convert()), actionCollection, "convert");
	aToolsRotateLeft=new KAction(i18n("Rotate &Left"), "rotation_acw_file", CTRL+Key_L, this, SLOT(toolsRotateLeft()), actionCollection , "aToolsRotateLeft");
	aToolsRotateRight=new KAction(i18n("Rotate &Right"), "rotation_cw_file", CTRL+Key_R, this, SLOT(toolsRotateRight()), actionCollection , "aToolsRotateRight");
	KActionMenu *actionConv = new KActionMenu( i18n("Convert"), actionCollection, "tools_conv" );
	actionConv->insert(aConvert);
	actionConv->insert(aToolsRotateLeft);
	actionConv->insert(aToolsRotateRight);

#ifndef HAVE_KIPI
	aCompareFast =new KAction(i18n("&Exact Comparison"), 0, this, SLOT(compareFast()), actionCollection, "Compare fast");
	aCompareAlmost =new KAction(i18n("&Approximate Comparison"), 0, this, SLOT(compareAlmost()), actionCollection, "Compare almost");
	KActionMenu *actionCmp = new KActionMenu( i18n("&Find Images"), BarIcon ("filefind", 16), actionCollection, "tools_campare" );
	actionCmp->insert(aCompareFast);
	actionCmp->insert(aCompareAlmost);

#ifndef SHOWIMG_NO_SCAN_IMAGE
	aScan =new KAction(i18n("Scan Image..."),  "scanner", 0, this, SLOT(slotScanImage()), actionCollection , "scanimage");
#endif

#endif /* HAVE_KIPI */

}

void
Tools::readConfig(KConfig *config)
{
	setConvertPath(config->readPathEntry("convertPath", "convert"));
	setJpegtranPath(config->readPathEntry("jpegtranPath", "jpegtran"));
}

void
Tools::writeConfig(KConfig *config)
{
	config->setGroup("Paths");
	config->writeEntry( "convertPath", getConvertPath() );
	config->writeEntry( "jpegtranPath", getJpegtranPath() );

	if(m_p_renameS) m_p_renameS->writeConfig(config, CONFIG_BATCHRENAME);

	config->sync();
}

QString
Tools::getConvertPath()
{
	return m_convertPath;
}
void
Tools::setConvertPath(const QString& path)
{
	m_convertPath = path;
}

QString
Tools::getJpegtranPath()
{
	return m_jpegtranPath;
}
void
Tools::setJpegtranPath(const QString& path)
{
	m_jpegtranPath = path;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char
Tools::getRed(QImage* im, int x, int y)
{
	return qRed(im->pixel(x, y));
}
char
Tools::getGreen(QImage* im, int x, int y)
{
	return qGreen(im->pixel(x, y));
}
char
Tools::getBlue(QImage* im, int x, int y)
{
	return qBlue(im->pixel(x, y));
}

ImageSimilarityData*
Tools::image_sim_fill_data(const QString& filename)
{
	int w, h;
	uchar *pix;
	int has_alpha;
	int p_step;

	int i,j;
	int x_inc, y_inc;
	int xs, ys;
	const int INC=1;

	QImage *pixbuf;
	ImageSimilarityData *sd = new ImageSimilarityData();
	sd->filename=filename;

	QFileInfo info(QDir::homeDirPath()+"/.showimg/cache/" + QFileInfo(filename).absFilePath()+".dat");
	if(info.exists())
	{
		QFile f(QDir::homeDirPath () + "/.showimg/cache/"+QFileInfo(filename).absFilePath()+".dat");
		if ( f.open(IO_ReadOnly) )
		{
			QDataStream s( &f );
			s >> sd->ratio;
			for(int i=0;i<PAS*PAS;i++) s >> sd->avg_r[i];
			for(int i=0;i<PAS*PAS;i++) s >> sd->avg_g[i];
			for(int i=0;i<PAS*PAS;i++) s >> sd->avg_b[i];
			f.close();
		}
		sd->filled = true;
		return sd;
	}

	pixbuf = new QImage(filename);
	if (!sd || !pixbuf) return 0L;
	KImageEffect::equalize(*pixbuf);

	w = pixbuf->width();
	h = pixbuf->height();
	pix = pixbuf->bits();
	has_alpha = pixbuf->hasAlphaBuffer();
	p_step = has_alpha ? 4 : 3;

	x_inc = w / PAS;
	y_inc = h / PAS;
	if (x_inc < 1 || y_inc < 1) return 0L;

	j = 0;
	for (ys = 0; ys < PAS; ys++)
	{
		i = 0;
		for (xs = 0; xs < PAS; xs++)
		{
			int x, y;
			int r, g, b;
			r = g = b = 0;
			for (y = j; y < j + y_inc; y+=INC)
			{
				for (x = i; x < i + x_inc; x+=INC)
				{
					r +=getRed(pixbuf, x, y);
					g +=getGreen(pixbuf, x, y);
					b +=getBlue(pixbuf, x, y);
				}
			}
			r /= x_inc * y_inc;
			g /= x_inc * y_inc;
			b /= x_inc * y_inc;

			sd->avg_r[ys * PAS + xs] = r;
			sd->avg_g[ys * PAS + xs] = g;
			sd->avg_b[ys * PAS + xs] = b;

			i += x_inc;
		}
		j += y_inc;
	}
	sd->filled = true;
	sd->ratio=((float)w)/h;
	delete(pixbuf); pixbuf=NULL;

	//////////////////////SAUVEGARDE//////////////
	QFile f(QDir::homeDirPath () + "/.showimg/cache/"+QFileInfo(filename).absFilePath()+".dat");
	KStandardDirs::makeDir(QFileInfo(f).dirPath(true));
	if ( f.open(IO_WriteOnly) )
	{
		QDataStream s( &f );
		s << sd->ratio;
		for(int i=0;i<PAS*PAS;i++) s << sd->avg_r[i];
		for(int i=0;i<PAS*PAS;i++) s << sd->avg_g[i];
		for(int i=0;i<PAS*PAS;i++) s << sd->avg_b[i];
		f.close();
	}
	return sd;
}

float
Tools::image_sim_compare(ImageSimilarityData *a, ImageSimilarityData *b)
{
	float sim;
	int i;

	if (!a || !b || !a->filled || !b->filled) return 0.0;
	sim = 0.0;
	for (i = 0; i < PAS*PAS; i++)
	{
		sim += (float)abs(a->avg_r[i] - b->avg_r[i]) / 255.0;
		sim += (float)abs(a->avg_g[i] - b->avg_g[i]) / 255.0;
		sim += (float)abs(a->avg_b[i] - b->avg_b[i]) / 255.0;
	}
	sim /= (1024.0 * 3.0);
	return 1.0 - sim;
}

float
Tools::image_sim_compare_fast(ImageSimilarityData *a, ImageSimilarityData *b, float min)
{
	float sim;
	int i,j;

	if (!a || !b || !a->filled || !b->filled) return 0.0;

	if( fabs(a->ratio - b->ratio) > 0.1 ) return 0.0;
	min = 1.0 - min;
	sim = 0.0;

	for (j = 0; j < PAS*PAS; j+= PAS)
	{
		for (i = j; i < j + PAS; i++)
		{
			sim += (float)abs(a->avg_r[i] - b->avg_r[i]) / 255.0;
			sim += (float)abs(a->avg_g[i] - b->avg_g[i]) / 255.0;
			sim += (float)abs(a->avg_b[i] - b->avg_b[i]) / 255.0;
		}
		/* check for abort, if so return 0.0 */
		if (j>PAS*PAS/3 && 1-sim/((j+1) * 3.0) <min)
		{
			return 0.0;
		}
	}
	sim /= (PAS*PAS * 3.0);

	return 1.0 - sim;
}

void
Tools::compareAlmost ()
{
	emit sigSetMessage(i18n("Comparisons in progress..."));
	KProgressDialog *progres =  new KProgressDialog (getMainWindow(), "Comparisons",
			i18n("Image Comparisons"),
			QString::null,
			true);

	QDict < QPtrVector < QFile > >*res = new QDict < QPtrVector < QFile > >;

	QPtrVector < ImageSimilarityData > *listRatW = new QPtrVector < ImageSimilarityData >;
	QPtrVector < ImageSimilarityData > *listRatH = new QPtrVector < ImageSimilarityData >;
	QPtrVector < ImageSimilarityData > *list;
	listRatW->setAutoDelete(true);
	listRatH->setAutoDelete(true);

	///////////////
	progres->setLabel(i18n("Create matrix for:"));
	progres->adjustSize();

	FileIconItem *item = getMainWindow()->getImageListView()->firstItem ();
	int total=0;
	while (item){item = item->nextItem();total++;}
	progres->progressBar()->setTotalSteps(total);
	progres->adjustSize();
	progres->show();

	QTime debut=QTime::currentTime ();
	int current=0;
	ImageSimilarityData *is;
	item = getMainWindow()->getImageListView()->firstItem ();
	while (item)
	{
		if(!item->isImage()) {item = item->nextItem (); continue;}

		progres->setLabel(
				"<qt>"
				+i18n("Create matrix for:<center>%1</center>").arg(item->fullName())
				+"</qt>");
		if ( progres->wasCancelled() )
		{
			delete (progres); progres=NULL;
			delete(res); res=NULL;
			emit sigSetMessage(i18n("Ready"));
			return;
		}
		progres->progressBar()->setProgress (++current);
		// file creation
		// file added
		if( (is=image_sim_fill_data(item->fullName()))!=NULL )
		{
			if(is->ratio>1)
				list = listRatW;
			else
				list = listRatH;
			list->resize (list->size () + 1);
			list->insert (list->size () - 1, is );
		}
		//we compuate the next one
		item = item->nextItem ();
	}
	MYDEBUG << "Time of the creation of matrices is  " << debut.msecsTo(QTime::currentTime()) << endl;
	debut=QTime::currentTime ();

	//////////////
	total=0;
	progres->progressBar()->setProgress(total);
	///////////////The files are compare
	QDict < QFile > *fait = new QDict < QFile >;
	list = listRatW;
	bool done=false;
	while(list!=NULL)
	{
		if (list->size () != 1)
		{
			progres->setLabel(i18n("Approximate comparison in progress..."));
			for (unsigned int i = 0; i < list->size (); i++)
			{
				////////PROGRESSBAR MANAGEMENT//////////
				progres->progressBar()->setProgress (++total);
// 				kapp->processEvents();
				if ( progres->wasCancelled() )
				{
					done=true;
					break;
				}
				/////CREATION of ImageSimilarityData of the first image////////
				ImageSimilarityData *i1 = list->at(i);
				if (i1 && !fait->find(i1->filename))
				{
					for (unsigned int j = i + 1; j < list->size (); j++)
					{
						/////CREATION of ImageSimilarityData os the seconde image////////
						ImageSimilarityData *i2 = list->at(j);

						///////COMPARAISON DES IMAGES ////////////
						float eq = image_sim_compare_fast(i1, i2, (float)0.88);
						if (eq>=0.88)   //files are the same
						{
							QPtrVector < QFile > *vect;
							 //they are added in the file
							if (!res->find (i1->filename))
							{
								vect = new QPtrVector < QFile >;
								vect->setAutoDelete(true);
								res->insert (i1->filename, vect);
							}
							else
							{
								vect = (QPtrVector < QFile > *)res->find(i1->filename);
							}
							vect->resize (vect->size () + 1);
							vect->insert (vect->size () - 1, new QFile(i2->filename));
							fait->insert(i2->filename, new QFile(i2->filename));
						}
					}
				}
			}
		}
		if(!done)
		{
			list = listRatH;
			done=true;
		}
		else
			list=NULL;
	}
	delete(fait); fait=NULL;
	delete (progres); progres=NULL;
	MYDEBUG << "Time of the cmp is " << debut.msecsTo(QTime::currentTime()) << endl;
	///////end/////////
	emit sigSetMessage(i18n("Ready"));
	if(!res->isEmpty())
	{
		DisplayCompare(getMainWindow(), res).exec();
	}
	else
	{
		KMessageBox::information(getMainWindow(), "<qt>"+i18n("No similar files found.")+"</qt>");
	}

	delete(res); res=NULL;
	delete(listRatH);listRatH=NULL;
	delete(listRatW);listRatW=NULL;
}


void
Tools::compareFast ()
{
	emit sigSetMessage(i18n("Fast comparisons in progress..."));
	KProgressDialog *progres =  new KProgressDialog (getMainWindow(), "Comparisons",
			i18n("Image Comparisons"),
			QString::null,
			true);
	progres->setLabel(i18n("Comparisons in progress..."));
	progres->show();

	QDict < QPtrVector < QFile > >*dict = new QDict < QPtrVector < QFile > >;
	dict->setAutoDelete(true);
	QDict < QPtrVector < QFile > >*res = new QDict < QPtrVector < QFile > >;
	QPtrVector < QFile > *list;
	long total = 0;

	QString size;
	QFile *file;
	int nbrF=0;

	FileIconItem *item = getMainWindow()->getImageListView()->firstItem();
	while (item)
	{
		if(!item->isImage())  {item = item->nextItem (); continue;}
		nbrF++;

		//creation d un file
		file = new QFile (item->fullName ());
		//on recupere la taille sous forme de chaine
		size = QString::number (QFileInfo (*file).size ());
		//si pas dans la table, on creer
		if (!dict->find (size))
		{
			list = new QPtrVector < QFile >;
			list->setAutoDelete(true);
			dict->insert (size, list);
		}
		//on recupere la liste
		list = (QPtrVector < QFile > *)dict->find (size);
		//on ajoute le file
		list->resize (list->size () + 1);
		list->insert (list->size () - 1, file);
		//on passe au suivant
		item = item->nextItem ();
	}

	/////////////
	//comparaison des fichiers
	QDictIterator < QPtrVector < QFile > >it (*dict);        // iterator for dict
	while (it.current ())
	{
		QDict < QFile > *fait = new QDict < QFile >;
		list = (QPtrVector < QFile > *)it.current ();
		if ( progres->wasCancelled() )
			break;
		progres->progressBar()->setProgress (total += list->size ());
		if ( progres->wasCancelled() )
		{
			delete (it);
			delete (progres); progres=NULL;
			delete (res); res=NULL;
			KApplication::restoreOverrideCursor ();
			return;
		}
		if (list->size () != 1)
		{
			for (unsigned int i = 0; i < list->size (); i++)
			{
				QFile *file1 = (QFile *) (list->at (i));
				if (!fait->find (file1->name()))
				{
					for (unsigned int j = i + 1; j < list->size (); j++)
					{
						QFile *file2 = (QFile *) (list->at (j));
						if (equals (file1, file2))	  //les fic sont egaux
						{
							QPtrVector < QFile > *vect;
							//on ajoute le file
							if (!res->find (file1->name ()))
							{
								vect = new QPtrVector < QFile >;
								vect->setAutoDelete(true);
								res->insert (file1->name (), vect);
							}
							else
							{
								vect = (QPtrVector < QFile > *)res->find (file1->name ());
							}
							vect->resize (vect->size () + 1);
							vect->insert (vect->size () - 1, file2);

							fait->insert(file2->name(), file2);
						}
					}
				}
			}
		}
		delete(fait); fait=NULL;
		++it;
	}

	delete (it);
	delete (progres); progres=NULL;
	emit sigSetMessage(i18n("Ready"));
	if(!res->isEmpty())
	{
		DisplayCompare(getMainWindow(), res).exec();
	}
	else
	{
		KMessageBox::information(getMainWindow(), "<qt>"+i18n("No identical files found.")+"</qt>");
	}
	delete(res); res=NULL;
}


bool
Tools::equals (QFile *f1, QFile *f2)
{
	if (QFileInfo (*f1).size () != QFileInfo (*f2).size ())
	{
		return false;
	}

	f1->open (IO_ReadOnly);
	f2->open (IO_ReadOnly);

	QDataStream s1 (f1);
	QDataStream s2 (f2);

	Q_INT8 b1, b2;
	bool eq = true;

	while (!s1.atEnd () && eq)
	{
		s1 >> b1;
		s2 >> b2;

		eq = (b1 == b2);
	}

	f1->close ();
	f2->close ();

	return eq;
}

//-----------------------------------------------------------------------------
QStringList Tools::getDirectoryMimeFilter()
{
	if(m_dir_mime_type_list.count()==0)
	{
		m_dir_mime_type_list<<"inode/directory";
		m_dir_mime_type_list<<"inode/directory-locked";
	}
	return m_dir_mime_type_list;
}


QStringList Tools::getImageMimeFilter()
{
	if(m_image_mime_type_list.count()==0)
	{
		m_image_mime_type_list<<"image/jpeg";
		m_image_mime_type_list<<"image/gif";
		m_image_mime_type_list<<"image/png";
		m_image_mime_type_list<<"image/bmp";
		m_image_mime_type_list<<"image/tiff";
		m_image_mime_type_list<<"image/xpm";
		m_image_mime_type_list<<"image/xcf";
		m_image_mime_type_list<<"image/x-xcf";
		m_image_mime_type_list<<"image/x-gimp";
		m_image_mime_type_list<<"image/x-xcf-gimp";
		m_image_mime_type_list<<"image/xcf";
		m_image_mime_type_list<<"image/ico";
		m_image_mime_type_list<<"image/pbm";
		m_image_mime_type_list<<"image/eps";
		m_image_mime_type_list<<"image/krl";
		m_image_mime_type_list<<"image/ppm";
		m_image_mime_type_list<<"image/x-psd";
		m_image_mime_type_list<<"image/xbm";
		m_image_mime_type_list<<"image/svg+xml";
		m_image_mime_type_list<<"image/svg-xml";
		m_image_mime_type_list<<"image/pgm";
	}
	return m_image_mime_type_list;
}

QStringList Tools::getVideoMimeFilter()
{
	if(m_video_mime_type_list.count()==0)
	{
		m_video_mime_type_list<<"video/mp4";
		m_video_mime_type_list<<"video/mpeg";
		m_video_mime_type_list<<"video/quicktime";
		m_video_mime_type_list<<"video/x-matroska";
		m_video_mime_type_list<<"video/mp4";
		m_video_mime_type_list<<"video/x-ms-asf";
		m_video_mime_type_list<<"video/x-msvideo";
		m_video_mime_type_list<<"video/x-ms-wmv";
		m_video_mime_type_list<<"video/x-ogm";
		m_video_mime_type_list<<"video/x-theora";
	}
	return m_video_mime_type_list;
}


QStringList Tools::getSVGMimeFilter()
{
	if(m_svg_mime_type_list.count()==0)
	{
		m_svg_mime_type_list<<"image/svg+xml";
		m_svg_mime_type_list<<"image/svg-xml";
	}
	return m_svg_mime_type_list;
}

bool
Tools::isImage(const QString& info)
{
	QFileInfo finfo(info);
	return isImage(finfo);
}


bool
Tools::isImage(const QFileInfo& a_info)
{
	bool l_isImage = false;
#if KDE_IS_VERSION(3,3,0)
	KMimeType::Ptr l_mime = KMimeType::findByPath( a_info.absFilePath(), 0, true );
	if( l_mime->is(KMimeType::defaultMimeType()) )
		l_mime = KMimeType::findByFileContent( a_info.absFilePath() );
	QStringList l_mime_list = getImageMimeFilter();
	for ( QStringList::Iterator l_mime_it = l_mime_list.begin(); l_mime_it != l_mime_list.end() && !l_isImage; ++l_mime_it )
	{
		if(l_mime->is(*l_mime_it))
			l_isImage = true;
	}
#endif
	if(!l_isImage)
		l_isImage=KImageIO::canRead(KImageIO::type(a_info.filePath ()));
	return l_isImage;
}

bool
Tools::isImage(const KURL& a_url, const QString& a_mimeType)
{
	bool l_isImage = false;
	KMimeType::Ptr l_mime =
		a_mimeType.isNull()
			? KMimeType::findByURL( a_url, 0, a_url.isLocalFile())
			: KMimeType::mimeType(a_mimeType);
	QStringList l_mime_list = getImageMimeFilter();
	for ( QStringList::Iterator l_mime_it = l_mime_list.begin(); l_mime_it != l_mime_list.end() && !l_isImage; ++l_mime_it )
	{
		if(l_mime->is(*l_mime_it))
			l_isImage = true;
	}
	return l_isImage;
}


bool
Tools::isVideo(const QString& a_info)
{
	QFileInfo finfo(a_info);
	return isVideo(finfo);
}
bool
Tools::isVideo(const QFileInfo& a_info)
{
	bool l_isVideo = false;
#if KDE_IS_VERSION(3,3,0)
	KMimeType::Ptr l_mime = KMimeType::findByPath( a_info.absFilePath(), 0, true );
	if( l_mime->is(KMimeType::defaultMimeType()) )
		l_mime = KMimeType::findByFileContent( a_info.absFilePath() );
	QStringList l_mime_list = getVideoMimeFilter();
	for ( QStringList::Iterator l_mime_it = l_mime_list.begin(); l_mime_it != l_mime_list.end() && !l_isVideo; ++l_mime_it )
	{
		if(l_mime->is(*l_mime_it))
			l_isVideo = true;
	}

#endif
	return l_isVideo;
}
bool
Tools::isVideo(const KURL& a_url, const QString& a_mimeType)
{
	bool l_isVideo = false;
	KMimeType::Ptr l_mime =
		a_mimeType.isNull()
			? KMimeType::findByURL( a_url, 0, a_url.isLocalFile())
			: KMimeType::mimeType(a_mimeType);
	QStringList l_mime_list = getVideoMimeFilter();
	for ( QStringList::Iterator l_mime_it = l_mime_list.begin(); l_mime_it != l_mime_list.end() && !l_isVideo; ++l_mime_it )
	{
		if(l_mime->is(*l_mime_it))
			l_isVideo = true;
	}
	return l_isVideo;
}

bool
Tools::isSVG(const QString& a_info)
{
	QFileInfo finfo(a_info);
	return isSVG(finfo);
}
bool
Tools::isSVG(const QFileInfo& a_info)
{
	bool l_isSVG = false;
#if KDE_IS_VERSION(3,3,0)
	KMimeType::Ptr l_mime = KMimeType::findByPath( a_info.absFilePath(), 0, true );
	if( l_mime->is(KMimeType::defaultMimeType()) )
		l_mime = KMimeType::findByFileContent( a_info.absFilePath() );
	QStringList l_mime_list = getSVGMimeFilter();
	for ( QStringList::Iterator l_mime_it = l_mime_list.begin(); l_mime_it != l_mime_list.end() && !l_isSVG; ++l_mime_it )
	{
		if(l_mime->is(*l_mime_it))
			l_isSVG = true;
	}

#endif
	return l_isSVG;
}

bool
Tools::isSVG(const KURL& a_url, const QString& a_mimeType)
{
	bool l_isSVG = false;
	KMimeType::Ptr l_mime =
		a_mimeType.isNull()
			? KMimeType::findByURL( a_url, 0, a_url.isLocalFile())
			: KMimeType::mimeType(a_mimeType);
	QStringList l_mime_list = getSVGMimeFilter();
	for ( QStringList::Iterator l_mime_it = l_mime_list.begin(); l_mime_it != l_mime_list.end() && !l_isSVG; ++l_mime_it )
	{
		if(l_mime->is(*l_mime_it))
			l_isSVG = true;
	}
	return l_isSVG;
}


bool
Tools::isJPEG(const QString& a_info)
{
	QFileInfo finfo(a_info);
	return isJPEG(finfo);
}

bool
Tools::isJPEG(const QFileInfo& a_info)
{
	bool l_isJPEG = false;
	KMimeType::Ptr l_mime = KMimeType::findByPath( a_info.absFilePath(), 0, true );
	if( l_mime->is(KMimeType::defaultMimeType()) )
		l_mime = KMimeType::findByFileContent( a_info.absFilePath() );
	l_isJPEG = l_mime->is(Tools::getJPEGMimeType());
	return l_isJPEG;
}




bool
Tools::isJPEG(const KURL& a_url, const QString& a_mimeType)
{
	bool l_isJPEG = false;
	KMimeType::Ptr l_mime =
		a_mimeType.isNull()
			? KMimeType::findByURL( a_url, 0, a_url.isLocalFile())
			: KMimeType::mimeType(a_mimeType);
	l_isJPEG = l_mime->is(getJPEGMimeType());
#if TOOLS_DEBUG > 5
	MYDEBUG
		<<" a_url="<<a_url.prettyURL()
		<<" a_mimeType="<<a_mimeType
		<< " ->l_isJPEG="<<(l_isJPEG?"true":"false")
	<<endl;
#endif
	return l_isJPEG;
}

QString
Tools::getJPEGMimeType()
{
	return "image/jpeg";
}

//-----------------------------------------------------------------------------


void
Tools::toolsRotateLeft()
{
	emit sigSetMessage(i18n("Rotating..."));
	QPtrList <QString> list;
	list.setAutoDelete (true);

	KURL::List listIm = getMainWindow()->getImageListView()->selectedImageURLs();
	for(KURL::List::iterator it=listIm.begin(); it != listIm.end(); ++it)
		list.append(new QString((*it).path()));

	if(list.isEmpty())
	{
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("You have to select at least one file.")+"</qt>");
	}
	else
	{
// 		getMainWindow()->stopWatchDir();
		getMainWindow()->getImageListView()->stopLoading();

		KProgressDialog *progres =  new KProgressDialog (getMainWindow(), "Rotation",
				i18n("Image Rotation"),
				QString::null,
				true);
		progres->progressBar()->setTotalSteps(list.count()+1);
		progres->adjustSize();
		progres->show();

		int done=0;
		QString *name;

		KShellProcess* proc = new KShellProcess ();
		QString  msg, com;

		//
		bool HAVE_convert  = !KStandardDirs::findExe(getConvertPath()).isNull();
		bool HAVE_jpegtran = !KStandardDirs::findExe(getJpegtranPath()).isNull();

		//
		if(!HAVE_convert)
			KMessageBox::error(getMainWindow(), "<qt>"+i18n("You must install <tt>convert</tt> in order to manipulate images.")+"</qt>");
		else
			for ( name=list.first(); name != 0; name=list.next() )
		{
			if ( progres->wasCancelled() ) break;
			done++;
			//
			msg = "<qt>"
					+i18n("Rotating <center><b>%1</b><br>(%2/%3)</center>")
					.arg(*name)
					.arg(done)
					.arg(list.count())
					+"</qt>";
			progres->setLabel (msg);
			progres->progressBar()->setProgress (done);
			progres->update();
// 			kapp->processEvents();
			//
			QFileInfo finfo(*name);
			if(
//				finfo.extension().lower() == QString::fromLatin1("jpg") &&
				Tools::isJPEG(finfo) &&
				HAVE_jpegtran
			)
			{
				QString dest = locateLocal("tmp", ".showimgtmppic%1%2")
						.arg(getpid())
						.arg(finfo.fileName());

				com = QString("%1 -rotate 270 -copy all -outfile").arg(KStandardDirs::findExe(getJpegtranPath()));
				com += ' ' 		+ KProcess::quote(dest);
				com += ' ' 		+ KProcess::quote(*name);
				com += "&& mv -f " 	+ KProcess::quote(dest);
				com += ' ' 		+ KProcess::quote(*name);
			}
			else
			{
				com = QString("%1 -rotate 270").arg(KStandardDirs::findExe(getConvertPath()));
				com += " " + KProcess::quote(*name);
				com += " " + KProcess::quote(*name);
			}
			proc->clearArguments();
			*proc << com;
			proc->start (KShellProcess::Block);
// 			kapp->processEvents ();

			ImageLoader::rotateEXIFThumbnail(*name, ImageLoader::ROT_270);
// 			kapp->processEvents ();
		}
		delete(progres);
		delete(proc);
	}
	emit sigSetMessage(i18n("Ready"));
// 	getMainWindow()->startWatchDir();
	getMainWindow()->slotRefresh();
}


void
Tools::toolsRotateRight()
{
	emit sigSetMessage(i18n("Rotating..."));
	QPtrList <QString> list;
	list.setAutoDelete (true);

	KURL::List listIm = getMainWindow()->getImageListView()->selectedImageURLs();
	for(KURL::List::iterator it=listIm.begin(); it != listIm.end(); ++it)
		list.append(new QString((*it).path()));

	if(list.isEmpty())
	{
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("You have to select at least one file.")+"</qt>");
	}
	else
	{
		getMainWindow()->getImageListView()->stopLoading();

		KProgressDialog *progres =  new KProgressDialog (getMainWindow(), "Rotation",
				i18n("Image Rotation"),
				QString::null,
				true);
		progres->progressBar()->setTotalSteps(list.count()+1);
		progres->adjustSize();
		progres->show();

		int done=0;
		QString *name;

		KShellProcess* proc = new KShellProcess ();
		QString  msg, com;

		//
		bool HAVE_convert = ! KStandardDirs::findExe(getConvertPath()).isNull();
		bool HAVE_jpegtran = !KStandardDirs::findExe(getJpegtranPath()).isNull();

		//
		if(!HAVE_convert)
			KMessageBox::error(getMainWindow(), "<qt>"+i18n("You must install <tt>convert</tt> in order to manipulate images.")+"</qt>");
		else
			for ( name=list.first(); name != 0; name=list.next() )
		{
			if ( progres->wasCancelled() ) break;
			done++;
			//
			msg = "<qt>"
					+i18n("Rotating <center><b>%1</b><br>(%2/%3)</center>")
					.arg(*name)
					.arg(done)
					.arg(list.count())
					+"</qt>";
			progres->setLabel (msg);
			progres->progressBar()->setProgress (done);
			progres->update();
 			kapp->processEvents();
			//
			QFileInfo finfo(*name);
			if(
				Tools::isJPEG(finfo) &&
				HAVE_jpegtran
			)
			{
				QString dest = locateLocal("tmp", ".showimgtmppic%1%2")
						.arg(getpid())
						.arg(finfo.fileName());

				com = getJpegtranPath()+QString(" -rotate 90 -copy all -outfile");
				com += ' ' 		+ KProcess::quote(dest);
				com += ' ' 		+ KProcess::quote(*name);
				com += "&& mv -f " 	+ KProcess::quote(dest);
				com += ' ' 		+ KProcess::quote(*name);
			}
			else
			{
				com = getConvertPath() + QString(" -rotate 90");
				com += " " + KProcess::quote(*name);
				com += " " + KProcess::quote(*name);
			}
			proc->clearArguments();
			*proc << com;
			proc->start (KShellProcess::Block);
// 			kapp->processEvents ();

			ImageLoader::rotateEXIFThumbnail(*name, ImageLoader::ROT_90);
// 			kapp->processEvents ();
		}
		delete(progres);
		delete(proc);
	}
	emit sigSetMessage(i18n("Ready"));
// 	getMainWindow()->startWatchDir();
	getMainWindow()->slotRefresh();
}


void
Tools::convert()
{
	QPtrList <QString> list;
	list.setAutoDelete (false);

	for (FileIconItem * item = getMainWindow()->getImageListView()->firstItem ();
			item != 0;
			item = item->nextItem ())
	{
		if (item->isSelected () && item->isImage())
		{
			list.append(new QString(item->fullName()));
		}
	}
	if(list.isEmpty())
	{
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("You have to select at least one file.")+"</qt>");
		return;
	}

	//------------------------------------------------------------------------------
	if(!m_p_formatConver) m_p_formatConver = new FormatConversion(getMainWindow());
	m_p_formatConver->setCaption(i18n("Format Conversion of One Image", "Format Conversion of %n Images",list.count()));
	switch(m_p_formatConver->exec())
	{
		case KDialogBase::Accepted :
			break;
			default : return;
	}
	emit sigSetMessage(i18n("Conversion in progress..."));

	QString ext=m_p_formatConver->getType();
	QString opt=m_p_formatConver->getOptions();
	bool replace = m_p_formatConver->replace();

// 	getMainWindow()->stopWatchDir();
	getMainWindow()->getImageListView()->stopLoading();

	KProgressDialog *progres =  new KProgressDialog (getMainWindow(), "Conversion",
			i18n("Image conversion"),
			QString::null,
			true);
	progres->progressBar()->setTotalSteps(list.count()+1);
	progres->adjustSize();
	progres->show();

	QString *name;
	int done=0;
	KShellProcess* proc = new KShellProcess ();
	//connect(proc, SIGNAL(processExited(KProcess *)), this, SLOT(slotEndConvert(KProcess *)));

	QString  msg, com;

	for ( name=list.first(); name != 0; name=list.next() )
	{
		msg = "<qt>"
				+i18n("Conversion of <b>%1</b>\n(%2/%3)")
				.arg(*name)
				.arg(done)
				.arg(list.count())
				+"</qt>";
		done++;
		//
		if ( progres->wasCancelled() ) break;
		progres->setLabel(msg);
		progres->progressBar()->setProgress(done);
		progres->update();
// 		kapp->processEvents();
		//
		proc->clearArguments();
		com = QString("convert %1 '%2' '%3'")
				.arg(opt)
				.arg(*name)
				.arg(MainWindow::getFullPath(name)+MainWindow::getFileName(name)+'.'+ext);
		*proc << com;
		proc->start (KShellProcess::Block);

		if(replace && ext!=MainWindow::getFileExt(name))
		{
			FileIconItem *item = getMainWindow()->getImageListView()->findItem(MainWindow::getFileName(name));
			if(item) item->suppression();
		}
	}
	delete(progres); progres=NULL;
	delete(proc); proc=NULL;
	emit sigSetMessage(i18n("Conversion done"));
	getMainWindow()->getImageListView()->slotLoadFirst();
// 	getMainWindow()->startWatchDir();
	getMainWindow()->slotRefresh ();
}

void
Tools::slotEndConvert(KProcess *proc)
{
	if(proc->exitStatus()!=0)
	{
		//qWarning(i18n("ERROR: during the conversion"));
	}

}

void
Tools::slotScanned( const QImage& img, int )
{
	if(getMainWindow()->getLastDestURL().isEmpty())
		getMainWindow()->setLastDestURL(getMainWindow()->getCurrentURL());
	QString l_dest =
		KFileDialog::getSaveFileName(
			getMainWindow()->getLastDestURL().url(),
			"*.png *.jpg *.gif *.bmp",
			getMainWindow(),
			i18n("Save file as...")
		);

	if(!l_dest.isEmpty())
	{
		QString filepath = l_dest;
		QString ext=QFileInfo(filepath).extension().upper();
		if(ext.isEmpty())
		{
			filepath+=".png";
			ext="PNG";
		}
		else
			if(ext==QString::fromLatin1("JPG")) ext="JPEG";

		if(! img.save(filepath, ext.local8Bit(), 100) )
			KMessageBox::error(getMainWindow(), "<qt>"+i18n("Error saving image.")+"</qt>");

		KURL l_url;
		l_url.setPath(QFileInfo(filepath).dirPath(true));
		getMainWindow()->setLastDestURL(l_url);
	}
}

void
Tools::slotScanImage()
{
#ifndef SHOWIMG_NO_SCAN_IMAGE
	KApplication::setOverrideCursor (waitCursor);

	if ( !m_p_scanDialog )
	{
		m_p_scanDialog = KScanDialog::getScanDialog(getMainWindow(), "scandialog" );
		if ( !m_p_scanDialog ) // no scanning support installed?
		{
			KApplication::restoreOverrideCursor ();
			KMessageBox::error(getMainWindow(), "<qt>"+i18n("Error while initialising scanning (no scanning support installed?)")+"</qt>");
			return;
		}

		connect( m_p_scanDialog, SIGNAL( finalImage( const QImage&, int )),
				 SLOT( slotScanned( const QImage&, int ) ));
	}

	if ( m_p_scanDialog->setup() ) // only if scanner configured/available
		m_p_scanDialog->show();

	KApplication::restoreOverrideCursor ();
#endif
}


bool
Tools::saveAs(const QImage* image, const QString& oldPath, const QString& newPath)
{
	QString ext(QFileInfo(newPath).extension().upper());
	if(ext == QString::fromLatin1("JPG")) ext="JPEG";
	if(!image->save(newPath, ext.local8Bit(), 100)) return false;
	if(ext != "JPEG") return true;


#ifdef HAVE_LIBEXIF
	/////////////
	QFile oldfile(oldPath);
	if (!oldfile.open(IO_ReadOnly))
	{
		MYWARNING << "Unable to open " << oldPath << " for reading"<<endl;
		return false;
	}
	QByteArray old_RawData = oldfile.readAll();
	if (old_RawData.size()==0)
	{
		MYWARNING << "No data available; empty file"<<endl;
		oldfile.close();
		return false;
	}

	ExifData* old_ExifData = exif_data_new_from_data((unsigned char*)old_RawData.data(), old_RawData.size());
	if (!old_ExifData)
	{
		MYWARNING << "Unable to load exif data"<<endl;
		oldfile.close();
		return false;
	}

	JPEGData* old_jpegData=jpeg_data_new_from_data((unsigned char*)old_RawData.data(), old_RawData.size());
	if (!old_jpegData)
	{
		MYWARNING << "Unable to create JPEGData object"<<endl;
		oldfile.close();
		return false;
	}
	oldfile.close();

	/////////////save EXIF
	QFile newfile(newPath);
	if (!newfile.open(IO_ReadOnly/*IO_WriteOnly*/))
	{
		MYWARNING << "Unable to open " << newPath << " for reading"<<endl;
		return false;
	}
	QByteArray  new_RawData = newfile.readAll();
	if ( new_RawData.size()==0)
	{
		MYWARNING << "No data available; empty file"<<endl;
		newfile.close();
		return false;
	}
	ExifData* new_ExifData = exif_data_new_from_data((unsigned char*)new_RawData.data(), new_RawData.size());
	if (!new_ExifData)
	{
		MYWARNING << "Unable to load exif data"<<endl;
		newfile.close();
		return false;
	}
	JPEGData* new_jpegData = jpeg_data_new_from_data((unsigned char*)new_RawData.data(), new_RawData.size());
	if (!new_jpegData)
	{
		MYWARNING << "Unable to create JPEGData object"<<endl;
		newfile.close();
		return false;
	}
	newfile.close();


	jpeg_data_set_exif_data(new_jpegData, old_ExifData);
	unsigned char* dest=0L;
	unsigned int destSize=0;
	jpeg_data_save_data(new_jpegData, &dest, &destSize);
	jpeg_data_unref(old_jpegData); jpeg_data_unref(new_jpegData);

	if (!newfile.open(IO_WriteOnly))
	{
		MYWARNING << "Unable to open " << newPath << " for writing"<<endl;
		return false;
	}
	QDataStream stream(&newfile);
	stream.writeRawBytes((char*)dest, destSize);
	free(dest);
	newfile.close();
#endif
	return true;
}


void
Tools::renameSeries()
{
	if(!getMainWindow()->getImageListView()->hasSelection())
	{
		KApplication::restoreOverrideCursor ();
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("You have to select at least one file.")+"</qt>");
		return;
	}
	KApplication::setOverrideCursor (waitCursor);
	if(!m_p_renameS)
	{
		m_p_renameS = new RenameSeries(getMainWindow(), "RenameSeries");
		m_p_renameS->readConfig(KGlobal::config(), CONFIG_BATCHRENAME);
	}
	else
		m_p_renameS->clear();

	QString fullName, name;
	bool hasFiles=false;
	for (FileIconItem * item = getMainWindow()->getImageListView()->firstItem (); item != 0; item = item->nextItem ())
	{
		if (item->isSelected () )
		{
			m_p_renameS->addFile(item->fullName());
			hasFiles=true;
		}
	}
/*	if(!hasFiles)
	{
		KApplication::restoreOverrideCursor ();
		KMessageBox::error(getMainWindow(), "<qt>"+i18n("You have to select at least one file.")+"</qt>");
		return;
	}
*/
	KApplication::restoreOverrideCursor ();
// 	getMainWindow()->getDirectoryView()->stopWatchDir();
	if(m_p_renameS->exec())
	{
		QDict<QString> renamedFiles = m_p_renameS->getRenamedFiles();
		getMainWindow()->updateDB(renamedFiles);
// 		getMainWindow()->slotRefresh();
	}
// 	getMainWindow()->getDirectoryView()->startWatchDir();
}

MainWindow * Tools::getMainWindow()
{
	return &MAINWINDOW;
};

#include "tools.moc"
