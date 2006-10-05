/***************************************************************************
                          exif.h  -  description
                             -------------------
    begin                : Dec 1999
    copyright            : (C) 1999-2005 by Matthias Wandel
                               2000-2005 by Richard Groult
    email                : mwandel@sentex.net  rgroult@jalix.org
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
 *   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301   *
 *   USA.                                                                  *
 *                                                                         *
 *   For license exceptions see LICENSE.EXC file, attached with the source *
 *   code.                                                                 *
 *                                                                         *
 ***************************************************************************/

#ifndef EXIF_H
#define EXIF_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MAX_COMMENT 1000
#ifndef TRUE
    #define TRUE 1
    #define FALSE 0
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>

#include <utime.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <qstring.h>

#ifndef _MAX_PATH
# define _MAX_PATH 300
#endif

//--------------------------------------------------------------------------
// This structure stores Exif header image elements in a simple manner
// Used to store camera data as extracted from the various ways that it can be
// stored in an exif header
typedef struct {
    char  FileName     [_MAX_PATH+1];
    time_t FileDateTime;
    unsigned FileSize;
    char  CameraMake   [32];
    char  CameraModel  [40];
    char  DateTime     [20];
    int   Height, Width;
    int   IsColor;
    int   Process;
    int   FlashUsed;
    float FocalLength;
    float ExposureTime;
    float ApertureFNumber;
    float Distance;
    float CCDWidth;
    float ExposureBias;
    int   Whitebalance;
    int   MeteringMode;
    int   ExposureProgram;
    int   ISOequivalent;
    int   CompressionLevel;
    char  Comments[MAX_COMMENT];

    unsigned char * ThumbnailPointer;  // Pointer at the thumbnail
    unsigned ThumbnailSize;     // Size of thumbnail.

}ImageInfo_t;

typedef enum {
    READ_EXIF = 1,
    READ_IMAGE = 2,
    READ_ALL = 3
}ReadMode_t;
typedef unsigned char uchar;

//--------------------------------------------------------------------------
// This structure is used to store jpeg file sections in memory.
typedef struct {
    uchar *  Data;
    int      Type;
    unsigned Size;
}Section_t;

#define MAX_SECTIONS 20
#define PSEUDO_IMAGE_MARKER 0x123; // Extra value.

extern int ReadJpegSections (FILE * infile, ReadMode_t ReadMode);
extern void DiscardData(void);
extern void process_EXIF(unsigned char * CharBuf, unsigned int length);
extern void process_COM (const uchar * Data, int length);
extern void process_SOFn (const uchar * Data, int marker);
extern void DiscardData(void);
extern int Get16m(const void * Short);
extern int GetExifNonThumbnailSize(void);
extern QString ProcessFile(const char * FileName, bool showsize=false, const char* ThumbnailName=NULL);


#endif

