/***************************************************************************
                          tools.cpp  -  description
                             -------------------
    begin                : Jan 22 2005
    copyright            : (C) 2004 by Richard Groult
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

#include "tools.h"

#include "mainwindow.h"
#include "fileiconitem.h"
#include "imageloader.h"
#include "displaycompare.h"
#include "formatconversion.h"
#include "rename.h"

#include <stdlib.h>
#include <math.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kprogress.h>
#include <kprocess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kscan.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kimageeffect.h>
#include <kconfig.h>

#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qptrvector.h>

const char* CONFIG_BATCHRENAME =       "batch rename";

#define MYDEBUG kdDebug()<<__FILE__<<" " <<__LINE__ << " " << __FUNCTION__ << " "

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


Tools::Tools(MainWindow *mw)
	:formatConver(NULL),
	m_renameS(NULL)
{
	setName("Tools");
	this->mw=mw;
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

	if(m_renameS) m_renameS->writeConfig(config, CONFIG_BATCHRENAME);

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
	mw->setMessage(i18n("Comparisons in progress..."));
	KProgressDialog *progres =  new KProgressDialog (mw, "Comparisons",
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

	FileIconItem *item = mw->getImageListView()->firstItem ();
	int total=0;
	while (item){item = item->nextItem();total++;}
	progres->progressBar()->setTotalSteps(total);
	progres->adjustSize();
	progres->show();

	QTime debut=QTime::currentTime ();
	int current=0;
	ImageSimilarityData *is;
	item = mw->getImageListView()->firstItem ();
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
			mw->setMessage(i18n("Ready"));
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
				kapp->processEvents();
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
	mw->setMessage(i18n("Ready"));
	if(!res->isEmpty())
	{
		DisplayCompare(mw, res).exec();
	}
	else
	{
		KMessageBox::information(mw, "<qt>"+i18n("No similar files found.")+"</qt>");
	}

	delete(res); res=NULL;
	delete(listRatH);listRatH=NULL;
	delete(listRatW);listRatW=NULL;
}


void
Tools::compareFast ()
{
	mw->setMessage(i18n("Fast comparisons in progress..."));
	KProgressDialog *progres =  new KProgressDialog (mw, "Comparisons",
			i18n("Image Comparisons"),
			QString::null,
			true);
	progres->setLabel(i18n("Comparison in progress..."));
	progres->show();

	QDict < QPtrVector < QFile > >*dict = new QDict < QPtrVector < QFile > >;
	dict->setAutoDelete(true);
	QDict < QPtrVector < QFile > >*res = new QDict < QPtrVector < QFile > >;
	QPtrVector < QFile > *list;
	long total = 0;

	QString size;
	QFile *file;
	int nbrF=0;

	FileIconItem *item = mw->getImageListView()->firstItem();
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
	mw->setMessage(i18n("Ready"));
	if(!res->isEmpty())
	{
		DisplayCompare(mw, res).exec();
	}
	else
	{
		KMessageBox::information(mw, "<qt>"+i18n("No identical files found.")+"</qt>");
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

void
Tools::toolsRotateLeft()
{
	mw->setMessage(i18n("Rotating..."));
	QPtrList <QString> list;
	list.setAutoDelete (true);

	KURL::List listIm = mw->getImageListView()->selectedImageURLs();
	for(KURL::List::iterator it=listIm.begin(); it != listIm.end(); ++it)
		list.append(new QString((*it).path()));

	if(list.isEmpty())
	{
		KMessageBox::error(mw, "<qt>"+i18n("You have to select at least one file.")+"</qt>");
	}
	else
	{
// 		mw->stopWatchDir();
		mw->getImageListView()->stopLoading();

		KProgressDialog *progres =  new KProgressDialog (mw, "Rotation",
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
		bool HAVE_convert = !KStandardDirs::findExe(getConvertPath()).isNull();
		bool HAVE_jpegtran = !KStandardDirs::findExe(getJpegtranPath()).isNull();

		//
		if(!HAVE_convert)
			KMessageBox::error(mw, "<qt>"+i18n("You must install <tt>convert</tt> in order to manipulate images.")+"</qt>");
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
			progres->repaint();kapp->processEvents();
			//
			QFileInfo finfo(*name);
			if(finfo.extension().lower() == "jpg" && HAVE_jpegtran)
			{
				QString dest = locateLocal("tmp", ".showimgtmppic%1%2")
						.arg(getpid())
						.arg(finfo.fileName());

				com = QString("jpegtran -rotate 270 -copy all -outfile");
				com += " " 		+ KProcess::quote(dest);
				com += " " 		+ KProcess::quote(*name);
				com += "&& mv -f " 	+ KProcess::quote(dest);
				com += " " 		+ KProcess::quote(*name);
			}
			else
			{
				com = QString("convert -rotate 270");
				com += " " + KProcess::quote(*name);
				com += " " + KProcess::quote(*name);
			}
			proc->clearArguments();
			*proc << com;
			proc->start (KShellProcess::Block);
			kapp->processEvents ();

			ImageLoader::rotateEXIFThumbnail(*name, ImageLoader::ROT_270);
			kapp->processEvents ();
		}
		delete(progres);
		delete(proc);
	}
	mw->setMessage(i18n("Ready"));
// 	mw->startWatchDir();
	mw->slotRefresh();
}


void
Tools::toolsRotateRight()
{
	mw->setMessage(i18n("Rotating..."));
	QPtrList <QString> list;
	list.setAutoDelete (true);

	KURL::List listIm = mw->getImageListView()->selectedImageURLs();
	for(KURL::List::iterator it=listIm.begin(); it != listIm.end(); ++it)
		list.append(new QString((*it).path()));

	if(list.isEmpty())
	{
		KMessageBox::error(mw, "<qt>"+i18n("You have to select at least one file.")+"</qt>");
	}
	else
	{
// 		mw->stopWatchDir();
		mw->getImageListView()->stopLoading();

		KProgressDialog *progres =  new KProgressDialog (mw, "Rotation",
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
		bool HAVE_convert = !KStandardDirs::findExe("convert").isNull();
		bool HAVE_jpegtran = ! KStandardDirs::findExe("jpegtran").isNull();
		//
		if(!HAVE_convert)
			KMessageBox::error(mw, "<qt>"+i18n("You must install <tt>convert</tt> in order to manipulate images.")+"</qt>");
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
			progres->repaint();kapp->processEvents();
			//
			QFileInfo finfo(*name);
			if(finfo.extension().lower() == "jpg" && HAVE_jpegtran)
			{
				QString dest = locateLocal("tmp", ".showimgtmppic%1%2")
						.arg(getpid())
						.arg(finfo.fileName());

				com = QString("jpegtran -rotate 90 -copy all -outfile");
				com += " " 		+ KProcess::quote(dest);
				com += " " 		+ KProcess::quote(*name);
				com += "&& mv -f " 	+ KProcess::quote(dest);
				com += " " 		+ KProcess::quote(*name);
			}
			else
			{
				com = QString("convert -rotate 90");
				com += " " + KProcess::quote(*name);
				com += " " + KProcess::quote(*name);
			}
			proc->clearArguments();
			*proc << com;
			proc->start (KShellProcess::Block);
			kapp->processEvents ();

			ImageLoader::rotateEXIFThumbnail(*name, ImageLoader::ROT_90);
			kapp->processEvents ();
		}
		delete(progres);
		delete(proc);
	}
	mw->setMessage(i18n("Ready"));
// 	mw->startWatchDir();
	mw->slotRefresh();
}


void
Tools::convert()
{
	QPtrList <QString> list;
	list.setAutoDelete (false);

	for (FileIconItem * item = mw->getImageListView()->firstItem ();
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
		KMessageBox::error(mw, "<qt>"+i18n("You have to select at least one file.")+"</qt>");
		return;
	}

	//------------------------------------------------------------------------------
	if(!formatConver) formatConver = new FormatConversion(mw);
	formatConver->setCaption(i18n("Format Conversion of One Image", "Format Conversion of %n Images",list.count()));
	switch(formatConver->exec())
	{
		case KDialogBase::Accepted :
			break;
			default : return;
	}
	mw->setMessage(i18n("Conversion in progress..."));

	QString ext=formatConver->getType();
	QString opt=formatConver->getOptions();
	bool replace = formatConver->replace();

// 	mw->stopWatchDir();
	mw->getImageListView()->stopLoading();

	KProgressDialog *progres =  new KProgressDialog (mw, "Conversion",
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
		progres->repaint();kapp->processEvents();
		//
		proc->clearArguments();
		com = QString("convert %1 '%2' '%3'")
				.arg(opt)
				.arg(*name)
				.arg(MainWindow::getFullPath(name)+MainWindow::getFileName(name)+"."+ext);
		*proc << com;
		proc->start (KShellProcess::Block);

		if(replace && ext!=MainWindow::getFileExt(name))
		{
			FileIconItem *item = mw->getImageListView()->findItem(MainWindow::getFileName(name));
			if(item) item->suppression();
		}
	}
	delete(progres); progres=NULL;
	delete(proc); proc=NULL;
	mw->setMessage(i18n("Conversion done"));
	mw->getImageListView()->slotLoadFirst();
// 	mw->startWatchDir();
	mw->slotRefresh ();
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
	if(mw->getLastDestDir().isEmpty()) mw->setLastDestDir(mw->getCurrentDir());
	QString url=KFileDialog::getSaveFileName(mw->getLastDestDir(),
											 "*.png *.jpg *.gif *.bmp",
											 mw,
											 i18n("Save file as..."));
	if(!url.isEmpty())
	{
		QString filepath = url;
		QString ext=QFileInfo(filepath).extension().upper();
		if(ext.isEmpty())
		{
			filepath+=".png";
			ext="PNG";
		}
		else
			if(ext=="JPG")
		{
			ext="JPEG";
		}
		if(! img.save(filepath, ext.local8Bit(), 100) )
			KMessageBox::error(mw, "<qt>"+i18n("Error saving image.")+"</qt>");
		mw->setLastDestDir(QFileInfo(filepath).dirPath(true));
	}
}

void
Tools::slotScanImage()
{
#ifndef SHOWIMG_NO_SCAN_IMAGE
	KApplication::setOverrideCursor (waitCursor);

	if ( !m_scanDialog )
	{
		m_scanDialog = KScanDialog::getScanDialog(mw, "scandialog" );
		if ( !m_scanDialog ) // no scanning support installed?
		{
			KApplication::restoreOverrideCursor ();
			KMessageBox::error(mw, "<qt>"+i18n("Error while initialising scanning (no scanning support installed?)")+"</qt>");
			return;
		}

		connect( m_scanDialog, SIGNAL( finalImage( const QImage&, int )),
				 SLOT( slotScanned( const QImage&, int ) ));
	}

	if ( m_scanDialog->setup() ) // only if scanner configured/available
		m_scanDialog->show();

	KApplication::restoreOverrideCursor ();
#endif
}


void
Tools::renameSeries()
{
	KApplication::setOverrideCursor (waitCursor);

	if(!m_renameS)
	{
		m_renameS = new RenameSeries(mw, "RenameSeries");
		m_renameS->readConfig(KGlobal::config(), CONFIG_BATCHRENAME);
	}
	else
		m_renameS->clear();
	QString fullName, name;
	bool hasFiles=false;
	for (FileIconItem * item = mw->getImageListView()->firstItem (); item != 0; item = item->nextItem ())
	{
		if (item->isSelected () )
		{
			m_renameS->addFile(item->fullName());
			hasFiles=true;
		}
	}
	if(!hasFiles)
	{
		KApplication::restoreOverrideCursor ();
		KMessageBox::error(mw, "<qt>"+i18n("You have to select at least one file.")+"</qt>");
		return;
	}
	KApplication::restoreOverrideCursor ();
	if(m_renameS->exec())
	{
		mw->slotRefresh();
	}

}


#include "tools.moc"
