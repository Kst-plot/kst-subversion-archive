/***************************************************************************
                  planckIDEF.h  -  data source plugin template
                             -------------------
    begin                : Feb 17 2007
    copyright            : (C) 2007 The University of British Columbia
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

#ifndef PLANCKIDEF_H
#define PLANCKIDEF_H

#include <kstdatasource.h>
#include <fitsio.h>

typedef struct {
  double dTimeZero;
  double dTimeDelta;
  long frameLo;
  long frames;
} folderFile;

typedef struct {
  QString file;
  int table;
  int column;
} field;

class PLANCKIDEFSource : public KstDataSource {
  public:
    PLANCKIDEFSource(KConfig *cfg, const QString& filename, const QString& type);
   ~PLANCKIDEFSource();

    KstObject::UpdateType update(int = -1);
    int                   readField(double *v, const QString &field, int s, int n);
    bool                  isValidField(const QString &field) const;
    int                   samplesPerFrame(const QString &field);
    int                   frameCount(const QString& field = QString::null) const;
    QString               fileType() const;
    void                  save(QTextStream &ts, const QString& indent = QString::null);
    bool                  isEmpty() const;
    bool                  supportsHierarchy() const;
    bool                  reset();

    static bool           checkValidPlanckIDEFFile( const QString& filename );
    static bool           checkValidPlanckIDEFFolder( const QString& filename );

  private:
    static bool           isValidFilename( const QString& filename );
    static int            versionNumber( const QString& filename );

    void                  addToMetadata( fitsfile *ffits, int &iStatus );
    void                  addToFieldList( fitsfile *ffits, const int iNumCols, int &iStatus );
    bool                  initFile();
    bool                  initFolder();
    bool                  initialize();

    QDict<field>          _fields;
    double                _dTimeZero;
    double                _dTimeDelta;
    bool                  _bHasTime;
    bool                  _first;
    bool                  _isSingleFile;
    int                   _numFrames;
    int                   _numCols;
};

#endif

