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
  QString file;
  long frameLo;
  long frames;
} folderField;

typedef QValueList<folderField> fileList;

typedef struct {
  QString basefile;
  int table;
  int column;
} field;

class PLANCKIDEFSource : public KstDataSource {
  public:
    PLANCKIDEFSource(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e = QDomElement());
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
    virtual QString       configuration(QString setting);
    virtual bool          setConfiguration(QString setting, const QString &value);

    bool                  supportsTimeConversions() const;
    int                   sampleForTime(const KST::ExtDateTime& time, bool *ok = 0L);
    int                   sampleForTime(double ms, bool *ok = 0L);
    KST::ExtDateTime      timeForSample(int sample, bool *ok = 0L);
    double                relativeTimeForSample(int sample, bool *ok = 0L);

    class Config;
    static bool           checkValidPlanckIDEFFile( const QString& filename, PLANCKIDEFSource::Config *config );
    static bool           checkValidPlanckIDEFFolder( const QString& filename );

  private:
    long                  getNumFrames( const QString& filename );
    long                  getNumFrames( fitsfile* ffits, int iNumHeaderDataUnits );
    int                   readFileFrames( const QString& filename, field *fld, double *v, int s, int n );
    int                   readFolderFrames( field *fld, double *v, int s, int n );
    int                   sampleForOBT(double timeIDEF, bool *ok);

    static bool           isValidFilename( const QString& filename, Config *config );
    static QString        baseFilename( const QString& filename );

    void                  addToMetadata( fitsfile *ffits, int &iStatus );
    void                  addToFieldList( fitsfile *ffits, const int iNumCols, int &iStatus );
    void                  addToFieldList( fitsfile *ffits, const QString& prefix, const QString& baseName, const int iNumCols, int &iStatus );
    bool                  initFile( const QString& filename );
    bool                  initFile();
    bool                  initFolderFile( const QString& filename, const QString& prefix, const QString& baseName );
    bool                  initFolder();
    bool                  initialize();

    friend class ConfigWidgetPlanckIDEF;
    mutable Config       *_config;
    QDict<field>          _fields;
    QDict<fileList>       _basefiles;
    double                _dTimeZero;
    double                _dTimeDelta;
    bool                  _bHasTime;
    bool                  _first;
    bool                  _isSingleFile;
    int                   _numFrames;
    int                   _numCols;
};

#endif

