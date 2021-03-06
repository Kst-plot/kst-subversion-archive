/***************************************************************************
                    lfiio.cpp  - FITS file data source
                             -------------------
    begin                : Fri Oct 17 2003
    copyright            : (C) 2003 The University of British Columbia
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

#include "lfiio.h"

#include <kdebug.h>
#include <qfile.h>
#include <ctype.h>
#include <fitsio.h>
#include <stdlib.h>
 
LFIIOSource::LFIIOSource( const QString& filename, const QString& type )
: KstDataSource( filename, type )
{
  if( type.isEmpty( ) || type == "LFIIO" ) 
  {
    if( initFile( ) ) 
    {
      _valid = true;
    }
  }
}


LFIIOSource::~LFIIOSource( ) 
{
}


bool LFIIOSource::initFile( )
{
  KstObject::UpdateType updateType;
  bool                  bRetVal = false;

  _numFrames = 0;
  updateType = update( );
  if( updateType == KstObject::UPDATE )
  {
    bRetVal = true;
  }

  return bRetVal;
}


KstObject::UpdateType LFIIOSource::update( int u ) 
{
  Q_UNUSED( u )

  KstObject::UpdateType updateType =  KstObject::NO_CHANGE;
  QString               strTemplate;
  QString               strName;
  fitsfile*             ffits;
  char                  charTemplate[ FLEN_CARD ];
  char                  charName[ FLEN_CARD ];
  long                  lNumFrames;
  long                  lMaxRepeat = 1;
  long                  lRepeat;
  long                  lWidth;
  int                   iColNumber;
  int                   iNumCols;
  int                   iStatus = 0;
  int                   iResult = 0;
  int                   iTypeCode;
  int                   i;

  _valid  = false;

  if( !_filename.isNull( ) && !_filename.isEmpty( ) )
  {
    iResult = fits_open_table( &ffits, _filename.ascii( ), READONLY, &iStatus );
    if( iResult == 0 )
    {
      //
      // determine size of data...
      //
      iResult = fits_get_num_cols( ffits, &iNumCols, &iStatus );
      if( iResult == 0 )
      {
        iResult = fits_get_num_rows( ffits, &lNumFrames, &iStatus );
        if( iResult == 0 )
        {
          _strListColNames.clear( );

          _valid = true;

          //
	        // need to multiply lNumFrames by the maximum value of the vector repeat value...
	        //
          for( i=0; i<iNumCols; i++ )
          {
            iStatus = 0;
            
            sprintf( charTemplate, "%d", i+1 );
            iResult = fits_get_colname( ffits, CASEINSEN, charTemplate, charName, &iColNumber, &iStatus );
            if( iResult == 0 )
            { 
              strName = charName;
              _strListColNames.append( strName );
            }
            else
            {
              strName.setNum( i );
              _strListColNames.append( strName );
            }
              
            iStatus = 0;
            iResult = fits_get_coltype( ffits, i+1, &iTypeCode, &lRepeat, &lWidth, &iStatus );
            if( iResult == 0 )
            {
              if( lRepeat > lMaxRepeat )
              {
                lMaxRepeat = lRepeat;
              }
            }
          }

          if( lNumFrames * lMaxRepeat != _numFrames )
	        {
            _numCols   = iNumCols;
            _numFrames = lNumFrames * lMaxRepeat;
            updateType = KstObject::UPDATE;
          }
        }
      }
      iStatus = 0;
      fits_close_file( ffits, &iStatus );   
    }
  }
  
  return updateType;
}


int LFIIOSource::readField( double *v, const QString& field, int s, int n ) 
{
  double    dNan = strtod( "nan", NULL );
  fitsfile* ffits;
  bool      bOk;
  int       i;
  int       iCol;
  int       iRead = -1;
  int       iStatus = 0;
  int       iAnyNull;
  int       iResult = 0;

  if( n < 0 ) 
  {
    n = 1; /* n < 0 means read one sample, not frame - irrelavent here */
  }

  if( field == "INDEX" ) 
  {
    for( i = 0; i < n; i++ ) 
    {
      v[i] = (double)( s + i );
    }
    iRead =  n;
  }
  else
  {
    memset( v, 0, n * sizeof( double ) );
    
    bOk = getColNumber( field, &iCol );
    if( bOk ) 
    {
      _valid = false;

      if( !_filename.isNull( ) && !_filename.isEmpty( ) )
      {
        iResult = fits_open_table( &ffits, _filename.ascii( ), READONLY, &iStatus );
        if( iResult == 0 )
        {
          _valid = true;
          
          //
          // copy the data... 
          // N.B. fitsio column indices are 1 based, so we ask for iCol+1 instead of just iCol
          //
          iResult = fits_read_col( ffits, TDOUBLE, iCol+1, s+1, 1, n, &dNan, v, &iAnyNull, &iStatus );
          if( iResult == 0 )
          {
            iRead = n;
          }
          
          iStatus = 0;
          fits_close_file( ffits, &iStatus );      
        }
      }
    }
  }
  
  return iRead;
}


bool LFIIOSource::getColNumber( const QString& field, int* piColNumber ) const
{
  QString strName;
  bool    bOk     = false;
  bool    bRetVal = false;
  int     iCount;
  int     iCol;
  int     i;

  iCol = field.toUInt( &bOk );
  if( bOk )
  {
    if( iCol < _numCols )
    {
      *piColNumber = iCol;

      bRetVal = true;
    }
  }
  else
  {
    iCount = _strListColNames.count( );

    for( i=0; i<iCount; i++ )
    {
      strName = _strListColNames[i].lower( );
      if( strName.compare( field.lower( ) ) == 0 )
      {
        bRetVal = true;
        
        *piColNumber = i;

        break;
      }      
    }
  }

  return bRetVal;
}


bool LFIIOSource::isValidField( const QString& field ) const
{
  bool bRetVal = false;
  int  iCol;

  if( field == "INDEX" ) 
  {
    bRetVal = true;
  }
  else
  {
    bRetVal = getColNumber( field, &iCol );
  }

  return bRetVal;
}


int LFIIOSource::samplesPerFrame( const QString& field ) 
{
  Q_UNUSED( field )

  return 1;
}


int LFIIOSource::frameCount( ) const 
{
  return  _numFrames;
}


QString LFIIOSource::fileType( ) const 
{
  return "LFIIO";
}


void LFIIOSource::save( QTextStream& ts ) 
{
  KstDataSource::save( ts );
}


extern "C" {
KstDataSource *create_lfiio( const QString& filename, const QString& type ) 
{
  return new LFIIOSource( filename, type );
}

QStringList provides_lfiio( ) 
{
  QStringList rc;
  
  rc += "LFIIO";
  
  return rc;
}

bool understands_lfiio( const QString& filename ) 
{
  fitsfile* ffits;
  bool      bRetVal = false;
  int       iStatus = 0;

  //
  // determine if it is a FITS file...
  //
  if( fits_open_table( &ffits, filename.ascii( ), READONLY, &iStatus ) == 0 )
  {
    bRetVal = true;

    fits_close_file( ffits, &iStatus );      
  }
  else
  {
    //
    // failed to open the file, so we can't understand it...
    //
  }
  
  return bRetVal;
}

}

// vim: ts=2 sw=2 et
