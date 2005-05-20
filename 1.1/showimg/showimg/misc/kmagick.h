//
// Copyright (c) 2000-2001 - Andrew Richards <ajr@users.sourceforge.net>
//
// This file is part of the PikView image viewer released under the GNU GPL.
// For license details see the file COPYING included with this distribution.
//

#ifndef KMAGICK_H
#define KMAGICK_H

#include <qimage.h>

void kimgio_magick_read(QImageIO *image);
void kimgio_magick_write(QImageIO *image);

void kimgio_magick_register(QWidget *);

#endif
