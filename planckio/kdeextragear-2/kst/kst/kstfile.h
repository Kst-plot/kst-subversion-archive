/***************************************************************************
                          kstfile.h  -  description
                             -------------------
    begin                : Thu Aug 24 2000
    copyright            : (C) 2000 by Barth Netterfield
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTFILE_H
#define KSTFILE_H

#include <qstring.h>
#include <qtextstream.h>
#include <qdom.h>

#include "kstobject.h"
#include "plancktoi.h"

typedef enum {UNKNOWN, EMPTY, FRAME, ASCII, DIRFILE, PLANCK} KstFileType;

/**File structure for KST
  *@author Barth Netterfield
  */

class KTempFile;

class KstFile : public KstObject {
public:
  KstFile(const QString &filename_in, KstFileType newType = UNKNOWN);
  KstFile(QDomElement &e);
  virtual ~KstFile();

  /** Updates number of samples.
      For ascii files, it also reads
      and writes to a temporary binary file.
      It returns 1 if there was new data */
  virtual KstObject::UpdateType update(int = -1);

  /** Reads a field from the file.  Data is returned in the
      double Array V[] */
  int readField(double *V, const QString &field, int s, int n);

  /** Returns true if the field is valid, or false if it is not */
  bool isValidField(const QString &field) const;

  /** Returns samples per frame for field <field>.  For
      ascii column data, this is always 1.  For frame data
      this could greater than 1. */
  int samplesPerFrame(const QString &field);

  /** Returns the size of the file (in frames) as of last update */
  int numFrames() const;

  /** Returns the file name.
      The string is stored in a separate static variable, so changes
      to this are ignored.  It is updated each time the fn is called */
  QString fileName() const;

  QStringList fieldList() const;

  /** Returns the file type or an error message in a static string
      The string is stored in a separate static variable, so changes
      to this are ignored.  It is updated each time the fn is called */
  QString fileType() const;

  /** Save file description info into stream ts */
  void save(QTextStream &ts);

private: // Private methods
  /** common part of the constructor: */
  void commonConstructor(const QString &filename_in, KstFileType newType);

  /** Determines the file type */
  int determineType();

  /** Read filename from indirect file: return 1 if file changes */
  int readIFile();

  /** initialize files */
  void init();

  /** Read a field from an Ascii file.  n is number of frames, unless
   it is negative, in which case, read one sample, not entire frames */
  int asciiReadField(double *V, const QString &field, int s, int n);

  /** Update an Ascii file: read lines, write to temp_file */
  bool asciiUpdate();

  /** Initializations for ascii files: */
  bool asciiInitFile();

  /** Update a planck file */
  bool planckUpdate();

  /** Initializations for planck files: */
  bool planckInitFile();

  /** Returns true if the field is valid, or false if it is not */
  bool asciiIsValidField(const QString &field) const;

  /** Read a field from a frame file */
  int frameReadField(double *V, const QString &field, int s, int n);

  /** Update Frame Data file: determine length */
  bool frameUpdate();

  /** Initialization for Frame Files */
  bool frameInitFile();

  /** Returns true if the field is valid, or false if it is not */
  bool frameIsValidField(const QString &field) const;

  /** Determine samples per frame for Frame Files */
  int frameSampsPerFrame(const QString &field);

  /** Read a field from a dirfile file */
  int dirfileReadField(double *V, const QString &field, int s, int n);

  /** Update Dirfile Data file: determine length */
  bool dirfileUpdate();

  /** Initialization for Dirfile Files */
  bool dirfileInitFile();

  /** Returns true if the field is valid, or false if it is not */
  bool dirfileIsValidField(const QString &field) const;

  /** Determine samples per Frame for Dirfile Files */
  int dirfileSampsPerFrame(const QString &field);

  /** Read from a planck file */
  int planckReadField(double *V, const QString &field, int s, int n);

  /** Update stdin temp file - true if more data is read */
  bool UpdateStdin();

  void setFilename(QString newName);

private: // Private attributes
  /** Name of current file file */
  QString IndirectFilename;
  bool IsIndirect;

  /** Name of the File */
  QString Filename;
  QCString EncodedFilename;
  char *CFilename;

  /** File Type */
  KstFileType Type;

  QStringList _fieldList;

  /** How many RVectors are using this file */
  int NumUsed;

  /** Number of Frames available */
  int NumFrames;

  /** Used by stdin files */
  bool IsStdin;
  KTempFile *StdinFile;

  /** used by ASCII files */
  int ByteLength;
  int *RowIndex;
  int NumLinesAlloc;

  /** Used by frame files */
  int BytePerFrame;
  int FramePerFile;
  char RootFileName[255];  // Name of file without hex extension
  int RootExt;             // Root Extension (number): -1 if no extension
  int MaxExt;              // The largest value the extension

  KSharedPtr<Planck::TOI> _planck;
};

typedef KSharedPtr<KstFile> KstFilePtr;
class KstFileList : public KstObjectList<KstFilePtr> {
  public:
    KstFileList() : KstObjectList<KstFilePtr>() {}
    KstFileList(const KstFileList& x) : KstObjectList<KstFilePtr>(x) {}
    virtual ~KstFileList() {}

    virtual KstFileList::Iterator findFileName(const QString& x) {
      for (KstFileList::Iterator it = begin(); it != end(); ++it) {
        if ((*it)->fileName() == x) {
          return it;
        }
      }
      return end();
    }
};


#endif
