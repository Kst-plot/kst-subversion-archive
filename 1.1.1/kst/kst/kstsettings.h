/***************************************************************************
              kstsettings.h: a collection of settings for kst
                             -------------------
    begin                : Nov 23, 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef KSTSETTINGS_H
#define KSTSETTINGS_H

#include <qcolor.h>

#include "kstplotdefines.h"

enum EMailEncryption {
  EMailEncryptionNone,
  EMailEncryptionSSL,
  EMailEncryptionTLS,
  EMailEncryptionMAXIMUM
};

enum EMailAuthentication {
  EMailAuthenticationPLAIN,
  EMailAuthenticationLOGIN,
  EMailAuthenticationCRAMMD5,
  EMailAuthenticationDIGESTMD5,
  EMailAuthenticationMAXIMUM
};

class KstSettings {
  public:
    KstSettings();
    KstSettings(const KstSettings&);
    KstSettings& operator=(const KstSettings&);

    // do not delete this object
    static KstSettings *globalSettings();
    static void setGlobalSettings(const KstSettings *settings);
    static void checkUpdates();
    void save();
    void reload();

    long plotUpdateTimer;
    QColor backgroundColor;
    QColor foregroundColor;

    bool promptWindowClose : 1;
    bool showQuickStart : 1;
    bool xMajor : 1;
    bool yMajor : 1;
    bool xMinor : 1;
    bool yMinor : 1;
    bool majorGridColorDefault : 1;
    bool minorGridColorDefault : 1;
    bool xAxisInterpret : 1;
    bool yAxisInterpret : 1;
    bool useUTC : 1;

    QColor majorColor;
    QColor minorColor;
    KstAxisInterpretation xAxisInterpretation;
    KstAxisDisplay xAxisDisplay;
    KstAxisInterpretation yAxisInterpretation;
    KstAxisDisplay yAxisDisplay;

    QString emailSender;
    QString emailSMTPServer;
    QString emailUsername;
    QString emailPassword;
    int     emailSMTPPort;
    bool    emailRequiresAuthentication : 1;
    EMailEncryption emailEncryption;
    EMailAuthentication emailAuthentication;

    struct Printing {
      QString pageSize;
      QString orientation;
      QString plotDateTimeFooter;
      QString maintainAspect;
      QString curveWidthAdjust;
      QString monochrome;
    };
    Printing printing;

  private:
    static KstSettings *_self;
};

#endif
// vim: ts=2 sw=2 et
