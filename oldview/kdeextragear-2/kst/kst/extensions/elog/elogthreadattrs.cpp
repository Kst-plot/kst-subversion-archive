/***************************************************************************
                                   elogthreadattrs.cpp
                             -------------------
    begin                : Feb 09 2004
    copyright            : (C) 2004 The University of British Columbia
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

#include "elogthreadattrs.h"
#include <kstdebug.h>
#include <kst.h>
#include <kstevents.h>
#include <qiodevice.h>
#include <qbuffer.h>
#include <qregexp.h>
#include <qstring.h>
#include <qcstring.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

ElogThreadAttrs::ElogThreadAttrs(KstELOG* elog) : ElogThread(elog) {
}

ElogThreadAttrs::~ElogThreadAttrs() {
}

void ElogThreadAttrs::doTransmit(int sock) {
  QStringList	strListAttributes;
  QStringList	strListAttribute;
  QString strSplit( "\n" );
  QString strSplitAttribute( "=" );
  QString strUserName;
  QString strUserPassword;
  QString strWritePassword;
  QString strLogbook;
  QString strAttributes;
  QString strText;
  bool bFirst = TRUE;
  char request[100000];
  char response[100000];
  char str[80];
  int i, n;

  strUserName 					= _elog->configuration()->userName();
  strUserPassword 			= _elog->configuration()->userPassword();
  strWritePassword			= _elog->configuration()->writePassword();
  strLogbook  					= _elog->configuration()->name();
  
  strcpy(request, "GET /");
  if (!strLogbook.isEmpty()) {
    sprintf(request + strlen(request), "%s/?cmd=new", strLogbook.ascii());
  }
  strcat(request, " HTTP/1.0\r\n");
  sprintf(request + strlen(request), "Host: %s\r\n", _host_name);
  sprintf(request + strlen(request), "User-Agent: ELOG\r\n");

  if (!strWritePassword.isEmpty()) {
    if (bFirst) {
      sprintf(request + strlen(request), "Cookie: ");
    }
    bFirst = FALSE;
    base64_encode(strWritePassword.ascii(), str);
    sprintf(request + strlen(request), "wpwd=%s;", str);
  }

  if (!strUserName.isEmpty()) {
    if (bFirst) {
      sprintf(request + strlen(request), "Cookie: ");
    }
    bFirst = FALSE;
    sprintf(request + strlen(request), "unm=%s;", strUserName.ascii());
  }

  if (!strUserPassword.isEmpty()) {
    if (bFirst) {
      sprintf(request + strlen(request), "Cookie: ");
    }
    bFirst = FALSE;
    base64_encode(strUserPassword.ascii(), str);
    sprintf(request + strlen(request), "upwd=%s;", str);
  }

  if (!bFirst) {
    strcat(request, "\r\n");
  }
  strcat(request, "\r\n");
 
  send(sock, request, strlen(request), 0);
    
  //
  // handle the response...
  //
  i = recv(sock, response, 10000, 0);
  if (i >= 0) {    
    n = i;
    while (i > 0) {
      i = recv(sock, response + n, 10000, 0);
      if (i > 0) {
        n += i;
      }
    }
    response[n] = 0;
    
    if( doResponseError(response) ) {
      doResponse(response);
    }
  }
  else
  {
    doError( tr2i18n("ELOG attributes: unable to receive response"), KstDebug::Notice );
  }
}

void ElogThreadAttrs::run( ) {
  int status, sock, error;
  
  error = makeConnection( &sock, &status );
  if( status == 0 ) {
    doTransmit(sock);
    close(sock);
  }
  else {
    switch( error ) {
      case -1:
        doError( tr2i18n("Failed to retrieve ELOG attributes: failed to connect to host.") );
        break;
      case -2:
        doError( tr2i18n("Failed to retrieve ELOG attributes: failed to get host name.") );
        break;
      case -3:
        doError( tr2i18n("Failed to retrieve ELOG attributes: failed to create socket.") );
        break;
      case -4:
        doError( tr2i18n("Failed to retrieve ELOG attributes: failed to get host by address.") );
        break;
      case -5:
        doError( tr2i18n("Failed to retrieve ELOG attributes: failed to get host by name.") );
        break;
      default:
        doError( tr2i18n("Failed to retrieve ELOG attributes: unknown error.") );
        break;
    }
  }
}

void ElogThreadAttrs::doResponse( char* response ) {
  QCustomEvent   eventAttrs(KstELOGAttrsEvent);
  KstELOGAttribStruct	attrib;
  ELOGAttribList attribs;
  QStringList		 strAttribs;
  QString        strResponse;
  QString        strAttrib;
  QString        strValues;
  QString        strValue;
  QString        strType;
  QString        strFont;
  static char    attribname[] = " class=\"attribname\">";
  static char    attribvalue[] = " class=\"attribvalue\">";
  static char    select[] = "<select ";
  static char    input[] = "<input ";
  static char    endlabel[] = "</td>";
  static char    fontstart[] = "<font ";
  static char    fontend[] = "</font>";
  static char    optionvalue[] = "<option value=\"";
  static char    type[] = " type=";
  static char    value[] = " value=\"";
  static char    valueNoQuote[] = " value=";
  char*				   p = response;
  char*					 pAttribNameStart;
  char*					 pAttribNameEnd = NULL;
  char*					 pAttribValueStart;
  char*					 pAttribValueEnd = NULL;
  bool           bAppend;
  int					   iFontStart;
  int						 iFontEnd;
  int            iOptionsStart = 0;
  int						 iOptionsEnd;
  int					   iValueStart;
  int						 iValueEnd;
  int						 iInput;
  int						 iSelect;
  
  //
  // find and categorise the attributes...
  //
  while( p != NULL ) {
    pAttribValueEnd = NULL;
    attrib.bMandatory = FALSE;
    attrib.values.clear();
    attrib.pWidget = NULL;
    
    //
    // retrieve the attribute label name...
    //
    pAttribNameStart = strstr( p, attribname );
    if( pAttribNameStart != NULL ) {
      pAttribNameStart += strlen( attribname );
      pAttribNameEnd = strstr( pAttribNameStart, endlabel );
      if( pAttribNameEnd != NULL ) {
        *pAttribNameEnd  = '\0';
        strAttrib = pAttribNameStart;
        pAttribNameEnd += strlen( endlabel );
        
        //
        // check for a mandatory field...
        //
        iFontStart = strAttrib.find( fontstart );
        if( iFontStart != -1 ) {
          iFontEnd = strAttrib.find( fontend );
          if( iFontEnd != -1 ) {
            strFont = strAttrib.mid( iFontStart, iFontEnd + strlen( fontend ) - iFontStart );
            strAttrib.remove( iFontStart, iFontEnd + strlen( fontend ) - iFontStart );
            if( strFont.find( '*' ) ) {
              attrib.bMandatory = TRUE;
            }
          }
        }
        
        //
        // remove the trailing : if present...
        //
        if( strAttrib.findRev( ':' ) == (int)strAttrib.length( ) - 1 ) {
          strAttrib.truncate( strAttrib.length( ) - 1 );
        }
        attrib.attribName = strAttrib;

        //
        // determine the input type...
        //
        pAttribValueStart = strstr( pAttribNameEnd, attribvalue );
        if( pAttribValueStart != NULL ) {
          pAttribValueStart += strlen( attribvalue );
          pAttribValueEnd   = strstr( pAttribValueStart, endlabel );
          if( pAttribValueEnd != NULL ) {
            iOptionsStart = 0;
            *pAttribValueEnd  = '\0';
            strValues = pAttribValueStart;
            pAttribValueEnd += strlen( endlabel );
            
            iInput = strValues.find( input );
            iSelect = strValues.find( select );
            if( ( iInput != -1 && iSelect == -1 ) || 
                ( iInput != -1 && iSelect != -1 && iInput < iSelect ) ) {     
              bAppend = FALSE;
              while( ( iOptionsStart = strValues.find( type, iOptionsStart ) ) != -1 ) {
                iOptionsStart += strlen( type );
                iOptionsEnd = strValues.find( " ", iOptionsStart );
                if( iOptionsEnd != -1 ) {
                  strType = strValues.mid( iOptionsStart, iOptionsEnd - iOptionsStart );
                  iOptionsStart += iOptionsEnd - iOptionsStart;
                  if( strType == "radio" ) {
                    attrib.type = AttribTypeRadio;
                    
                    iValueStart = strValues.find( value, iOptionsStart );
                    if( iValueStart != -1 ) {
                      iValueStart += strlen( value );
                      iValueEnd = strValues.find( "\"", iValueStart );
                      iOptionsStart = iValueEnd + strlen( "\"" );
                      strValue = strValues.mid( iValueStart, iValueEnd - iValueStart );
                      if( !strValue.isEmpty() ) {
                        attrib.values.append( strValue );
                        bAppend = TRUE;
                      }
                    }
                  }
                  else if( strType == "checkbox" ) {
                    attrib.type = AttribTypeCheck;

                    iValueStart = strValues.find( value, iOptionsStart );
                    if( iValueStart != -1 ) {
                      iValueStart += strlen( value );
                      iValueEnd = strValues.find( "\"", iValueStart );
                      if( iValueEnd != -1 ) {
                        iOptionsStart = iValueEnd + strlen( "\"" );
                        strValue = strValues.mid( iValueStart, iValueEnd - iValueStart );
                        if( !strValue.isEmpty() ) {
                          attrib.values.append( strValue );
                          bAppend = TRUE;
                        }
                      }
                    } else {
                      iValueStart = strValues.find( valueNoQuote, iOptionsStart );
                      if( iValueStart != -1 ) {
                        iValueStart += strlen( valueNoQuote );
                        iValueEnd = strValues.find( QRegExp(">| "), iValueStart );
                        if( iValueEnd != -1 ) {
                          iOptionsStart = iValueEnd + strlen( ">" );
                          strValue = strValues.mid( iValueStart, iValueEnd - iValueStart );
                          if( !strValue.isEmpty() ) {
                            attrib.values.append( strValue );
                            bAppend = TRUE;
                          }
                          if( strValue == "1" && attrib.values.count() == 1 ) {
                            attrib.type = AttribTypeBool;
                          }
                        }
                      }
                    }
                  }
                  else if( strType == "\"text\"" ) {
                    attrib.type = AttribTypeText;
                    bAppend = TRUE;
                    break;
                  }
                  else {
                    break;
                  }
                }
                else {
                  break;
                }
              }
              if( bAppend ) {
                attribs.append( attrib );               
              }
            }
            else if( ( iSelect != -1 && iInput == -1 ) || 
                     ( iSelect != -1 && iInput != -1 && iSelect < iInput ) ) {
              //
              // we have a combobox, so need to determine the options...
              //
              attrib.type = AttribTypeCombo;
             
              while( ( iOptionsStart = strValues.find( optionvalue, iOptionsStart ) ) != -1 ) {
                iOptionsStart += strlen( optionvalue );
                iOptionsEnd = strValues.find( "\">", iOptionsStart );
                if( iOptionsEnd != -1 ) {
                  strValue = strValues.mid( iOptionsStart, iOptionsEnd - iOptionsStart );
                  iOptionsStart = iOptionsEnd + strlen( "\">" );
                  if( !strValue.isEmpty() ) {
                    attrib.values.append( strValue );
                  }
                }
                else {
                  break;
                }
              }
              attribs.append( attrib );
            }
          }
        }
      }
    }
    p = pAttribValueEnd;   
  }
  
  eventAttrs.setData( &attribs );
  QApplication::sendEvent( (QObject*)_elog->entry(), (QEvent*)&eventAttrs );
}

bool ElogThreadAttrs::doResponseError( const char* response ) {
  QString strError;
  bool bRetVal = TRUE;
  
  if (strstr(response, "<title>ELOG error</title>")) {
    doError( tr2i18n("Failed to access ELOG: no such logbook was found.") );
    bRetVal = FALSE;
  }
  else if (strstr(response, "<title>ELOG password</title>")) {
    doError( tr2i18n("Failed to access ELOG: password was incorrect or missing.") );
    bRetVal = FALSE;
  }
  
  return bRetVal;
}
// vim: ts=2 sw=2 et
