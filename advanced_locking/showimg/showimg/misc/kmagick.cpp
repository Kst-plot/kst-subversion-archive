//
// Copyright (c) 2000-2005 - Andrew Richards <ajr@users.sourceforge.net>
//               2003-2005 - Richard Groult <rgroult@jalix.org>
//
// This file is part of the PikView image viewer released under the GNU GPL.
// For license details see the file COPYING included with this distribution.
//

#include "kmagick.h"

#include "qxcfi.h"

#include <qimage.h>
#include <qfileinfo.h>
#include <qwidget.h>
#include <qpaintdevice.h>

#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kpixmapio.h>
#include <kapplication.h>

void
kimgio_magick_read_PSD(QImageIO *_imageio)
{
	QImage img;
	char imagename[255];
	strcpy(imagename, _imageio->fileName().ascii());
	KShellProcess *proc = new KShellProcess ();
	QString com;
	com.append (locate("appdata", "convert2png.pl"));
	com.append (" ");
	com.append (KShellProcess::quote(imagename));
	*proc <<  com;
	proc->start (KShellProcess::Block);
	img.load("/tmp/showimgFromPSD.png");
	_imageio->setImage(img);
	_imageio->setStatus(0);
}


void
kimgio_magick_write_PSD(QImageIO *)
{
	kdWarning()<<"TODO kimgio_magick_write_PSD(QImageIO *)"<<endl;
}

void
kimgio_magick_read(QImageIO *_imageio)
{
	kdWarning()<<"kimgio_magick_read" << _imageio->fileName() <<endl;
}

void
kimgio_magick_write(QImageIO *)
{
	kdWarning()<<"TODO kimgio_magick_write_JPEG(QImageIO *)"<<endl;
}

void
kimgio_magick_register(QWidget *)
{
	QImageIO::defineIOHandler("PSD", "^8BPS", 0, kimgio_magick_read_PSD, NULL);
	XCFImageFormat::registerFormat();
}

