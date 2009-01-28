/***************************************************************************
                    planckIDEF.cpp  - Planck IDIS DMC Exchange Format file data source
                             -------------------
    begin                : Feb 13 2007
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

#include "planckIDEF.h"
#include "planckIDEFconfig.h"

#include <ctype.h>
#include <stdlib.h>

#include <qcheckbox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstylesheet.h>

#include <kdebug.h>

#include "kststring.h"

class PLANCKIDEFSource::Config {
  public:
    Config() {
      _checkFilename = true;
    }

    void read(KConfig *cfg, const QString& fileName = QString::null) {
      Q_UNUSED( fileName );

      cfg->setGroup("PlanckIDEF General");
      _checkFilename = cfg->readBoolEntry("Check Filename", true);
    }

    bool _checkFilename;

    void save(QTextStream& str, const QString& indent) {
      if (_checkFilename) {
        str << indent << "checkfilename=\"" << _checkFilename << "\"/>";
      }
    }

    void load(const QDomElement& e) {
      _checkFilename = false;

      QDomNode n = e.firstChild();
      while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
          if (e.tagName() == "checkfilename") {
            _checkFilename = true;
          }
        }
        n = n.nextSibling();
      }
   }
};


PLANCKIDEFSource::PLANCKIDEFSource( KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e)
: KstDataSource( cfg, filename, type ), _config(0L)
{
  _fields.setAutoDelete( TRUE );

  if( type.isEmpty( ) || type == "PLANCKIDEF" )
  {
    if( initialize( ) )
    {
      _config = new PLANCKIDEFSource::Config;
      _config->read(cfg, filename);
      if (!e.isNull()) {
        _config->load(e);
      }

      _valid = true;
    }
  }
}


PLANCKIDEFSource::~PLANCKIDEFSource( )
{
  delete _config;
  _config = 0L;
}


bool PLANCKIDEFSource::isValidFilename( const QString& filename, Config *config )
{
  bool ok = false;

  if( !config || config->_checkFilename )
  {
    unsigned gzLength;

    if( filename.right( 3 ).lower( ) == ".gz" ) 
    {
      gzLength = 3;
    }
    else
    {
      gzLength = 0;
    }

    if( !ok )
    {
      //
      // check for a valid science timeline file...
      //
      // Accc-xxxx-T-yyyymmdd.fits
      //  A = one character coding the instrument identifier
      //  ccc = three character coding the obervation frequency
      //  xxxx = four digits identifying the operational day to which the data belong
      //  T = {C,R} one letter identifying if the timeline is converted or reduced
      //  yyyy = four digits coding the start timeline year
      //  mm = two digits coding the start timeline month
      //  dd = two digits coding the start timeline day
      //

      unsigned checkLength = gzLength + 25;
      if( filename.length() >= checkLength )
      {
        QString tail = filename.right( checkLength );
        char instrument;
        char frequency[3];
        char timeline;
        int operationalDay;
        int year;
        int month;
        int day;

        if( sscanf( tail.latin1(), "%c%c%c%c-%4d-%c-%4d%2d%2d.fits", 
          &instrument, &frequency[0], &frequency[1], &frequency[2], 
          &operationalDay, &timeline, &year, &month, &day ) == 9 )
        {
          if( year > 0      &&
              month >= 1    &&
              month <= 12   &&
              day >= 1      &&
              day <= 31     )
          {
            ok = true;
          }
        }
      }
    }

    if( !ok )
    {
      //
      // check for a valid HK timeline file...
      //
      // A-namefile-xxxx-yyyymmdd.fits
      //  A = one character coding the instrument identifier
      //  xxxx = four digits identifying the operational day to which the data belong
      //  yyyy = four digits coding the start timeline year
      //  mm = two digits coding the start timeline month
      //  dd = two digits coding the start timeline day
      //

      unsigned checkLength = gzLength + 19;
      if( filename.length() >= checkLength )
      {
        QString tail = filename.right( checkLength );
        int operationalDay;
        int year;
        int month;
        int day;

        if( sscanf( tail.latin1(), "-%4d-%4d%2d%2d.fits", 
          &operationalDay, &year, &month, &day ) == 4 )
        {
          if( year > 0      &&
              month >= 1    &&
              month <= 12   &&
              day >= 1      &&
              day <= 31     )
          {
            ok = true;
          }
        }
      }
    }

    if( !ok )
    {
      //
      // check for a previosuly valid definition of the filename...
      //
      // *_yyyymmddhhmm_vv.fits
      //  yyyy = four digits coding the start timeline year
      //  mm = two digits coding the start timeline month
      //  dd = two digits coding the start timeline day
      //  hh = two digits coding the start timeline hour
      //  mm = two digits coding the start timeline minute
      //  vv = version number (to be used in case of regeneration of HK timelines, starting from 00)
      //

      unsigned checkLength = gzLength + 21;
      if( filename.length() >= checkLength )
      {
        QString tail = filename.right( checkLength );
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int version;

        if( sscanf( tail.latin1(), "_%4d%2d%2d%2d%2d_%2d.fits", 
            &year, &month, &day, &hour, &minute, &version ) == 6 )
        {
          if( year > 0      &&
              month >= 1    &&
              month <= 12   &&
              day >= 1      &&
              day <= 31     )
          {
            ok = true;
          }
        }
      }
    }
  }
  else
  {
    ok = true;
  }

  return ok;
}


QString PLANCKIDEFSource::baseFilename( const QString& filename )
{
  QString base;
  unsigned minLength = 14;

  if( filename.right( 3 ).lower( ) == ".gz" ) 
  {
    minLength += 3;
  }

  //
  // -yyyymmdd.fits
  //  yyyy = four digits coding the start timeline year
  //  mm = two digits coding the start timeline month
  //  dd = two digits coding the start timeline day
  //
  if( filename.length() > minLength )
  {
    base = filename.left( filename.length() - minLength );
  }

  return base;
}


bool PLANCKIDEFSource::reset( )
{
  return true;
}


void PLANCKIDEFSource::addToMetadata( fitsfile *ffits, int &iStatus )
{
  int iResult;
  int keysexist;
  int morekeys;

  iResult = fits_get_hdrspace( ffits, &keysexist, &morekeys, &iStatus );
  if( iResult == 0 )
  {
    QString strKey;
    char keyname[FLEN_KEYWORD];
    char value[FLEN_VALUE];
    char comment[FLEN_COMMENT];
    int keynum;
    int hdu;

    fits_get_hdu_num( ffits, &hdu );

    for( keynum=1; keynum<=keysexist; ++keynum )
    {
      iResult = fits_read_keyn( ffits, keynum, keyname, value, comment, &iStatus );
      if( iResult == 0 )
      {
        strKey.sprintf("%02d_%03d %s", hdu, keynum, keyname);

        KstString *metaString;
        KstObjectTag newTag( strKey, tag( ) );
        QString str;

        if( strlen( comment ) > 0 )
        {
          if( strlen( value ) > 0 )
          {
            str.sprintf( "%s / %s", value, comment );
          }
          else
          {
            str.sprintf( "%s", comment );
          }
        }
        else if( strlen( value ) > 0 )
        {
          str.sprintf( "%s", value );
        }

        metaString = new KstString( newTag, this, str );
        _metaData.insert( keyname, metaString );
      }
    }
  }
}


void PLANCKIDEFSource::addToFieldList( fitsfile *ffits, const int iNumCols, int &iStatus )
{
  QString str;
  char charTemplate[ FLEN_CARD ];
  char charName[ FLEN_CARD ];
  char headerName[ FLEN_CARD ];
  long lRepeat;
  long lWidth;
  int iHDUNumber;
  int iTypeCode;
  int iColNumber;
  int iResult;
  int table;
  int col;

  table = fits_get_hdu_num( ffits, &iHDUNumber );

  iResult = fits_read_key_str( ffits, "EXTNAME", headerName, NULL, &iStatus );
  iStatus = 0;

  for( col=0; col<iNumCols; col++ )
  {
    iResult = fits_get_coltype( ffits, col+1, &iTypeCode, &lRepeat, &lWidth, &iStatus );
    if( iResult == 0 )
    {
      sprintf( charTemplate, "%d", col+1 );

      if( fits_get_colname( ffits, CASEINSEN, charTemplate, charName, &iColNumber, &iStatus ) == 0 )
      {
        if( lRepeat == 1 )
        {
          field *fld = new field;

          str = QString( "%1" ).arg( charName );

          if( strcmp( charName, "FLAG" ) == 0 )
          {
            str = QString( "%1_%2" ).arg( charName ).arg( headerName );
          }

          if( _fields.find( str ) != 0 )
          {
            str = QString( "%1_%2" ).arg( charName ).arg( iHDUNumber-1 );
          }

          fld->basefile = QString("");
          fld->table = table;
          fld->column = iColNumber;

          _fields.insert( str, fld );
          _fieldList.append( str );
        }
      }
    }
  }
}


void PLANCKIDEFSource::addToFieldList( fitsfile *ffits, const QString& prefix, const QString& baseName, const int iNumCols, int &iStatus )
{
  QString str;
  char charTemplate[ FLEN_CARD ];
  char charName[ FLEN_CARD ];
  long lRepeat;
  long lWidth;
  int iHDUNumber;
  int iTypeCode;
  int iColNumber;
  int iResult;
  int table;
  int col;

  table = fits_get_hdu_num( ffits, &iHDUNumber );

  for( col=0; col<iNumCols; col++ )
  {
    iResult = fits_get_coltype( ffits, col+1, &iTypeCode, &lRepeat, &lWidth, &iStatus );
    if( iResult == 0 )
    {
      sprintf( charTemplate, "%d", col+1 );

      if( fits_get_colname( ffits, CASEINSEN, charTemplate, charName, &iColNumber, &iStatus ) == 0 )
      {
        if( lRepeat == 1 )
        {
          field *fld = new field;

          if( prefix.isEmpty( ) )
          {
            str = QString( "%1" ).arg( charName );
            if( _fields.find( str ) != 0 )
            {
              str = QString( "%1_%2" ).arg( charName ).arg( iHDUNumber-1 );
            }
          }
          else
          {
            str = QString( "%1/%2" ).arg( prefix ).arg( charName );
            if( _fields.find( str ) != 0 )
            {
              str = QString( "%1/%2_%3" ).arg( prefix ).arg( charName ).arg( iHDUNumber-1 );
            }
          }

          fld->basefile = baseName;
          fld->table = table;
          fld->column = iColNumber;

          _fields.insert( str, fld );
          _fieldList.append( str );
        }
      }
    }
  }
}


long PLANCKIDEFSource::getNumFrames( fitsfile* ffits, int iNumHeaderDataUnits )
{
  long lNumRows = 0;

  if( iNumHeaderDataUnits > 1 )
  {
    int iHDUType;
    int iResult = 0;
    int iStatus = 0;

    if( fits_movabs_hdu( ffits, 2, &iHDUType, &iStatus ) == 0 )
    {
      if( fits_get_hdu_type( ffits, &iHDUType, &iStatus ) == 0 )
      {
        if( iHDUType == BINARY_TBL )
        {
          iResult = fits_get_num_rows( ffits, &lNumRows, &iStatus );
        }
      }
    }
  }

  return lNumRows;
}


long PLANCKIDEFSource::getNumFrames( const QString& filename )
{
  fitsfile* ffits;
  int numFrames = 0;
  int iResult = 0;
  int iStatus = 0;

  iResult = fits_open_file( &ffits, filename.ascii( ), READONLY, &iStatus );
  if( iResult == 0 )
  {
    int iNumHeaderDataUnits;

    if( fits_get_num_hdus( ffits, &iNumHeaderDataUnits, &iStatus ) == 0 )
    {
      numFrames = getNumFrames( ffits, iNumHeaderDataUnits );
    }

    iStatus = 0;

    fits_close_file( ffits, &iStatus );
  }

  return numFrames;
}


bool PLANCKIDEFSource::initFile( const QString& filename )
{
  QString   prefixNew;
  QString   str;
  fitsfile* ffits;
  bool      bRetVal = false;
  int       iResult = 0;
  int       iStatus = 0;

  iResult = fits_open_file( &ffits, filename.ascii( ), READONLY, &iStatus );
  if( iResult == 0 )
  {
    int iNumHeaderDataUnits;

    if( fits_get_num_hdus( ffits, &iNumHeaderDataUnits, &iStatus ) == 0 )
    {
      long lNumRows;
      int iHDUType;
      int i;

      _numFrames = getNumFrames( ffits, iNumHeaderDataUnits );

      if( _numFrames > 0 )
      {
        fits_movabs_hdu( ffits, 1, &iHDUType, &iStatus );

        //
        // add the fields and metadata...
        //
        for( i=0; i<iNumHeaderDataUnits; i++ )
        {
          if( iStatus == 0 )
          {
            addToMetadata( ffits, iStatus );

            //
            // the first table never contains data...
            //
            if( i > 0 )
            {
              //
              // create the field entries...
              //
              fits_get_hdu_type( ffits, &iHDUType, &iStatus );
              if( iStatus == 0 )
              {
                if( iHDUType == BINARY_TBL )
                {
                  int iNumCols;

                  iResult = fits_get_num_cols( ffits, &iNumCols, &iStatus );
                  if( iResult == 0 )
                  {
                    iResult = fits_get_num_rows( ffits, &lNumRows, &iStatus );
                    if( iResult == 0 )
                    {
                      if( iResult == 0 )
                      {
                        addToFieldList( ffits, iNumCols, iStatus );
                      }
                    }
                  }
                }
              }
            }

            fits_movrel_hdu( ffits, 1, &iHDUType, &iStatus);
          }
        }

        bRetVal = true;
      }
    }

    iStatus = 0;

    fits_close_file( ffits, &iStatus );
  }

  return bRetVal;
}


bool PLANCKIDEFSource::initFolderFile( const QString& filename, const QString& prefix, const QString& baseName )
{
  QString   prefixNew;
  QString   str;
  fitsfile* ffits;
  bool      bRetVal = false;
  int       iResult = 0;
  int       iStatus = 0;

  iResult = fits_open_file( &ffits, filename.ascii( ), READONLY, &iStatus );
  if( iResult == 0 )
  {
    int iNumHeaderDataUnits;

    if( fits_get_num_hdus( ffits, &iNumHeaderDataUnits, &iStatus ) == 0 )
    {
      long lNumRows;
      int iHDUType;
      int i;

      _numFrames = getNumFrames( ffits, iNumHeaderDataUnits );
      if( _numFrames > 0 )
      {
        fits_movabs_hdu( ffits, 1, &iHDUType, &iStatus );

        //
        // add the fields and metadata...
        //
        for( i=0; i<iNumHeaderDataUnits; i++ )
        {
          if( iStatus == 0 )
          {
         //   addToMetadata( ffits, iStatus );

            //
            // the first table never contains data...
            //
            if( i > 0 )
            {
              //
              // create the field entries...
              //
              fits_get_hdu_type( ffits, &iHDUType, &iStatus );
              if( iStatus == 0 )
              {
                if( iHDUType == BINARY_TBL )
                {
                  int iNumCols;

                  iResult = fits_get_num_cols( ffits, &iNumCols, &iStatus );
                  if( iResult == 0 )
                  {
                    iResult = fits_get_num_rows( ffits, &lNumRows, &iStatus );
                    if( iResult == 0 )
                    {
                      if( !prefix.isEmpty() ) 
                      {
                        char value[FLEN_VALUE];
                        char comment[FLEN_COMMENT];

                        prefixNew.truncate( 0 );

                        iResult = fits_read_keyword( ffits, "EXTNAME", value, comment, &iStatus );
                        if( iResult == 0 )
                        {
                          prefixNew = prefix + separator() + QString( value ).remove( QChar( '\'' ) );
                        }

                        iResult = 0;
                        iStatus = 0;
                      }

                      if( iResult == 0 )
                      {
                        addToFieldList( ffits, prefixNew, baseName, iNumCols, iStatus );
                      }
                    }
                  }
                }
              }
            }

            fits_movrel_hdu( ffits, 1, &iHDUType, &iStatus);
          }
        }

        bRetVal = true;
      }
    }

    iStatus = 0;

    fits_close_file( ffits, &iStatus );
  }

  return bRetVal;
}


bool PLANCKIDEFSource::initFolder( )
{
  QDir        folder( _filename, "*.fits *.fits.gz", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::Readable );
  QStringList files;
  QStringList filesBase;
  bool        bRetVal = true;

  _basefiles.setAutoDelete(false);

  files = folder.entryList( );
  if( files.size() > 0 )
  {
    for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
    {
      if( isValidFilename( *it, 0L ) )
      {
        fileList*   folderFields;
        folderField folderField;
        QString     baseName = baseFilename(*it);
        QString     pathname = folder.path() + separator() + *it;
        int         numFrames;

        folderFields = _basefiles.find( baseName );
        if( folderFields == 0L )
        {
          fileList* folderFieldsNew = new fileList;

          if( initFolderFile( pathname, baseName, baseName ) )
          {
            //
            // first add the INDEX field...
            //
            QString strIndex;
            field *fld = new field;

            fld->basefile = baseName;
            fld->table = 0;
            fld->column = 0;

            strIndex = baseName + separator() + "INDEX";
            _fields.insert( strIndex, fld );
            _fieldList.append( strIndex );

            //
            // now add everything else...
            //
            folderField.frameLo = 0;
            folderField.frames  = _numFrames;
            folderField.file    = pathname;

            folderFieldsNew->append( folderField );

            _basefiles.replace( baseName, folderFieldsNew );
          }
        }
        else
        {
          numFrames = getNumFrames( pathname );
          if( numFrames > 0 )
          {
            folderField.frameLo  = folderFields->back().frameLo + folderFields->back().frames;
            folderField.frames   = numFrames;
            folderField.file     = pathname;

            folderFields->append( folderField );
          }
        }
      }
    }
  }

  _basefiles.setAutoDelete(true);

  return bRetVal;
}


bool PLANCKIDEFSource::initFile( )
{
  bool bRetVal = false;

  if( initFile( _filename ) )
  {
    field *fld = new field;

    fld->table = 0;
    fld->column = 0;

    _fields.insert( "INDEX", fld );
    _fieldList.prepend( "INDEX" );

    updateNumFramesScalar( );

    bRetVal = true;
  }

  return bRetVal;
}


bool PLANCKIDEFSource::initialize( )
{
  bool bRetVal = true;

  _numFrames = 0;

  if( !_filename.isNull( ) && !_filename.isEmpty( ) )
  {
    QFileInfo fileInfo( _filename );

    if( fileInfo.isFile( ) )
    {
      bRetVal = initFile( );
      if( bRetVal )
      {
        _isSingleFile = true;
      }
    }
    else if( fileInfo.isDir( ) )
    {
      bRetVal = initFolder( );
      if( bRetVal )
      {
        _isSingleFile = false;
      }
    }
  }

  return bRetVal;
}


KstObject::UpdateType PLANCKIDEFSource::update( int u )
{
  if (KstObject::checkUpdateCounter(u)) {
    return lastUpdateResult();
  }

  KstObject::UpdateType updateType =  KstObject::NO_CHANGE;

  return setLastUpdateResult(updateType);
}


int PLANCKIDEFSource::readFileFrames( const QString& filename, field *fld, double *v, int s, int n )
{
  double    dNan = strtod( "nan", NULL );
  fitsfile* ffits;
  int       iRead = -1;
  int       iStatus = 0;
  int       iAnyNull;
  int       iResult = 0;

  iResult = fits_open_file( &ffits, filename.ascii( ), READONLY, &iStatus );
  if( iResult == 0 )
  {
    int iHDUType;

    if( fits_movabs_hdu( ffits, fld->table, &iHDUType, &iStatus ) == 0 )
    {
      if( iHDUType == BINARY_TBL )
      {
        _valid = true;

        if( n < 0 )
        {
          iResult = fits_read_col( ffits, TDOUBLE, fld->column, s+1, 1, 1, &dNan, v, &iAnyNull, &iStatus );
          if( iResult == 0 )
          {
            iRead = 1;
          }
        }
        else
        {
          iResult = fits_read_col( ffits, TDOUBLE, fld->column, s+1, 1, n, &dNan, v, &iAnyNull, &iStatus );
          if( iResult == 0 )
          {
            iRead = n;
          }
        }

        iStatus = 0;
      }
    }

    fits_close_file( ffits, &iStatus );
  }

  return iRead;
}


int PLANCKIDEFSource::readFolderFrames( field *fld, double *v, int s, int n )
{
  int iRead = -1;

  if( !fld->basefile.isEmpty( ) )
  {
    fileList*   folderFields;
    double*     vNew = v;
    int         sNew = s;
    int         nNew = n;
    int         iReadSub = 0;

    folderFields = _basefiles.find( fld->basefile );
    if( folderFields != 0L )
    {
      for( fileList::ConstIterator it = folderFields->begin(); it != folderFields->end(); ++it )
      {
        //
        // check if we need to read any frames from the current file...
        //
        if( n < 0 && (*it).frameLo + (*it).frames > s )
        {
          sNew = s - (*it).frameLo;
          if( sNew < 0 )
          {
            sNew = 0;
          }

          nNew = -1;

          vNew = v + (*it).frameLo + sNew;

          iReadSub = readFileFrames( (*it).file, fld, vNew, sNew, nNew );
          if( iReadSub > 0 )
          {
            if( iRead < 0 )
            {
              iRead = iReadSub;
            }
            else
            {
              iRead += iReadSub;
            }
          }
        }
        else if( (*it).frameLo < s + n && (*it).frameLo + (*it).frames > s )
        {
          sNew = s - (*it).frameLo;
          if( sNew < 0 )
          {
            sNew = 0;
          }

          nNew = n;
          if( sNew + nNew > (*it).frames )
          {
            nNew =  (*it).frames - sNew;
          }

          vNew = v + (*it).frameLo + sNew;

          if( nNew > 0 )
          {
            iReadSub = readFileFrames( (*it).file, fld, vNew, sNew, nNew );
            if( iReadSub > 0 )
            {
              if( iRead < 0 )
              {
                iRead = iReadSub;
              }
              else
              {
                iRead += iReadSub;
              }
            }
          }
        }
      }
    }
  }

  return iRead;
}


int PLANCKIDEFSource::readField( double *v, const QString& fieldName, int s, int n )
{
  int       i;
  int       iRead = -1;

  if( fieldName == "INDEX" )
  {
    for( i = 0; i < n; ++i )
    {
      v[i] = (double)( s + i );
    }

    iRead =  n;
  }
  else
  {
    field *fld = 0L;

    fld = _fields.find( fieldName );
    if( fld != 0L ) 
    {
      if( fieldName == fld->basefile + separator() + QString("INDEX") )
      {
        for( i = 0; i < n; ++i )
        {
          v[i] = (double)( s + i );
        }

        iRead =  n;
      }
      else
      {
        _valid = false;

        if( !_filename.isNull( ) && !_filename.isEmpty( ) )
        {
          if( _isSingleFile )
          {
            iRead = readFileFrames( _filename, fld, v, s, n );
          }
          else
          {
            iRead = readFolderFrames( fld, v, s, n );
          }
        }
      }
    }
  }

  return iRead;
}


bool PLANCKIDEFSource::isValidField( const QString& field ) const
{
  bool bRetVal = false;

  if( field == "INDEX" )
  {
    bRetVal = true;
  }
  else
  {
    if( _fields.find( field ) != 0L )
    {
      bRetVal = true;
    }
  }

  return bRetVal;
}


int PLANCKIDEFSource::samplesPerFrame( const QString& fieldName )
{
  Q_UNUSED( fieldName )

  int rc = 1;

  return rc;
}


int PLANCKIDEFSource::frameCount( const QString& fieldName ) const
{
  int rc = 0;

  if( _isSingleFile )
  {
    rc = _numFrames;
  }
  else
  {
    field* fld;

    if( !fieldName.isEmpty() )
    {
      fld = _fields.find( fieldName );
      if( fld != 0L )
      {
        fileList* folderFields;

        folderFields = _basefiles.find( fld->basefile );
        if( folderFields != 0L )
        {
          for( fileList::ConstIterator it = folderFields->begin(); it != folderFields->end(); ++it )
          {
            rc += (*it).frames;
          }
        }
      }
    }
  }

  return rc;
}


bool PLANCKIDEFSource::isEmpty( ) const 
{
  return _fields.isEmpty();
}


QString PLANCKIDEFSource::fileType( ) const
{
  return "PLANCKIDEF";
}


void PLANCKIDEFSource::save( QTextStream& ts, const QString& indent )
{
  KstDataSource::save( ts, indent );
  _config->save(ts, indent);
}


bool PLANCKIDEFSource::supportsHierarchy( ) const
{
  return true;
}


bool PLANCKIDEFSource::checkValidPlanckIDEFFolder( const QString& filename )
{
  QDir folder( filename, "*.fits *.fits.gz", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::Readable );
  QStringList files;
  QString pathname;
  bool ok = false;

  files = folder.entryList( );
  if( files.size() > 0 )
  {
    for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it) 
    {
      pathname = folder.path() + separator() + *it;

      if( checkValidPlanckIDEFFile( pathname, 0L ) )
      {
        ok = true;

        break;
      }
    }
  }

  return ok;
}


bool PLANCKIDEFSource::checkValidPlanckIDEFFile( const QString& filename, Config *cfg )
{
  bool ok = false;
  fitsfile* ffits;
  int iStatus = 0;

  //
  // determine if it is a Planck IDIS DMC Exchange Format file...
  //
  if( isValidFilename( filename, cfg ) )
  {
    if( fits_open_file( &ffits, filename.ascii( ), READONLY, &iStatus ) == 0 )
    {
      int iNumHeaderDataUnits;

      if( fits_get_num_hdus( ffits, &iNumHeaderDataUnits, &iStatus ) == 0 )
      {
        char  value[FLEN_VALUE];
        char  comment[FLEN_COMMENT];
        int   iHDUType;
        int   iValue;
        int   i;

        //
        // the primary header should never have any data...
        //
        if( fits_get_hdu_type( ffits, &iHDUType, &iStatus ) == 0 )
        {
          if( iHDUType == IMAGE_HDU )
          {
            if( fits_read_key( ffits, TLOGICAL, "SIMPLE", &iValue, comment, &iStatus ) == 0 )
            {
              if( iValue != 0 )
              {
                if( fits_read_key( ffits, TLOGICAL, "EXTEND", &iValue, comment, &iStatus ) == 0 )
                {
                  if( iValue != 0 )
                  {
                    if( fits_read_key( ffits, TINT, "NAXIS", &iValue, comment, &iStatus ) == 0 )
                    {
                      if( iValue == 0 )
                      {
                        ok = true;
                      }
                    }
                  }
                }
              }
            }
          }
        }

        //
        // the name of each binary table must conform to aaa-bbbb[-ccc]
        //  where bbbb is a member of {OBTT, TOD., OBTH, HKP.}...
        //
        if( ok && iStatus == 0 )
        {
          if( iNumHeaderDataUnits > 1 )
          {
            long rowsCompare = 0;
            long rows;
            int cols;

            for( i=0; i<iNumHeaderDataUnits-1 && ok; i++ )
            {
              if( fits_movrel_hdu( ffits, 1, &iHDUType, &iStatus ) == 0 )
              {
                bool bOBTHeader = false;

                ok = false;

                if( iStatus == 0 && iHDUType == BINARY_TBL )
                {
                  if( fits_read_keyword( ffits, "EXTNAME", value, comment, &iStatus ) == 0 )
                  {
                    QString section = value;

                    ok = true;

                    section.stripWhiteSpace();
                    if( section.compare( "OBT" ) == 0 )
                    {
                      bOBTHeader = true;
                    }
                  }

                  //
                  // all tables should have the same number of rows...
                  //
                  if( ok )
                  {
                    bool okCols = false;

                    if( fits_get_num_cols( ffits, &cols, &iStatus ) == 0 )
                    {
                      if( cols > 0 )
                      {
                        okCols = true;
                      }
                    }

                    if( okCols )
                    {
                      if( fits_get_num_rows( ffits, &rows, &iStatus ) == 0 )
                      {
                        if( i == 0 )
                        {
                          rowsCompare = rows;
                        }
                        else if( rowsCompare == rows )
                        {
                          ok = true;
                        }
                        else
                        {
                          ok = false;
                        }
                      }
                      else
                      {
                        ok = false;
                      }
                    }
                  }
                }
              }
              else
              {
                ok = false;
              }
            }
          }
          else
          {
            ok = false;
          }
        }
      }

      if( iStatus != 0 ) 
      {
        ok = false;
      }

      iStatus = 0;

      fits_close_file( ffits, &iStatus );
    }
  }

  return ok;
}


QString PLANCKIDEFSource::configuration(QString setting) 
{
  if (setting.lower() == "checkfilename") 
  {
    if (_config->_checkFilename) 
    {
      return QString("true");
    }
    else
    {
      return QString("false");
    }
  }

  return QString();
}


bool PLANCKIDEFSource::setConfiguration(QString setting, const QString &value) {
  if (setting.lower() == "checkfilename") {
    if (value.lower() == "true") {
      _config->_checkFilename = true;
      return true;
    } else if (value.lower() == "false") {
      _config->_checkFilename = false;
      return true;
    }
  }

  return false;
}


bool PLANCKIDEFSource::supportsTimeConversions() const {
  return _fieldList.contains("OBT");
}


int PLANCKIDEFSource::sampleForOBT(double timeIDEF, bool *ok) {
  double    value;
  int       indexLo = 0;
  int       indexHi = _numFrames-1;
  int       index;
  int       sample = -1;

  while( indexHi > indexLo )
  {
    index = ( indexLo + indexHi ) / 2;
    if( readField( &value, "OBT", index, 1 ) == 1 )
    {
      if( value == timeIDEF )
      {
        sample = index;

        if (ok)
        {
          *ok = true;
        }

        break;
      }
      else if( value > timeIDEF )
      {
        indexHi = index;
      }
      else
      {
        indexLo = index;
      }

      if( ( indexLo + indexHi ) / 2 == indexLo )
      {
        sample = indexLo;

        if (ok)
        {
          *ok = true;
        }

        break;
      }
    }
    else
    {
      break;
    }
  }

  return sample;
}


int PLANCKIDEFSource::sampleForTime(const KST::ExtDateTime& time, bool *ok) {
  int sample = -1;

  if (!_valid)
  {
    if (ok)
    {
      *ok = false;
    }

    return sample;
  }

  double timeIDEF;

  //
  // need to convert from KST::ExtDateTime to IDEF OBT (TAI in 2^-16 seconds)...
  //
  timeIDEF = 65536.0 * ( (double)time.toTime_t() - ( 86400.0 * ( 365.0 * 12.0 + 3.0 )));

  sample = sampleForOBT( timeIDEF, ok );

  return sample;
}


int PLANCKIDEFSource::sampleForTime(double ms, bool *ok) {
  int sample = -1;

  if (!_valid)
  {
    if (ok)
    {
      *ok = false;
    }

    return sample;
  }

  double timeZero;

  if( readField( &timeZero, "OBT", 0, 1 ) == 1 )
  {
    double timeIDEF;

    //
    // need to convert from ms time to TAI in 2^-16 seconds...
    //
    timeIDEF = timeZero + ( ms * 65.536 );

    sample = sampleForOBT( timeIDEF, ok );
  }

  return sample;
}


KST::ExtDateTime PLANCKIDEFSource::timeForSample(int sample, bool *ok) {
  KST::ExtDateTime t;

  if (!_valid)
  {
    if (ok)
    {
      *ok = false;
    }

    return t;
  }

  double time;

  if( readField( &time, "OBT", sample, 1 ) == 1 )
  {
    if( time == time )
    {
      t.setTime_t( (int)((time / 65.536) - ( 1000 * 86400 * (365 * 12 + 3))));

      if (ok)
      {
        *ok = true;
      }
    }
  }

  return t;
}


double PLANCKIDEFSource::relativeTimeForSample(int sample, bool *ok) {
  double timeRelativeMS = -1.0;

  if (!_valid)
  {
    if (ok)
    {
      *ok = false;
    }

    return timeRelativeMS;
  }

  double value;
  double valueZero;

  if( readField( &valueZero, "OBT", 0, 1 ) == 1 )
  {
    if( readField( &value, "OBT", sample, 1 ) == 1 )
    {
      timeRelativeMS = ( value - valueZero ) / 65.536;

      if (ok)
      {
        *ok = true;
      }
    }
  }

  return timeRelativeMS;
}


class ConfigWidgetPlanckIDEF : public KstDataSourceConfigWidget {
  public:
    ConfigWidgetPlanckIDEF() : KstDataSourceConfigWidget() {
      QGridLayout *layout = new QGridLayout(this, 1, 1);
      _ac = new PlanckIDEFConfig(this);
      layout->addWidget(_ac, 0, 0);
      layout->activate();
    }

    virtual ~ConfigWidgetPlanckIDEF() {}

    virtual void setConfig(KConfig *cfg) {
      KstDataSourceConfigWidget::setConfig(cfg);
    }

    virtual void load() {
      _cfg->setGroup("PlanckIDEF General");
      _ac->_checkFilename->setChecked(_cfg->readBoolEntry("Check Filename", true));
    }

    virtual void save() {
      _cfg->setGroup("PlanckIDEF General");
      _cfg->writeEntry("Check Filename", (int)_ac->_checkFilename->isChecked());
    }

    PlanckIDEFConfig *_ac;
};


extern "C" {
  KstDataSource *create_planckIDEF( KConfig *cfg, const QString& filename, const QString& type )
  {
    return new PLANCKIDEFSource( cfg, filename, type );
  }

  KstDataSource *load_planckIDEF(KConfig *cfg, const QString& filename, const QString& type, const QDomElement& e) 
  {
    return new PLANCKIDEFSource(cfg, filename, type, e);
  }

  QStringList provides_planckIDEF( )
  {
    QStringList rc;

    rc += "PLANCKIDEF";

    return rc;
  }

  bool supportsTime_planckIDEF(KConfig*, const QString& filename) 
  {
    return true;
  }

  bool supportsHierarchy_planckIDEF( )
  {
    return true;
  }

  int understands_planckIDEF( KConfig* cfg, const QString& filename )
  {
    PLANCKIDEFSource::Config config;
    QFileInfo fileinfo( filename );
    int       iRetVal = 0;

    config.read(cfg, filename);

    if( fileinfo.isFile( ) )
    {
      if( PLANCKIDEFSource::checkValidPlanckIDEFFile( filename, &config ) )
      {
        iRetVal = 99;
      }
    }
    else if( fileinfo.isDir( ) )
    {
      if( PLANCKIDEFSource::checkValidPlanckIDEFFolder( filename ) )
      {
        iRetVal = 99;
      }
    }

    return iRetVal;
  }

  QWidget *widget_planckIDEF(const QString& filename) {
    Q_UNUSED(filename)

    return new ConfigWidgetPlanckIDEF;
  }
}

KST_KEY_DATASOURCE_PLUGIN(planckIDEF)

