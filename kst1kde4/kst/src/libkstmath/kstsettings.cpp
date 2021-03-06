/***************************************************************************
             kstsettings.cpp: a collection of settings for kst
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

#include <QApplication>
#include <QPrinter>
#include <QSettings>

#include <kemailsettings.h>

#include "kstsettings.h"

KstSettings::KstSettings() {
  plotUpdateTimer = 200;
  plotFontSize    = 12;
  plotFontMinSize = 5;
  backgroundColor = QColor(255, 255, 255); // white
  foregroundColor = QColor(0,0,0); // black
  promptPlotDelete = false;
  promptWindowClose = true;
  showQuickStart = true;
  tiedZoomGlobal = false;
  curveColorSequencePalette = "Kst Colors";

  xMajor = false;
  yMajor = false;
  xMinor = false;
  yMinor = false;
  majorColor = QColor(128,128,128);
  minorColor = QColor(128,128,128);
  majorGridColorDefault = true;
  minorGridColorDefault = true;

  xAxisInterpret = false;
  xAxisInterpretation = AXIS_INTERP_CTIME;
  xAxisDisplay = AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS;
  yAxisInterpret = false;
  yAxisInterpretation = AXIS_INTERP_CTIME;
  yAxisDisplay = AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS;

  defaultLineWeight = 0;

  emailSMTPPort = 25;
  emailRequiresAuthentication = false;
  emailEncryption = EMailEncryptionNone;
  emailAuthentication = EMailAuthenticationPLAIN;

  timezone = "UTC";
  offsetSeconds = 0;

  setPrintingDefaults();
}


KstSettings::KstSettings(const KstSettings& x) {
  *this = x;
}


KstSettings& KstSettings::operator=(const KstSettings& x) {
  plotUpdateTimer = x.plotUpdateTimer;
  plotFontSize    = x.plotFontSize;
  plotFontMinSize = x.plotFontMinSize;
  backgroundColor = x.backgroundColor;
  foregroundColor = x.foregroundColor;
  promptPlotDelete = x.promptPlotDelete;
  promptWindowClose = x.promptWindowClose;
  showQuickStart = x.showQuickStart;
  tiedZoomGlobal = x.tiedZoomGlobal;
  curveColorSequencePalette = x.curveColorSequencePalette;

  timezone = x.timezone;
  offsetSeconds = x.offsetSeconds;

  xMajor = x.xMajor;
  yMajor = x.yMajor;
  xMinor = x.xMinor;
  yMinor = x.yMinor;
  majorColor = x.majorColor;
  minorColor = x.minorColor;
  majorGridColorDefault = x.majorGridColorDefault;
  minorGridColorDefault = x.minorGridColorDefault;

  xAxisInterpret = x.xAxisInterpret;
  xAxisInterpretation = x.xAxisInterpretation;
  xAxisDisplay = x.xAxisDisplay;
  yAxisInterpret = x.yAxisInterpret;
  yAxisInterpretation = x.yAxisInterpretation;
  yAxisDisplay = x.yAxisDisplay;

  defaultLineWeight = x.defaultLineWeight;

  emailSender = x.emailSender;
  emailSMTPServer = x.emailSMTPServer;
  emailSMTPPort = x.emailSMTPPort;
  emailRequiresAuthentication = x.emailRequiresAuthentication;
  emailUsername = x.emailUsername;
  emailPassword = x.emailPassword;
  emailEncryption = x.emailEncryption;
  emailAuthentication = x.emailAuthentication;

  printing.pageSize = x.printing.pageSize;
  printing.orientation = x.printing.orientation;
  printing.plotDateTimeFooter = x.printing.plotDateTimeFooter;
  printing.maintainAspect = x.printing.maintainAspect;
  printing.curveWidthAdjust = x.printing.curveWidthAdjust;
  printing.monochrome = x.printing.monochrome;

  printing.monochromeSettings.enhanceReadability = 
      x.printing.monochromeSettings.enhanceReadability;
  printing.monochromeSettings.pointStyleOrder = 
      x.printing.monochromeSettings.pointStyleOrder;
  printing.monochromeSettings.lineStyleOrder = 
      x.printing.monochromeSettings.lineStyleOrder;
  printing.monochromeSettings.lineWidthOrder = 
      x.printing.monochromeSettings.lineWidthOrder;
  printing.monochromeSettings.maxLineWidth = 
      x.printing.monochromeSettings.maxLineWidth;
  printing.monochromeSettings.pointDensity = 
      x.printing.monochromeSettings.pointDensity;

  return *this;
}


KstSettings *KstSettings::_self = 0L;


void KstSettings::cleanup() {
  delete _self;
  _self = 0L;
}


KstSettings *KstSettings::globalSettings() {
  if (!_self) {
    _self = new KstSettings();
    qAddPostRoutine(KstSettings::cleanup);
    _self->reload();
  }

  return _self;
}


void KstSettings::setGlobalSettings(const KstSettings *settings) {
  globalSettings(); // force instantiation
  *_self = *settings;
}


void KstSettings::save() {
  QSettings cfg("kstrc");

  cfg.beginGroup("Kst");
  cfg.setValue("Plot Update Timer", (qlonglong)plotUpdateTimer);
  cfg.setValue("Plot Font Size", (qlonglong)plotFontSize);
  cfg.setValue("Plot Font Min Size", (qlonglong)plotFontMinSize);
  cfg.setValue("Background Color", backgroundColor);
  cfg.setValue("Foreground Color", foregroundColor);
  cfg.setValue("Prompt on Plot Delete", promptPlotDelete);
  cfg.setValue("Prompt on Window Close", promptWindowClose);
  cfg.setValue("Show QuickStart", showQuickStart);
  cfg.setValue("Tied-zoom Global", tiedZoomGlobal);
  cfg.setValue("Curve Color Sequence", curveColorSequencePalette);

  cfg.setValue("Timezone", timezone);
  cfg.setValue("OffsetSeconds", offsetSeconds);

  cfg.beginGroup("Grid Lines");
  cfg.setValue("X Major", xMajor);
  cfg.setValue("Y Major", yMajor);
  cfg.setValue("X Minor", xMinor);
  cfg.setValue("Y Minor", yMinor);
  cfg.setValue("Major Color", majorColor);
  cfg.setValue("Minor Color", minorColor);
  cfg.setValue("Default Major Color", majorGridColorDefault);
  cfg.setValue("Default Minor Color", minorGridColorDefault);

  cfg.beginGroup("X Axis");
  cfg.setValue("Interpret", xAxisInterpret);
  cfg.setValue("Interpretation", xAxisInterpretation);
  cfg.setValue("Display", xAxisDisplay);
  
  cfg.beginGroup("Y Axis");
  cfg.setValue("Interpret", yAxisInterpret);
  cfg.setValue("Interpretation", yAxisInterpretation);
  cfg.setValue("Display", yAxisDisplay);

  cfg.beginGroup("Curve");
  cfg.setValue("DefaultLineWeight", defaultLineWeight);

  cfg.beginGroup("EMail");
  cfg.setValue("Sender", emailSender);
  cfg.setValue("Server", emailSMTPServer);
  cfg.setValue("Port", emailSMTPPort);
  cfg.setValue("Authenticate", emailRequiresAuthentication);
  cfg.setValue("Username", emailUsername);
  cfg.setValue("Password", emailPassword);
  cfg.setValue("Encryption", emailEncryption);
  cfg.setValue("Authentication", emailAuthentication);

  cfg.beginGroup("Printing");
  cfg.setValue("kde-pagesize", printing.pageSize);
  cfg.setValue("kde-orientation", printing.orientation);
  cfg.setValue("kst-plot-datetime-footer", printing.plotDateTimeFooter);
  cfg.setValue("kst-plot-maintain-aspect-ratio", printing.maintainAspect);
  cfg.setValue("kst-plot-curve-width-adjust", printing.curveWidthAdjust);
  cfg.setValue("kst-plot-monochrome", printing.monochrome);

  cfg.setValue("kst-plot-monochromesettings-enhancereadability", 
                 printing.monochromeSettings.enhanceReadability);
  cfg.setValue("kst-plot-monochromesettings-pointstyleorder",
                 printing.monochromeSettings.pointStyleOrder);
  cfg.setValue("kst-plot-monochromesettings-linestyleorder",
                 printing.monochromeSettings.lineStyleOrder);
  cfg.setValue("kst-plot-monochromesettings-linewidthorder",
                 printing.monochromeSettings.lineWidthOrder);
  cfg.setValue("kst-plot-monochromesettings-maxlinewidth",
                 printing.monochromeSettings.maxLineWidth);
  cfg.setValue("kst-plot-monochromesettings-pointdensity",
                 printing.monochromeSettings.pointDensity);

  cfg.sync();
}


void KstSettings::reload() {
  QSettings cfg("kstrc");

  cfg.beginGroup("Kst");
  plotUpdateTimer = cfg.value("Plot Update Timer", 200).toInt();
  plotFontSize    = cfg.value("Plot Font Size", 12).toInt();
  plotFontMinSize = cfg.value("Plot Font Min Size", 5).toInt();
  backgroundColor = cfg.value("Background Color", backgroundColor).value<QColor>();
  foregroundColor = cfg.value("Foreground Color", foregroundColor).value<QColor>();
  promptPlotDelete = cfg.value("Prompt on Plot Delete", false).toBool();
  promptWindowClose = cfg.value("Prompt on Window Close", true).toBool();
  showQuickStart = cfg.value("Show QuickStart", true).toBool();
  tiedZoomGlobal = cfg.value("Tied-zoom Global", true).toBool();
  curveColorSequencePalette = cfg.value("Curve Color Sequence", "Kst Colors").toString();
  timezone = cfg.value("Timezone", "UTC").toString();
  offsetSeconds = cfg.value("OffsetSeconds", 0).toInt();

  cfg.beginGroup("Grid Lines");
  xMajor = cfg.value("X Major", false).toBool();
  yMajor = cfg.value("Y Major", false).toBool();
  xMinor = cfg.value("X Minor", false).toBool();
  yMinor = cfg.value("Y Minor", false).toBool();
  majorColor = cfg.value("Major Color", majorColor).value<QColor>();
  minorColor = cfg.value("Minor Color", minorColor).value<QColor>();
  majorGridColorDefault = cfg.value("Default Major Color", true).toBool();
  minorGridColorDefault = cfg.value("Default Minor Color", true).toBool();

  cfg.beginGroup("X Axis");
  xAxisInterpret = cfg.value("Interpret", false).toBool();
  xAxisInterpretation = (KstAxisInterpretation)cfg.value("Interpretation", AXIS_INTERP_CTIME).toInt();
  xAxisDisplay = (KstAxisDisplay)cfg.value("Display", AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS).toInt();

  cfg.beginGroup("Y Axis");
  yAxisInterpret = cfg.value("Interpret", false).toBool();
  yAxisInterpretation = (KstAxisInterpretation)cfg.value("Interpretation", AXIS_INTERP_CTIME).toInt();
  yAxisDisplay = (KstAxisDisplay)cfg.value("Display", AXIS_DISPLAY_QTLOCALDATEHHMMSS_SS).toInt();

  cfg.beginGroup("Curve");
  defaultLineWeight = cfg.value("DefaultLineWeight", 0).toInt();

  cfg.beginGroup("EMail");
  KEMailSettings es;
  emailSender = cfg.value("Sender", es.getSetting(KEMailSettings::EmailAddress)).toString();
  emailSMTPServer = cfg.value("Server", es.getSetting(KEMailSettings::OutServer)).toString();
  emailSMTPPort = cfg.value("Port", 25).toInt();
  emailRequiresAuthentication = cfg.value("Authenticate", !es.getSetting(KEMailSettings::OutServerLogin).isEmpty()).toBool();
  emailUsername = cfg.value("Username", es.getSetting(KEMailSettings::OutServerLogin)).toString();
  emailPassword = cfg.value("Password", es.getSetting(KEMailSettings::OutServerPass)).toString();
  emailEncryption = (EMailEncryption)cfg.value("Encryption", es.getSetting(KEMailSettings::OutServerTLS) == "true" ? EMailEncryptionTLS : EMailEncryptionNone).toInt();
  emailAuthentication = (EMailAuthentication)cfg.value("Authentication", EMailAuthenticationPLAIN).toInt();

  cfg.beginGroup("Printing");
  printing.pageSize = cfg.value("kde-pagesize", QString::number((int)QPrinter::Letter)).toString();
  printing.orientation = cfg.value("kde-orientation", "Landscape").toString();
  printing.plotDateTimeFooter = cfg.value("kst-plot-datetime-footer", "0").toString();
  printing.maintainAspect = cfg.value("kst-plot-maintain-aspect-ratio", "0").toString();
  printing.curveWidthAdjust = cfg.value("kst-plot-curve-width-adjust", "0").toString();
  printing.monochrome = cfg.value("kst-plot-monochrome", "0").toString();
  printing.monochromeSettings.enhanceReadability = 
      cfg.value("kst-plot-monochromesettings-enhancereadability", "0").toString();
  printing.monochromeSettings.pointStyleOrder = 
      cfg.value("kst-plot-monochromesettings-pointstyleorder", "0").toString();
  printing.monochromeSettings.lineStyleOrder = 
      cfg.value("kst-plot-monochromesettings-linestyleorder", "1").toString();
  printing.monochromeSettings.lineWidthOrder = 
      cfg.value("kst-plot-monochromesettings-linewidthorder", "2").toString();
  printing.monochromeSettings.maxLineWidth = 
      cfg.value("kst-plot-monochromesettings-maxlinewidth", "3").toString();
  printing.monochromeSettings.pointDensity = 
      cfg.value("kst-plot-monochromesettings-pointdensity", "2").toString();
}


void KstSettings::setPrintingDefaults() {
  printing.pageSize = QString::number((int)QPrinter::Letter);
  printing.orientation = "Landscape";
  printing.plotDateTimeFooter = "0";
  printing.maintainAspect = "0";
  printing.curveWidthAdjust = "0";
  printing.monochrome = "0";
  printing.monochromeSettings.enhanceReadability = "0";
  printing.monochromeSettings.pointStyleOrder = "0";
  printing.monochromeSettings.lineStyleOrder = "1";
  printing.monochromeSettings.lineWidthOrder = "2";
  printing.monochromeSettings.maxLineWidth = "3";
  printing.monochromeSettings.pointDensity = "2";
}


int KstSettings::utcOffset() {
  return offsetSeconds;
}
