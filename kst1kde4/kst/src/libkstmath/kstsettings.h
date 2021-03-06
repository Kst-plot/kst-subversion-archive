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

#include <QColor>

#include "kstplotdefines.h"
#include "kst_export.h"

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
    KST_EXPORT KstSettings();
    KST_EXPORT KstSettings(const KstSettings&);
    KST_EXPORT KstSettings& operator=(const KstSettings&);

    // do not delete this object
    KST_EXPORT static KstSettings *globalSettings();
    KST_EXPORT static void setGlobalSettings(const KstSettings *settings);
    KST_EXPORT void save();
    static void cleanup();
    void reload();

    KST_EXPORT void setPrintingDefaults(); // set printing settings to default

    long plotUpdateTimer;
    long plotFontSize;
    long plotFontMinSize;
    QColor backgroundColor;
    QColor foregroundColor;
    QString curveColorSequencePalette;

    bool promptPlotDelete : 1;
    bool promptWindowClose : 1;
    bool showQuickStart : 1;
    bool tiedZoomGlobal : 1;
    bool xMajor : 1;
    bool yMajor : 1;
    bool xMinor : 1;
    bool yMinor : 1;
    bool majorGridColorDefault : 1;
    bool minorGridColorDefault : 1;
    bool xAxisInterpret : 1;
    bool yAxisInterpret : 1;
    bool emailRequiresAuthentication : 1;

    QColor majorColor;
    QColor minorColor;
    KstAxisInterpretation xAxisInterpretation;
    KstAxisDisplay xAxisDisplay;
    KstAxisInterpretation yAxisInterpretation;
    KstAxisDisplay yAxisDisplay;

    int defaultLineWeight;

    QString emailSender;
    QString emailSMTPServer;
    QString emailUsername;
    QString emailPassword;
    int     emailSMTPPort;
    EMailEncryption emailEncryption;
    EMailAuthentication emailAuthentication;

    KST_EXPORT int utcOffset();
    QString timezone;
    int offsetSeconds;

    struct PrintingMonochrome {
      QString enhanceReadability;

      // order of "-1" means property is not included in cycling
      QString pointStyleOrder;
      QString lineStyleOrder;
      QString lineWidthOrder;
      QString maxLineWidth;
      QString pointDensity;
    };

    struct Printing {
      QString pageSize;
      QString orientation;
      QString plotDateTimeFooter;
      QString maintainAspect;
      QString curveWidthAdjust;
      QString monochrome;
      PrintingMonochrome monochromeSettings;
    };
    Printing printing;

  private:
    static KstSettings *_self;
};

#endif
