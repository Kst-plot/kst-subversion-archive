/***************************************************************************
                          kst.cpp  -  description
                             -------------------
    begin                : Tue Aug 22 13:46:13 CST 2000
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

#include <assert.h>

#include <QClipboard>
#include <QDateTime>
#include <QEvent>
#include <QEventLoop>
#include <QInputDialog>
#include <QMessageBox>
#include <qpaintdevicemetrics.h>
#include <QMenu>
#include <QPrinter>
#include <QPrintDialog> 
#include <QProgressBar>
#include <QValidator>

#include "extensiondlg.h"
#include "extensionmgr.h"
#include "kst.h"
#include "kst2dplot.h"
#include "kstchangefiledialog.h"
#include "kstchangenptsdialog.h"
#include "kstchoosecolordialog.h"
#include "kstcurvedifferentiate.h"
#include "kstcurvedialog.h"
#include "kstcsddialog.h"
#include "kstdatamanager.h"
#include "kstdatanotifier.h"
#include "kstdatawizard.h"
#include "kstdebugdialog.h"
#include "kstdebugnotifier.h"
#include "kstdoc.h"
#include "ksteqdialog.h"
#include "kstevents.h"
#include "ksteventmonitor.h"
#include "ksteventmonitorentry.h"
#include "kstfitdialog.h"
#include "kstfilterdialog.h"
#include "kstgraphfiledialog.h"
#include "ksthsdialog.h"
#include "kstvvdialog.h"
#include "kstiface_impl.h"
#include "kstimagedialog.h"
#include "kstlegenddefaults.h"
#include "kstlogwidget.h"
#include "kstmatrixdialog.h"
#include "kstmatrixdefaults.h"
#include "kstmonochromedialog.h"
#include "kstplugindialog.h"
#include "kstbasicdialog.h"
#include "kstpluginmanager.h"
#include "kstprintoptionspage.h"
#include "kstpsddialog.h"
#include "kstquickstartdialog.h"
#include "kstsettingsdlg.h"
#include "kstuinames.h"
#include "kstvectordefaults.h"
#include "kstvectorsavedialog.h"
#include "kstvectordialog.h"
#include "kstviewmanager.h"
#include "kstviewscalarsdialog.h"
#include "kstviewstringsdialog.h"
#include "kstviewvectorsdialog.h"
#include "kstviewmatricesdialog.h"
#include "kstviewfitsdialog.h"
#include "kstviewwidget.h"
#include "kstviewwindow.h"
#include "plotmimesource.h"
#include "plugincollection.h"
#include "psversion.h"
#include "statuslabel.h"
#include "sysinfo.h"
#include "updatethread.h"
#include "kstobjectdefaults.h"

#define MODE_BUTTON_ID       5

#define KST_STATUSBAR_DATA   1
#define KST_STATUSBAR_STATUS 2

static KstApp *inst = 0L;

const QString& KstApp::defaultTag = KGlobal::staticQString("<Auto Name>");

void KstApp::doubleCheckCleanup() {
  KstApp *ptr = ::inst; // guard to prevent double delete on recursion
  ::inst = 0L;

  delete ptr;
}


KstApp* KstApp::inst() {
  return ::inst;
}


static QSettings *qSettingsObject = 0L;

KstApp::KstApp(QWidget *parent, const char *name)
: QMdiArea(parent, name) {
  assert(!::inst);
  ::inst = this;

  _updatesFromScriptEnabled = true;
  _plotHolderWhileOpeningDocument = new Kst2DPlotMap;
  KGlobal::dirs()->addResourceType("kst", KStandardDirs::kde_default("data") + "kst");

  _dataSourceConfig = qSettingsObject;

  clearWFlags(WDestructiveClose);

  _stopping = false;
  config = kapp->config();
  initStatusBar();
  setStandardToolBarMenuEnabled(true);

  initDocument();
  KstDebug::self()->setHandler(doc);
  setWindowTitle(doc->title());

  _debugDialog = new KstDebugDialog(this);
// xxx  _dataManager = new KstDataManager(doc, this);
// xxx  _viewManager = new KstViewManager(doc, this);
  _viewScalarsDialog = new KstViewScalarsDialog(this);
  _viewStringsDialog = new KstViewStringsDialog(this);
  _viewVectorsDialog = new KstViewVectorsDialog(this);
  _viewMatricesDialog = new KstViewMatricesDialog(this);
  _viewFitsDialog = new KstViewFitsDialog(this);
  _changeFileDialog = new KstChangeFileDialog(this);
  _chooseColorDialog = new KstChooseColorDialog(this);
  _differentiateCurvesDialog = new KstCurveDifferentiate(this);
  _changeNptsDialog = new KstChangeNptsDialog(this);
  _graphFileDialog = new KstGraphFileDialog(this);
  _vectorSaveDialog = new KstVectorSaveDialog(this);
  _monochromeDialog = new KstMonochromeDialog(this);
  _quickStartDialog = new KstQuickStartDialog(this, 0 , true);

  initActions();
  readOptions();
  XYZoomAction->setChecked(true);
  toggleMouseMode();

  _updateThread = new UpdateThread(doc);
  _updateThread->setUpdateTime(KstSettings::globalSettings()->plotUpdateTimer);
  _updateThread->start();

  /*** Plot Dialog signals */
  connect(changeFileDialog, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(changeNptsDialog, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(graphFileDialog, SIGNAL(graphFileReq(const QString&,const QString&,int,int,bool,int)), this, SLOT(immediatePrintToPng(const QString&,const QString&,int,int,bool,int)));
  connect(graphFileDialog, SIGNAL(graphFileEpsReq(const QString&,int,int,bool,int)), this, SLOT(immediatePrintToEps(const QString&,int,int,bool,int)));

  // data manager signals
  connect(dataManager, SIGNAL(docChanged()), this, SLOT(registerDocChange()));
  connect(doc, SIGNAL(updateDialogs()), this, SLOT(updateDialogs()));
  connect(doc, SIGNAL(dataChanged()), this, SLOT(updateDataDialogs()));
  connect(dataManager, SIGNAL(editDataVector(const QString&)), KstVectorDialogI::globalInstance(), SLOT(showEdit(const QString&)));
  connect(dataManager, SIGNAL(editStaticVector(const QString&)), KstVectorDialogI::globalInstance(), SLOT(showEdit(const QString&)));
  connect(dataManager, SIGNAL(editDataMatrix(const QString&)), KstMatrixDialogI::globalInstance(), SLOT(showEdit(const QString&)));
  connect(dataManager, SIGNAL(editStaticMatrix(const QString&)), KstMatrixDialogI::globalInstance(), SLOT(showEdit(const QString&)));


  setAutoSaveSettings("KST-KMainWindow", true);
  _dcopIface = new KstIfaceImpl(doc, this);

  connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));

  QTimer::singleShot(0, this, SLOT(updateActions()));
}


KstApp::~KstApp() {
  destroyDebugNotifier();

  delete _plotHolderWhileOpeningDocument;
  _plotHolderWhileOpeningDocument = 0L;

  if (_updateThread) {
    _updateThread->setFinished(true);
    if (!_updateThread->wait(3000)) { // 3s
      _updateThread->terminate();
    }
  }

  KstDataSource::cleanupForExit(); // must be before deletions
  KstDataObject::cleanupForExit(); // must be before deletions
  delete _updateThread;
  _updateThread = 0L;
  delete _dcopIface;
  _dcopIface = 0L;
  ::inst = 0L;
  if (_dataSourceConfig) {
    _dataSourceConfig->sync();
    _dataSourceConfig = 0L;
  }
  delete qSettingsObject; // must be after cleanupForExit
  qSettingsObject = 0L;
}


QSize KstApp::sizeHint() const
{
  QSize size;
  QRect rect( KGlobalSettings::desktopGeometry(KstApp::inst()) );

  size.setWidth(3*rect.width()/4);
  size.setHeight(3*rect.height()/4);

  return size;
}


void KstApp::initialize() {
  KstSettings::checkUpdates();
  qSettingsObject = new QSettings("kstdatarc", QSettings::NativeFormat, this);
  KstDataSource::setupOnStartup(qSettingsObject);
  // NOTE: This is leaked in commandline mode if we never create a KstApp.
  //       Not too much of a problem right now, and less messy than hooking in
  //       cleanups in main.
}


void KstApp::loadExtensions() {
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  // Initialise the plugin loader and collection first.
  PluginCollection::self();

  // Do font checks after startup
  //checkFontPresent("Symbol");
  //checkFontPresent("Helvetica");

  QFont defaultFont; // Qt's current default font
  QFontInfo info(defaultFont);
  _defaultFont = info.family();
  // Null QString causes a crash in fontconfig on some systems.
  if (_defaultFont.isNull()) {
    _defaultFont = "helvetica";
  }

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QCStringList argList = args->getOptionList("E");
  ExtensionMgr *mgr = ExtensionMgr::self();
  mgr->setWindow(this);
  KService::List sl = KServiceType::offers("Kst Extension");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    KService::Ptr service = *it;
    QString name = service->property("Name").toString();
    if (!mgr->enabled(name) && !service->property("X-Kst-Enabled").toBool()) {
      continue;
    }
    mgr->loadExtension(service);
  }
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    KService::Ptr service = *it;
    QString name = service->property("Name").toString();
    KstExtension *e = mgr->extension(name);
    if (e) {
      QString ename = service->property("X-Kst-Extension-Name").toString();
      for (QCStringList::ConstIterator i = argList.begin(); i != argList.end(); ++i) {
        if (QString::fromLatin1(*i).startsWith(ename + ":")) {
          e->processArguments((*i).mid(ename.length() + 1));
        }
      }
    }
  }
  QApplication::restoreOverrideCursor();
}


const QString& KstApp::defaultFont() const {
  return _defaultFont;
}


void KstApp::checkFontPresent(const QString& font) {
  QFont f(font);
  QFontInfo info(f);

  if (info.family().toLower() != font.toLower()) {
    QString msg = QObject::tr("The %1 font was not found and was replaced by the %2 font; as a result, some labels may not display correctly.").arg(font).arg(info.family());
    KstDebug::self()->log(msg, KstDebug::Warning);
  }
}


void KstApp::EventELOGSubmitEntry(const QString& msg) {
  emit ELOGSubmitEntry(msg);
}


void KstApp::EventELOGConfigure() {
  emit ELOGConfigure();
}


void KstApp::customEvent(QEvent *pEvent) {
  if (pEvent->type() == KstELOGAliveEvent) {
    KstEventMonitorI::globalInstance()->enableELOG();
  } else if (pEvent->type() == KstELOGDeathEvent) {
    KstEventMonitorI::globalInstance()->disableELOG();
  } else if (pEvent->type() == KstELOGDebugInfoEvent) {
    QTextStream *pTextStream = (QTextStream*)pEvent->data();
    (*pTextStream) << KstDebug::self()->text();
  } else if (pEvent->type() == KstELOGConfigureEvent) {
    QTextStream *pTextStream = (QTextStream*)pEvent->data();
    if (document()) {
      document()->saveDocument(*pTextStream, true);
    }
  } else if (pEvent->type() == KstELOGCaptureEvent) {
    KstELOGCaptureStruct* pCapture = (KstELOGCaptureStruct*)pEvent->data();

    if( pCapture ) {
      QDataStream *pStream = pCapture->pBuffer;
      QSize size(pCapture->iWidth, pCapture->iHeight);
      KstViewWindow *vw = dynamic_cast<KstViewWindow*>(activeWindow());

      if (vw) {
        vw->immediatePrintToPng(pStream, size);
      }
    }
  }
}


void KstApp::updateActions() {
  // Hack
  StatusBarAction->setChecked(statusBar()->isShown());
  QApplication::flushX();
  QTimer::singleShot(0, this, SLOT(loadExtensions()));
}


void KstApp::initActions() {
  KAction *newTab = new KAction(QObject::tr("&New tab..."), 0, 0, this, SLOT(slotFileNewWindow()), actionCollection(), "file_new_window");
  newTab->setWhatsThis(QObject::tr("Create a new tab."));

  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSave->setWhatsThis(QObject::tr("Save to current Kst plot file."));

  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileSaveAs->setWhatsThis(QObject::tr("Save to new Kst plot file."));

  fileQuit = KStdAction::quit(this, SLOT(slotFileClose()), actionCollection());
  fileQuit->setWhatsThis(QObject::tr("Quit Kst."));

#if 0
  // KDE 3.3 only
  fileKeyBindings = KStdAction::keyBindings(guiFactory(), SLOT(slotConfigureKeys()), actionCollection());
#else
  fileKeyBindings = KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
#endif
  fileKeyBindings->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                     "to configure shortcuts."));

  filePreferences = KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection());
  filePreferences->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                     "to configure Kst settings."));

  fileCopy = KStdAction::copy(this, SLOT(slotCopy()), actionCollection());
  fileCopy->setWhatsThis(QObject::tr("Copy cursor position or plots to the clipboard."));

  filePaste = KStdAction::paste(this, SLOT(slotPaste()), actionCollection());
  filePaste->setWhatsThis(QObject::tr("Paste plots from the clipboard."));

  /************/
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()),
                                actionCollection());
  filePrint->setToolTip(QObject::tr("Print"));
  filePrint->setWhatsThis(QObject::tr("Print current display"));

  /************/
  StatusBarAction = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
                                              actionCollection());
  StatusBarAction->setWhatsThis(QObject::tr("Toggle Statusbar"));
  connect(StatusBarAction, SIGNAL(activated()), this, SLOT(setSettingsDirty()));

  /************/
  KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());

  /************/
  _recent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL &)), actionCollection());
  _recent->setWhatsThis(QObject::tr("Open a recently used Kst plot."));

  /************/
  PauseAction = new KToggleAction(QObject::tr("P&ause"),"player_pause", 0,
                                  actionCollection(), "pause_action");
  PauseAction->setToolTip(QObject::tr("Pause"));
  PauseAction->setWhatsThis(QObject::tr("When paused, new data will not be read."));
  connect(PauseAction, SIGNAL(toggled(bool)), this, SLOT(updatePausedState(bool)));

  /************/
  _actionSaveData = new KToggleAction(QObject::tr("Save Da&ta"), 0,0,0,0,
				actionCollection(), "save_vector_data");
  _actionSaveData->setToolTip(QObject::tr("Save Vector Data To Disk"));
  _actionSaveData->setWhatsThis(QObject::tr("When selected, data in vectors will be saved into the Kst file."));

  /************/
  XYZoomAction = new KRadioAction(QObject::tr("XY Mouse &Zoom"), "kst_zoomxy",
                                  KShortcut(Key_F2),
                                  this, SLOT(toggleMouseMode()),
                                  actionCollection(), "zoomxy_action");
  XYZoomAction->setExclusiveGroup("gfx");
  XYZoomAction->setToolTip(QObject::tr("XY mouse zoom"));
  XYZoomAction->setWhatsThis(QObject::tr("XY zoom: mouse zooming affects\n"
                                  "both X and Y axis"));
  XYZoomAction->setChecked(true);

  /************/
  XZoomAction = new KRadioAction(QObject::tr("&X Mouse Zoom"), "kst_zoomx",
                                 KShortcut(Key_F3),
                                 this, SLOT(toggleMouseMode()),
                                 actionCollection(), "zoomx_action");
  XZoomAction->setExclusiveGroup("gfx");
  XZoomAction->setToolTip(QObject::tr("X mouse zoom"));
  XZoomAction->setWhatsThis(QObject::tr("X zoom: Mouse zooming affects only the\n"
                                 "X axis (CTRL-mouse also does this)"));

  /************/
  YZoomAction = new KRadioAction(QObject::tr("&Y Mouse Zoom"), "kst_zoomy",
                                  KShortcut(Key_F4),
                                  this, SLOT(toggleMouseMode()),
                                  actionCollection(), "zoomy_action");
  YZoomAction->setExclusiveGroup("gfx");
  YZoomAction->setToolTip(QObject::tr("Y mouse zoom"));
  YZoomAction->setWhatsThis(QObject::tr("Y zoom: Mouse zooming affects only the\n"
                                 "Y axis (SHIFT-mouse also does this)"));

  /************/
  GfxAction = new KRadioAction(QObject::tr("&Graphics Mode"), "kst_graphics", 0,
                                this, SLOT(toggleMouseMode()),
                                actionCollection(), "graphics_action");
  GfxAction->setExclusiveGroup("zoom");
  GfxAction->setToolTip(QObject::tr("Graphics Editor"));
  GfxAction->setWhatsThis(QObject::tr("Use the mouse to create and edit graphics objects."));

  /************/


  /************/
  NewPlotAction = new KAction(QObject::tr("New Plot"), "kst_newplot", 0,
                                 this, SLOT(newPlot()),
                                 actionCollection(), "newplot_action");
  NewPlotAction->setWhatsThis(QObject::tr("Create a new plot in the\n"
                                      "current window."));

  /************/
  DataManagerAction = new KAction(QObject::tr("&Data Manager"), "kst_datamanager", 0,
                                  dataManager, SLOT(show_I()),
                                  actionCollection(), "datamanager_action");
  DataManagerAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                       "to manage data."));

  /************/
  ViewManagerAction = new KAction(QObject::tr("&View Manager"), "kst_viewmanager", 0,
                                  viewManager, SLOT(show_I()),
                                  actionCollection(), "viewmanager_action");
  ViewManagerAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                       "to manage views."));
  
  /************/
  VectorDialogAction = new KAction(QObject::tr("New &Vector..."), "kst_vectornew", 0,
                                 KstVectorDialogI::globalInstance(),
                                 SLOT(show()), actionCollection(),
                                 "vectordialog_action");
  VectorDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                        "to create a new vector."));

  /************/
  CurveDialogAction = new KAction(QObject::tr("New &Curve..."), "kst_curvenew", 0,
                                  KstCurveDialogI::globalInstance(),
                                  SLOT(show()), actionCollection(),
                                  "curvedialog_action");
  CurveDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                       "to create a new curve."));

  /************/
  CsdDialogAction = new KAction(QObject::tr("New &Spectrogram..."), "kst_csdnew", 0,
                                KstCsdDialogI::globalInstance(),
                                SLOT(show()), actionCollection(),
                                "csddialog_action");
  CsdDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                     "to create a new spectrogram."));


  /************/
  EqDialogAction = new KAction(QObject::tr("New &Equation..."),
                               "kst_equationnew", 0,
                               KstEqDialogI::globalInstance(),
                               SLOT(show()),
                               actionCollection(), "eqdialog_action");
  EqDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                    "to create a new equation."));

  /************/
  HsDialogAction = new KAction(QObject::tr("New &Histogram..."),
                               "kst_histogramnew", 0,
                               KstHsDialogI::globalInstance(),
                               SLOT(show()), actionCollection(),
                               "hsdialog_action");
  HsDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                    "to create a new histogram."));

  /************/
  PsdDialogAction = new KAction(QObject::tr("New &Spectrum..."),
                                "kst_psdnew", 0,
                                KstPsdDialogI::globalInstance(),
                                SLOT(show()), actionCollection(),
                                "psddialog_action");
  PsdDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                     "to create a new spectrum."));

  /************/
  PluginDialogAction = new KAction(QObject::tr("New &Plugin..."),
                                  "kst_pluginnew", 0,
                                   this, SLOT(selectDataPlugin()), actionCollection(),
                                   "plugindialog_action");
  PluginDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                        "to create a new plugin instance."));

  /************/
  MatrixDialogAction = new KAction(QObject::tr("New M&atrix..."), "kst_matrixnew", 0,
                                   KstMatrixDialogI::globalInstance(),
                                   SLOT(show()), actionCollection(),
                                   "matrixdialog_action");
  MatrixDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                        "to create a new matrix."));

  /************/
  ImageDialogAction = new KAction(QObject::tr("New &Image..."),
                                   "kst_imagenew", 0,
                                   KstImageDialogI::globalInstance(),
                                   SLOT(show()), actionCollection(),
                                   "imagedialog_action");
  ImageDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                        "to create a new image instance."));

  /************/
  ChangeFileDialogAction = new KAction(QObject::tr("Change Data &File..."),
                                       "kst_changefile", 0, this,
                                       SLOT(showChangeFileDialog()),
                                       actionCollection(),
                                       "changefiledialog_action");
  ChangeFileDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to change input files."));

  /************/
  ChooseColorDialogAction = new KAction(QObject::tr("Assign Curve &Color Per File..."),
                                        "kst_choosecolor", 0, this,
                                        SLOT(showChooseColorDialog()),
                                        actionCollection(),
                                        "choosecolordialog_action");
  ChooseColorDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                             "to change curve colors\n"
                                             "based on data file."));

  /************/
  DifferentiateCurvesDialogAction = new KAction(QObject::tr("&Differentiate Between Curves..."),
                                        "kst_differentiatecurves", 0, this,
                                        SLOT(showDifferentiateCurvesDialog()),
                                        actionCollection(),
                                        "differentiatecurves_action");
  DifferentiateCurvesDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                             "to differentiate between curves."));

/* xxx
  ViewScalarsDialogAction = new KAction(QObject::tr("View &Scalar Values"),
                                       0, 0, this,
                                       SLOT(showViewScalarsDialog()),
                                       actionCollection(),
                                       "viewscalarsdialog_action");
  ViewScalarsDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to view scalar values."));
  ViewScalarsDialogAction->setEnabled(false);

  ViewStringsDialogAction = new KAction(QObject::tr("View Strin&g Values"),
                                       0, 0, this,
                                       SLOT(showViewStringsDialog()),
                                       actionCollection(),
                                       "viewstringsdialog_action");
  ViewStringsDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to view string values."));

  ViewVectorsDialogAction = new KAction(QObject::tr("View Vec&tor Values"),
                                       0, 0, this,
                                       SLOT(showViewVectorsDialog()),
                                       actionCollection(),
                                       "viewvectorsdialog_action");
  ViewVectorsDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to view vector values."));
  ViewVectorsDialogAction->setEnabled(false);


  ViewMatricesDialogAction = new KAction(QObject::tr("View &Matrix Values"),
                                       0, 0, this,
                                       SLOT(showViewMatricesDialog()),
                                       actionCollection(),
                                       "viewmatricesdialog_action");
  ViewMatricesDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to view matrix values."));
  ViewMatricesDialogAction->setEnabled(false);

  ViewFitsDialogAction = new KAction(QObject::tr("View &Fit Results"),
                                       0, 0, this,
                                       SLOT(showViewFitsDialog()),
                                       actionCollection(),
                                       "viewfitsdialog_action");
  ViewFitsDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to view fit values."));
  ViewFitsDialogAction->setEnabled(false);

*/
  ChangeNptsDialogAction = new KAction(QObject::tr("Change Data Sample &Ranges..."),
                                       "kst_changenpts", 0, this,
                                       SLOT(showChangeNptsDialog()),
                                       actionCollection(),
                                       "changenptsdialog_action");
  ChangeNptsDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                            "to change data sample ranges."));

  /************/
  EventMonitorAction = new KAction(QObject::tr("New Event &Monitor..."),
                                     "kst_eventnew", 0,
                                     KstEventMonitorI::globalInstance(),
                                     SLOT(show()),
                                     actionCollection(),
                                     "eventmonitor_action");
  EventMonitorAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                          "create a new event monitor."));

  /************/
  GraphFileDialogAction = new KAction(QObject::tr("Export to Graphics File..."),
                                  "thumbnail", 0,
                                  this, SLOT(showGraphFileDialog()),
                                  actionCollection(),
                                  "graphfiledialog_action");
  GraphFileDialogAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                           "to export the plot as a\n"
                                           "graphics file."));

  /************/
  _vectorSaveAction = new KAction(QObject::tr("Save Vectors to Disk..."),
                                  0, 0,
                                  vectorSaveDialog, SLOT(show()),
                                  actionCollection(),
                                  "vectorsavedialog_action");
  _vectorSaveAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                       "to save vectors to text files."));

  /************/
  SamplesDownAction = new KAction(QObject::tr("&Back 1 Screen"),
                                  "player_rew",
                                  KShortcut(CTRL + Key_Left),
                                  this, SLOT(samplesDown()),
                                  actionCollection(),
                                  "samplesdown_action");
  //SamplesDownAction->setToolTip(QObject::tr("Back"));
  SamplesDownAction->setWhatsThis(QObject::tr("Reduce the starting frame by\n"
                                       "the current number of frames."));


  /************/
  SamplesUpAction = new KAction(QObject::tr("&Advance 1 Screen"),
                                "player_fwd",
                                KShortcut(CTRL + Key_Right),
                                this, SLOT(samplesUp()),
                                actionCollection(),
                                "samplesup_action");

  //SamplesUpAction->setToolTip(QObject::tr("Advance"));
  SamplesUpAction->setWhatsThis(QObject::tr("Increase the starting frame by\n"
                                     "the current number of frames."));

  /************/
  SamplesFromEndAction = new KAction(QObject::tr("Read From &End"),
                                     "player_end",
                                     KShortcut(SHIFT + CTRL + Key_Right),
                                     this, SLOT(fromEnd()),
                                     actionCollection(),
                                     "samplesend_action");
  SamplesFromEndAction->setToolTip(QObject::tr("Read from end"));
  SamplesFromEndAction->setWhatsThis(QObject::tr("Read current data from end of file."));

  /************/
#if 0
  PluginManagerAction = new KAction(QObject::tr("&Plugins..."), 0, 0,
                                 this, SLOT(showPluginManager()),
                                 actionCollection(), "pluginmanager_action");
  PluginManagerAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                           "to manage plugins."));
#endif

  /************/
  ExtensionManagerAction = new KAction(QObject::tr("&Extensions..."), 0, 0,
                                 this, SLOT(showExtensionManager()),
                                 actionCollection(), "extensionmanager_action");
  ExtensionManagerAction->setWhatsThis(QObject::tr("Bring up a dialog box\n"
                                           "to manage extensions."));


  /************/
  DataWizardAction = new KAction(QObject::tr("Data &Wizard"), "wizard", 
                                 KShortcut(CTRL+ALT+Key_W),
                                 this, SLOT(showDataWizard()),
                                 actionCollection(), "datawizard_action");
  DataWizardAction->setWhatsThis(QObject::tr("Bring up a wizard\n"
                                           "to easily load data."));


  /************/
  DebugDialogAction = new KAction(QObject::tr("Debug Kst..."), 0, 0,
                                 this, SLOT(showDebugDialog()),
                                 actionCollection(), "debug_action");
  DebugDialogAction->setWhatsThis(QObject::tr("Bring up a dialog\n"
                                           "to display debugging information."));


  /************/
  DataMode = new KToggleAction(QObject::tr("Data Mode"), "kst_datamode", 0,
                                 this, SLOT(toggleDataMode()),
                                 actionCollection(), "datamode_action");
  DataMode->setWhatsThis(QObject::tr("Toggle between cursor mode and data mode."));
  DataMode->setToolTip(QObject::tr("Data mode"));
  /************/
  _reloadAction = new KAction(QObject::tr("Reload"), "reload", Key_F5, this, SLOT(reload()),
                              actionCollection(), "reload");
  _reloadAction->setWhatsThis(QObject::tr("Reload the data from file."));

  _tiedZoomAction = new KAction(QObject::tr("&Tied Zoom"),"kst_zoomtie", 0,
                               this, SLOT(tieAll()),
                               actionCollection(), "zoomtie_action");
  _tiedZoomAction->setToolTip(QObject::tr("Enable tied zoom"));
  _tiedZoomAction->setWhatsThis(QObject::tr("Apply zoom actions to all plots\n"
                                     "(not just the active one)."));

  _gfxRectangleAction = new KRadioAction(QObject::tr("&Box"), "kst_gfx_rectangle", 
                                  KShortcut(Key_F8),
                                  this, SLOT(toggleMouseMode()),
                                  actionCollection(), "rectangle_action");
  _gfxRectangleAction->setExclusiveGroup("gfx");
  _gfxRectangleAction->setToolTip(QObject::tr("Draw box"));
  _gfxRectangleAction->setWhatsThis(QObject::tr("Draw box"));

  _gfxEllipseAction = new KRadioAction(QObject::tr("&Ellipse"), "kst_gfx_ellipse",
                                  KShortcut(Key_F9),
                                  this, SLOT(toggleMouseMode()),
                                  actionCollection(), "ellipse_action");
  _gfxEllipseAction->setExclusiveGroup("gfx");
  _gfxEllipseAction->setToolTip(QObject::tr("Draw ellipse"));
  _gfxEllipseAction->setWhatsThis(QObject::tr("Draw ellipse"));

  _gfxPictureAction = new KRadioAction(QObject::tr("&Picture"), "kst_gfx_picture",
                                   KShortcut(Key_F12),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "picture_action");
  _gfxPictureAction->setExclusiveGroup("gfx");
  _gfxPictureAction->setToolTip(QObject::tr("Insert picture"));
  _gfxPictureAction->setWhatsThis(QObject::tr("Insert picture"));

  _gfx2DPlotAction = new KRadioAction(QObject::tr("&Plot"), "kst_newplot",
                                   KShortcut(CTRL+Key_2),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "2dplot_action");
  _gfx2DPlotAction->setExclusiveGroup("gfx");
  _gfx2DPlotAction->setToolTip(QObject::tr("Insert Plot"));
  _gfx2DPlotAction->setWhatsThis(QObject::tr("Insert Plot"));

  _gfxArrowAction = new KRadioAction(QObject::tr("&Arrow"), "kst_gfx_arrow",
                                   KShortcut(Key_F11),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "arrow_action");
  _gfxArrowAction->setExclusiveGroup("gfx");
  _gfxArrowAction->setToolTip(QObject::tr("Draw arrow"));
  _gfxArrowAction->setWhatsThis(QObject::tr("Draw arrow"));

  _gfxLegendAction = new KRadioAction(QObject::tr("&Legend"), "kst_gfx_legend",
                                   KShortcut(CTRL+Key_3),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "legend_action");
  _gfxLegendAction->setExclusiveGroup("gfx");
  _gfxLegendAction->setToolTip(QObject::tr("Insert Legend"));
  _gfxLegendAction->setWhatsThis(QObject::tr("Insert Legend"));

  _gfxLineAction = new KRadioAction(QObject::tr("&Line"), "kst_gfx_line",
                                   KShortcut(Key_F10),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "line_action");
  _gfxLineAction->setExclusiveGroup("gfx");
  _gfxLineAction->setToolTip(QObject::tr("Draw line"));
  _gfxLineAction->setWhatsThis(QObject::tr("Draw line"));
  _gfxLineAction->setChecked(true);

  /************/
  _gfxLabelAction = new KRadioAction(QObject::tr("L&abel"), "text",
                                   KShortcut(Key_F7),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "label_action");
  _gfxLabelAction->setExclusiveGroup("gfx");
  _gfxLabelAction->setToolTip(QObject::tr("Draw label"));
  _gfxLabelAction->setWhatsThis(QObject::tr("Draw label"));

  LayoutAction = new KRadioAction(QObject::tr("Layout Mode"), "kst_layoutmode",
                                   KShortcut(Key_F6),
                                   this, SLOT(toggleMouseMode()),
                                   actionCollection(), "layoutmode_action");
  LayoutAction->setExclusiveGroup("gfx");
  LayoutAction->setToolTip(QObject::tr("Layout mode"));
  LayoutAction->setWhatsThis(QObject::tr("Use this mode to move, resize, and group plots."));

  // this is the mouse mode menu
  QPopupMenu* mouseModeMenu = new QPopupMenu(this);

  XYZoomAction->plug(mouseModeMenu);
  XZoomAction->plug(mouseModeMenu);
  YZoomAction->plug(mouseModeMenu);
  mouseModeMenu->insertSeparator();
  LayoutAction->plug(mouseModeMenu);
  mouseModeMenu->insertSeparator();
  _gfxLabelAction->plug(mouseModeMenu);
  _gfxRectangleAction->plug(mouseModeMenu);
  _gfxEllipseAction->plug(mouseModeMenu);
  _gfxLineAction->plug(mouseModeMenu);
  _gfxArrowAction->plug(mouseModeMenu);
  _gfxPictureAction->plug(mouseModeMenu);
  _gfx2DPlotAction->plug(mouseModeMenu);
  _gfxLegendAction->plug(mouseModeMenu);

  toolBar()->insertButton("thumbnail", MODE_BUTTON_ID, mouseModeMenu, true, QObject::tr("Select the desired mode"));
  toggleMouseMode();

  createGUI(0L);
}


void KstApp::slotConfigureKeys() {
#if 0
  // KDE 3.3 only:
  KKeyDialog::configure(actionCollection(), this);
#else
  KKeyDialog dlg(true, this);
  dlg.insert(actionCollection());
  ExtensionMgr *mgr = ExtensionMgr::self();
  const QMap<QString, bool>& extensions = mgr->extensions();
  for (QMap<QString, bool>::ConstIterator it = extensions.begin(); it != extensions.end(); ++it) {
    if (it.data()) {
      KXMLGUIClient *gc = dynamic_cast<KXMLGUIClient*>(mgr->extension(it.key()));
      if (gc) {
        KActionCollection *ac = gc->actionCollection();
        if (ac) {
          dlg.insert(ac);
        }
      }
    }
  }
  dlg.configure(true);
#endif
}


void KstApp::waitForUpdate() const {
  _updateThread->waitForUpdate();
}


bool KstApp::paused() const {
  return _updateThread->paused();
}


void KstApp::setPaused(bool in_paused) {
  PauseAction->setChecked(in_paused);
  _updateThread->setPaused(in_paused);
}


void KstApp::togglePaused() {
  setPaused(!PauseAction->isChecked());
}


KstApp::KstZoomType KstApp::getZoomRadio() {
  if (XZoomAction->isChecked()) {
    return XZOOM;
  } else if (YZoomAction->isChecked()) {
    return YZOOM;
  } else if (LayoutAction->isChecked()) {
    return LAYOUT;
  } else if (GfxAction->isChecked()) {
    return GRAPHICS;
  } else {
    return XYZOOM;
  }
}


void KstApp::initStatusBar() {
  _dataNotifier = new KstDataNotifier(statusBar());
  statusBar()->addWidget(_dataNotifier, 0, true);

  _dataBar = new StatusLabel(QString::null, statusBar());
  _dataBar->setTextFormat(Qt::PlainText);
  statusBar()->addWidget(_dataBar, 5, true);

  _readyBar = new StatusLabel(QObject::tr("Almost Ready"), statusBar());
  _readyBar->setTextFormat(Qt::PlainText);
  _readyBar->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusBar()->addWidget(_readyBar, 5, true);

  _progressBar = new KProgress(statusBar());
  _progressBar->setPercentageVisible(false);
  _progressBar->setCenterIndicator(true);
  statusBar()->addWidget(_progressBar, 2, true);
  _progressBar->setMaximumHeight( fontMetrics().height() );
  _progressBar->hide();

#ifdef HAVE_LINUX
  _memoryBar = new StatusLabel(QObject::tr("0 MB available"), statusBar());
  _memoryBar->setTextFormat(Qt::PlainText);
  statusBar()->addWidget(_memoryBar, 0, true);
  connect(&_memTimer, SIGNAL(timeout()), this, SLOT(updateMemoryStatus()));
  _memTimer.start(5000);
#endif

  statusBar()->show();

  slotUpdateMemoryMsg(QObject::tr("0 MB available"));
  slotUpdateStatusMsg(QObject::tr("Ready"));
  slotUpdateDataMsg(QString::null);
}


void KstApp::initDocument() {
  doc = new KstDoc(this);
  QTimer::singleShot(0, this, SLOT(delayedDocInit()));
}


void KstApp::delayedDocInit() {
  if (!activeWindow()) {
    doc->newDocument();
  }
}


void KstApp::selectDataPlugin() {
  QStringList l;

  // the KstDataObject plugins...
  QStringList dataObjectPlugins;
  const KstPluginInfoList pluginInfo = KstDataObject::pluginInfoList();
  {
    KstPluginInfoList::ConstIterator it = pluginInfo.begin();
    for (; it != pluginInfo.end(); ++it) {
      dataObjectPlugins << it.key();
    }
  }

  l += dataObjectPlugins;

  // the C-style plugins...
  QStringList cPlugins;
  const QMap<QString,QString> readable = PluginCollection::self()->readableNameList();
  {
    QMap<QString,QString>::const_iterator it = readable.begin();
    for (; it != readable.end(); ++it) {
      cPlugins << it.key();
    }
  }

  l += cPlugins;

  //
  // list the KstDataObject and C-style plugins together in ascending alphabetical order...
  //
  l.sort();

  bool ok = false;
  QStringList plugin = KInputDialog::getItemList(QObject::tr("Data Plugins"), QObject::tr("Create..."), l, 0, false, &ok, this);

  if (ok && !plugin.isEmpty()) {
    const QString p = plugin.join("");

    if (dataObjectPlugins.contains(p)) {
      KstDataObjectPtr ptr = KstDataObject::plugin(p);
      ptr->showDialog(true);
    } else if (cPlugins.contains(p)) {
      KstPluginDialogI::globalInstance()->showNew(readable[p]);
    }
  }
}


void KstApp::forceUpdate() {
  _updateThread->forceUpdate();
}


void KstApp::addRecentFile(const KURL& url) {
  _recent->addURL(url);
}


void KstApp::selectRecentFile(const KURL &url) {
  if (url.isEmpty()) {
    _recent->setCurrentItem(-1);
  } else {
    QStringList urls = _recent->items();
    int count = urls.count();
    for (int i = 0; i < count; ++i) {
      if (KURL(urls[i]) == url) {
        _recent->setCurrentItem(i);
        break;
      }
    }
  }
}


void KstApp::doDelayedOpens() {
  QValueList<KstOpen> queueCopy = QDeepCopy<QValueList<KstOpen> >(_openQueue);
  _openQueue.clear();

  for (QValueList<KstOpen>::ConstIterator i = queueCopy.begin(); i != queueCopy.end(); ++i) {
    openDocumentFile((*i).filename, (*i).file, (*i).n, (*i).f, (*i).s, (*i).ave);
  }
}


bool KstApp::openDocumentFile(const QString& in_filename,
    const QString& o_file, int o_n, int o_f, int o_s, bool o_ave, bool delayed) {
  static bool opening = false;

  if (delayed || opening) {
    KstOpen job;
    job.filename = in_filename;
    job.file = o_file;
    job.n = o_n;
    job.f = o_f;
    job.s = o_s;
    job.ave = o_ave;
    _openQueue.append(job);
    QTimer::singleShot(0, this, SLOT(doDelayedOpens()));
    return true;
  }

  opening = true;

  KURL url;
  bool rc = false;

  if (QFile::exists(in_filename) && QFileInfo(in_filename).isRelative()) {
    url.setPath(QFileInfo(in_filename).absFilePath());
  } else {
    url = KURL::fromPathOrURL(in_filename);
  }

  slotUpdateStatusMsg(QObject::tr("Opening %1...").arg(url.prettyURL()));

  if (doc->openDocument(url, o_file, o_n, o_f, o_s, o_ave)) {
    setCaption(doc->title());
    if (url.isLocalFile()) {
      QFileInfo finfo(in_filename);
      QString fileExport;

      fileExport = finfo.absFilePath();
      addRecentFile(fileExport);
      if (fileExport.endsWith(QString(".kst"), FALSE)) {
        fileExport.truncate(fileExport.length() - QString(".kst").length());
      }
      graphFileDlg()->setURL(fileExport);
    } else {
      addRecentFile(url);
    }

    rc = true;
  }

  slotUpdateStatusMsg(QObject::tr("Ready"));
  opening = false;

  return rc;
}


KstDoc *KstApp::document() const {
  return _doc;
}


void KstApp::saveOptions() {
  config->setGroup("General Options");
  _recent->saveEntries(config, "Recent Files");

  KST::legendDefaults.writeConfig(config);
  KST::vectorDefaults.writeConfig(config);
  KST::matrixDefaults.writeConfig(config);
  KST::objectDefaults.writeConfig(config);
  config->sync();
}


void KstApp::readOptions() {
  config->setGroup("General Options");
  _recent->loadEntries(config, "Recent Files");

  KST::legendDefaults.readConfig(config);
  KST::vectorDefaults.readConfig(config);
  KST::matrixDefaults.readConfig(config);
  KST::objectDefaults.readConfig(config);

  switchToTabPageMode();
}


void KstApp::saveProperties(QSettings *config) {
  QString name = doc->absFilePath();

  if (!name.isEmpty() && doc->title() != "Untitled") {
    config->setValue("Document", name);
    config->setValue("NamedDocument", true);
  } else {
    QString sl = KGlobal::dirs()->saveLocation("kst", "kst/");
    int i = 0;
    do {
      name = sl + QString("unsaved%1.kst").arg(i);
    } while(QFile::exists(name));
    doc->saveDocument(name, false, false);
    config->setValue("Document", name);
    config->setValue("NamedDocument", false);
  }
}


void KstApp::readProperties(QSettings* config) {
  QString name = config->value("Document").toString();

  if (name.isEmpty()) {
    return;
  }

  if (config->value("NamedDocument", false).toBool()) {
    _doc->openDocument(name);
  } else {
    _doc->openDocument(name);
    QFile::remove(name);
    _doc->setTitle("Untitled");
  }
}


bool KstApp::queryClose() {
  if (_doc->saveModified()) {
     _doc->cancelUpdate();
    _stopping = true;
    QTimer::singleShot(0, doc, SLOT(deleteContents()));

    return true;
  }

  return false;
}


bool KstApp::queryExit() {
  saveOptions();

  return true;
}


void KstApp::slotFileNew() {
  slotUpdateStatusMsg(QObject::tr("Creating new document..."));

  if (doc->saveModified()) {
    doc->newDocument();
    setWindowTitle(doc->title());
    selectRecentFile(KURL(""));
  }

  slotUpdateStatusMsg(QObject::tr("Ready"));
}


void KstApp::slotFileOpen() {
  slotUpdateStatusMsg(QObject::tr("Opening file..."));
/* xxx
  if (_doc->saveModified(false)) {
    QUrl url = QFileDialog::getOpenURL(this, QObject::tr("Open File"), "::<kstfiledir>", QObject::tr("*.kst|Kst Plot File (*.kst)\n*|All Files"), this, );
    if (!url.isEmpty()) {
      _doc->deleteContents();
      _doc->setModified(false);
      if (_doc->openDocument(url)) {
        setWindowTitle(doc->title());
        addRecentFile(url);
      }
    }
  }
*/
  slotUpdateStatusMsg(QObject::tr("Ready"));
}


bool KstApp::slotFileOpenRecent(const QUrl& newfile) {
  bool returnVal = false;
  slotUpdateStatusMsg(QObject::tr("Opening file..."));

  if (_doc->saveModified()) {
    if (_doc->openDocument(newfile)) {
      returnVal = true;
    }
    setWindowTitle(kapp->caption() + ": " + doc->title());
  }

  slotUpdateStatusMsg(QObject::tr("Ready"));

  return returnVal;
}


void KstApp::slotFileSave() {
  if (_doc->title() == "Untitled") {
    slotFileSaveAs();
  } else {
    slotUpdateStatusMsg(QObject::tr("Saving file..."));
    doc->saveDocument(doc->absFilePath(), false, false);
    slotUpdateStatusMsg(QObject::tr("Ready"));
  }
}


bool KstApp::slotFileSaveAs() {
  slotUpdateStatusMsg(QObject::tr("Saving file with a new filename..."));

  while (true) {
    QString folder;
    QString newName;

    if (_doc->lastFilePath().isEmpty()) {
      folder = QDir::currentPath();
    } else {
      folder = doc->lastFilePath();
    }

    newName = QFileDialog::getSaveFileName(this, QObject::tr("Save As"),
                  folder, QObject::tr("*.kst|Kst Plot File (*.kst)\n*|All Files"));
    if (!newName.isEmpty()) {
      QRegExp extension("*.kst", Qt::CaseInsensitive, QRegExp::WildCard);
      QString longName = newName;

      if (!extension.exactMatch(newName)) {
        longName = newName + QString(".kst");
      }
      if (_doc->saveDocument(longName, false, true)) {
        QFileInfo saveAsInfo(longName);

        addRecentFile(longName);
        _doc->setTitle(saveAsInfo.fileName());
        _doc->setAbsFilePath(saveAsInfo.absoluteFilePath());

        setWindowTitle(/* xxx kapp->caption()*/ + ": " + _doc->title());

        slotUpdateStatusMsg(QObject::tr("Ready"));

        return true;
      }
    } else {
      slotUpdateStatusMsg(QObject::tr("Ready"));

      return false;
    }
  }
}


void KstApp::slotFileClose() {
  if (_doc->saveModified()) {
    _doc->cancelUpdate();
    _stopping = true;
    QTimer::singleShot(0, _doc, SLOT(deleteContents()));
    close();
  }
}


void KstApp::immediatePrintWindowToFile(QMdiSubWindow *window, const QString& filename) {
  KstViewWindow *viewWindow;

  viewWindow = dynamic_cast<KstViewWindow*>(window);
  if (viewWindow && !viewWindow->view()->children().isEmpty()) {
    viewWindow->immediatePrintToFile(filename);
  }
}


void KstApp::immediatePrintActiveWindowToFile(const QString& filename) {
  KstViewWindow *viewWindow;

  viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
  if (viewWindow) {
    viewWindow->immediatePrintToFile(filename);
  }
}


void KstApp::immediatePrintWindowToPng(QMdiSubWindow *window, const QString& filename, const QString& format, int width, int height, int display) {
  KstViewWindow *viewWindow;

  viewWindow = dynamic_cast<KstViewWindow*>(window);
  if (viewWindow && !viewWindow->view()->children().isEmpty()) {
    QSize size;

    if (display == 0) {
      size.setWidth(width);
      size.setHeight(height);
    } else if (display == 1) {
      size.setWidth(width);
      size.setHeight(width);
    } else if (display == 2) {
      QSize sizeWindow(viewWindow->geometry().size());

      size.setWidth(width);
      size.setHeight((int)((double)width * (double)sizeWindow.height() / (double)sizeWindow.width()));
    } else {
      QSize sizeWindow(viewWindow->geometry().size());

      size.setHeight(height);
      size.setWidth((int)((double)height * (double)sizeWindow.width() / (double)sizeWindow.height()));
    }

    viewWindow->immediatePrintToPng(filename, size, format);
  }
}


void KstApp::immediatePrintActiveWindowToPng(const QString& filename, const QString& format, int width, int height, int display) {
  QMdiSubWindow *window;

  window = activeSubWindow();
  if (window) {
    immediatePrintWindowToPng(window, filename, format, width, height, display);
  }
}


void KstApp::immediatePrintWindowToEps(QMdiSubWindow *window, const QString& filename, int width, int height, int display) {
  KstViewWindow *view;

  view = dynamic_cast<KstViewWindow*>(window);
  if (view && !view->view()->children().isEmpty()) {
    QSize size;

    if (display == 0) {
      size.setWidth(width);
      size.setHeight(height);
    } else if (display == 1) {
      size.setWidth(width);
      size.setHeight(width);
    } else if (display == 2) {
      QSize sizeWindow(view->geometry().size());

      size.setWidth(width);
      size.setHeight((int)((double)width * (double)sizeWindow.height() / (double)sizeWindow.width()));
    } else {
      QSize sizeWindow(view->geometry().size());

      size.setHeight(height);
      size.setWidth((int)((double)height * (double)sizeWindow.width() / (double)sizeWindow.height()));
    }

    view->immediatePrintToEps(filename, size);
  }
}


void KstApp::immediatePrintActiveWindowToEps(const QString& filename, int width, int height, int display) {
  QMdiSubWindow *window = activeSubWindow();

  if (window) {
    immediatePrintWindowToEps(window, filename, width, height, display);
  }
}


void KstApp::slotFilePrint() {

  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;
  Kst2DPlotPtr rc;
  KstViewWindow *currentViewWindow;
  int currentPage = 0;
  int pages = 0;
/* xxx
  currentViewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
  windows = app->subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      if (viewWindow && !viewWindow->view()->children().isEmpty()) {
        ++pages;
      }

      if (viewWindow == currentViewWindow) {
        currentPage = pages;
      }
    }
  }
*/
  if (pages > 0) {
    QPrinter printer(QPrinter::HighResolution);
    KstSettings *ks = KstSettings::globalSettings();
    QPrinterDialog printerdlg(&printer);
/* xxx   
    printer.setOption("kde-pagesize", ks->printing.pageSize);
    printer.setOption("kde-orientation", ks->printing.orientation);
    printer.setOption("kst-plot-datetime-footer", ks->printing.plotDateTimeFooter);
    printer.setOption("kst-plot-maintain-aspect-ratio", ks->printing.maintainAspect);
    printer.setOption("kst-plot-curve-width-adjust", ks->printing.curveWidthAdjust);
    printer.setOption("kst-plot-monochrome", ks->printing.monochrome);
    // additional monochrome settings
    printer.setOption("kst-plot-monochromesettings-enhancereadability", ks->printing.monochromeSettings.enhanceReadability);
    printer.setOption("kst-plot-monochromesettings-pointstyleorder", ks->printing.monochromeSettings.pointStyleOrder);
    printer.setOption("kst-plot-monochromesettings-linestyleorder", ks->printing.monochromeSettings.lineStyleOrder);
    printer.setOption("kst-plot-monochromesettings-linewidthorder", ks->printing.monochromeSettings.lineWidthOrder);
    printer.setOption("kst-plot-monochromesettings-maxlinewidth", ks->printing.monochromeSettings.maxLineWidth);
    printer.setOption("kst-plot-monochromesettings-pointdensity", ks->printing.monochromeSettings.pointDensity);
*/  
    printerdlg.setFromTo(0, 0);
    printerdlg.setMinMax(1, pages);
// xxx    printer.setCurrentPage(currentPage);
// xxx    printer.setPageSelection(KPrinter::ApplicationSide);
  
    pages = 0;
// xxx    printerdlg.KPrinter::addDialogPage(new KstPrintOptionsPage);
    printerdlg.setWindowTitle(QObject::tr("Print"));
    if (!printerdlg.exec()) {
      return;
    }
  
    KstPainter paint(KstPainter::P_PRINT);
    paint.begin(&printer);
    QSize size(printer.width(), printer.height());
    bool datetimeFooter;
    bool maintainAspectRatio;
    bool monochrome;
    int lineAdjust;
    // additional monochrome options
    bool enhanceReadability;
    int pointStyleOrder, lineStyleOrder, lineWidthOrder, maxLineWidth, pointDensity;
  
    slotUpdateStatusMsg(QObject::tr("Printing..."));
 /* xxx 
    // make sure defaults are set for settings that are not overwritten
    ks->setPrintingDefaults();
  
    if (!printer.option("kde-pagesize").isEmpty()) {
      ks->printing.pageSize = printer.option("kde-pagesize");
    }
    if (!printer.option("kde-orientation").isEmpty()) {
      ks->printing.orientation = printer.option("kde-orientation");
    }
    if (printer.option("kst-plot-datetime-footer").isEmpty()) {
      datetimeFooter = ks->printing.plotDateTimeFooter == "1";
    } else {
      ks->printing.plotDateTimeFooter = printer.option("kst-plot-datetime-footer");
      datetimeFooter = printer.option("kst-plot-datetime-footer") == "1";
    }
    if (printer.option("kst-plot-maintain-aspect-ratio").isEmpty()) {
      maintainAspectRatio = ks->printing.maintainAspect == "1";
    } else {
      ks->printing.maintainAspect = printer.option("kst-plot-maintain-aspect-ratio");
      maintainAspectRatio = printer.option("kst-plot-maintain-aspect-ratio") == "1";
    }
    if (printer.option("kst-plot-curve-width-adjust").isEmpty()) {
      lineAdjust = ks->printing.curveWidthAdjust.toInt();
    } else {
      ks->printing.curveWidthAdjust = printer.option("kst-plot-curve-width-adjust");
      lineAdjust = printer.option("kst-plot-curve-width-adjust").toInt();
    }
    if (printer.option("kst-plot-monochrome").isEmpty()) {
      monochrome = ks->printing.monochrome == "1";
    } else {
      ks->printing.monochrome = printer.option("kst-plot-monochrome");
      monochrome = printer.option("kst-plot-monochrome") == "1";
    }
    // save additional monochrome settings
    if (printer.option("kst-plot-monochromesettings-enhancereadability").isEmpty()) {
      enhanceReadability = ks->printing.monochromeSettings.enhanceReadability == "1";
    } else {
      ks->printing.monochromeSettings.enhanceReadability = printer.option("kst-plot-monochromesettings-enhancereadability");
      enhanceReadability = printer.option("kst-plot-monochromesettings-enhancereadability") == "1";
    }
    if (printer.option("kst-plot-monochromesettings-pointstyleorder").isEmpty()) {
      pointStyleOrder = ks->printing.monochromeSettings.pointStyleOrder.toInt();
    } else {
      ks->printing.monochromeSettings.pointStyleOrder = printer.option("kst-plot-monochromesettings-pointstyleorder");
      pointStyleOrder = printer.option("kst-plot-monochromesettings-pointstyleorder").toInt();
    }
    if (printer.option("kst-plot-monochromesettings-linestyleorder").isEmpty()) {
      lineStyleOrder = ks->printing.monochromeSettings.lineStyleOrder.toInt();
    } else {
      ks->printing.monochromeSettings.lineStyleOrder = printer.option("kst-plot-monochromesettings-linestyleorder");
      lineStyleOrder = printer.option("kst-plot-monochromesettings-linestyleorder").toInt();
    }
    if (printer.option("kst-plot-monochromesettings-linewidthorder").isEmpty()) {
      lineWidthOrder = ks->printing.monochromeSettings.lineWidthOrder.toInt();
    } else {
      ks->printing.monochromeSettings.lineWidthOrder = printer.option("kst-plot-monochromesettings-linewidthorder");
      lineWidthOrder = printer.option("kst-plot-monochromesettings-linewidthorder").toInt();
    }
    if (printer.option("kst-plot-monochromesettings-maxlinewidth").isEmpty()) {
      maxLineWidth = ks->printing.monochromeSettings.maxLineWidth.toInt();
    } else {
      ks->printing.monochromeSettings.maxLineWidth = printer.option("kst-plot-monochromesettings-maxlinewidth");
      maxLineWidth = printer.option("kst-plot-monochromesettings-maxlinewidth").toInt();
    }
    if (printer.option("kst-plot-monochromesettings-pointdensity").isEmpty()) {
      pointDensity = ks->printing.monochromeSettings.pointDensity.toInt();
    } else {
      ks->printing.monochromeSettings.pointDensity = printer.option("kst-plot-monochromesettings-pointdensity");
      pointDensity = printer.option("kst-plot-monochromesettings-pointdensity").toInt();
    }
  
    ks->save();
  
    QList<int> pageList = printer.pageList();
    bool firstPage = true;

    it = createIterator();
    if (!it) {
      return;
    }

    while (it->currentItem()) {
      KstViewWindow *win;
      KstTopLevelViewPtr tlv;

      win = dynamic_cast<KstViewWindow*>(it->currentItem());
      tvl = win ? kst_cast<KstTopLevelView>(win->view()) : 0L;
      if (win && tlv && !tlv->children().isEmpty()) {
        ++pages;
        if (pageList.contains(pages)) {
          if (!firstPage && !printer.newPage()) {
            break;
          }
  
          win->print(paint, size, pages, lineAdjust, monochrome, enhanceReadability, datetimeFooter,  maintainAspectRatio, pointStyleOrder, lineStyleOrder, lineWidthOrder, maxLineWidth, pointDensity);
  
          firstPage = false;
        }
      }
      it->next();
    }
    paint.end();
    deleteIterator(it);
    slotUpdateStatusMsg(QObject::tr("Ready"));
  } else {
    slotUpdateStatusMsg(QObject::tr("Nothing to print"));
  }
*/
}


void KstApp::immediatePrintToFile(const QString& filename, bool revert) {
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;
  Kst2DPlotPtr rc;

  windows = subWindowList(QMdiArea::CreationOrder);
  if (windows.count() > 0) {
    QPrinter printer(QPrinter::HighResolution);

    printer.setPageSize(QPrinter::Letter);
    printer.setOrientation(QPrinter::Landscape);
    printer.setOutputFileName(filename);
    printer.setFromTo(0, 0);
  
    bool firstPage = true;
    KstPainter paint(KstPainter::P_PRINT);
    paint.begin(&printer);
// xxx    QPaintDeviceMetrics metrics(&printer);
    QRect rect;
    const QSize size;// xxx (metrics.width(), metrics.height());
  
    rect.setLeft(0);
    rect.setRight(size.height());
    rect.setBottom(size.height());
    rect.setTop(size.height());
  
    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow && viewWindow->view()->children().isEmpty()) {
        if (!firstPage && !printer.newPage()) {
          break;
        }
  
        viewWindow->view()->resizeForPrint(size);
        viewWindow->view()->paint(paint, QRegion());
        if (revert) {
          viewWindow->view()->revertForPrint();
        }
  
        firstPage = false;
      }
    }

    paint.end();
  }
}


void KstApp::immediatePrintToPng(const QString& filename, const QString& format, int width, int height, bool all, int display) {
  if (all) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
    QString dotFormat = QObject::tr(".%1").arg(format);
    QString filenameSub;
    QString filenameNew;
    int pages = 0;
    int iPos;

// xxx    iPos = filename.findRev(dotFormat, -1, false);
    if (iPos != -1 && iPos == (int)(filename.length() - dotFormat.length())) {
      filenameSub = filename.left(filename.length() - dotFormat.length());
    } else {
      filenameSub = filename;
    }
  
    windows = KstApp::inst()->subWindowList(QMdiArea::CreationOrder);
  
    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      pages++;
      filenameNew = QObject::tr("%1_%2").arg(filenameSub).arg(pages);
      immediatePrintWindowToPng(*i, filenameNew, format, width, height, display);
    }
  } else {
    immediatePrintActiveWindowToPng(filename, format, width, height, display);
  }
}

void KstApp::immediatePrintToEps(const QString& filename, int width, int height, bool all, int display) {
  if (all) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
    QString filenameSub;
    QString dotFormat = ".eps";
    QString filenameNew;
    int pages = 0;
    int iPos;

// xxx    iPos = filename.findRev(dotFormat, -1, false);
    if (iPos != -1 && iPos == (int)(filename.length() - dotFormat.length())) {
      filenameSub = filename.left(filename.length() - dotFormat.length());
    } else {
      filenameSub = filename;
    }

    windows = subWindowList(QMdiArea::CreationOrder);

    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      pages++;
      filenameNew = QObject::tr("%1_%2").arg(filenameSub).arg(pages);
      immediatePrintWindowToEps(*i, filenameNew, width, height, display);
    }
  } else {
    immediatePrintActiveWindowToEps(filename, width, height, display);
  }
}


void KstApp::slotFileQuit() {
  slotFileClose();
}


void KstApp::slotViewStatusBar() {
  if (_actionStatusBar->isChecked()) {
// xxx    statusBar()->show();
    updateStatusBarText();
  } else {
// xxx    statusBar()->hide();
  }
}


void KstApp::updateStatusBarText() {
/* xxx
  if (statusBar()->isVisible()) {
    QFontMetrics fm(fontMetrics());
    int widthUsed;
    int margin = 3;
    int spacing = 6;
    int widthCurrent;
    int widthAvailable;
    int widthData;
    int widthReady;

    widthCurrent = statusBar()->width();
    widthAvailable = widthCurrent - (2*margin) - spacing;
    widthData = fm.width(_dataBar->fullText());
    widthReady = fm.width(_readyBar->fullText());

    if (_progressBar->isVisible()) {
      widthAvailable -= _progressBar->width();
      widthAvailable -= spacing;
    }

    if (_dataNotifier->isVisible()) {
      widthAvailable -= _dataNotifier->geometry().width();
      widthAvailable -= spacing;
    }

    widthUsed  = widthData;
    widthUsed += widthReady;
#ifdef HAVE_LINUX
    widthUsed += fm.width(_memoryBar->fullText());
    widthAvailable -= spacing;
#endif

    if (widthUsed > widthAvailable) {
      if (widthData < widthAvailable) {
        statusBar()->setMaximumWidth(widthCurrent);

#ifdef HAVE_LINUX
        if ((widthAvailable - widthData)/2 > widthReady) {
          _memoryBar->setTextWidth(fm, widthAvailable - widthData - widthReady);
          _readyBar->setTextWidth(fm, widthReady);
        } else {
          _memoryBar->setTextWidth(fm, (widthAvailable - widthData)/2);
          _readyBar->setTextWidth(fm, (widthAvailable - widthData)/2);
        }
#else
        _readyBar->setTextWidth(fm, widthAvailable - widthData);
#endif
        _dataBar->setTextWidth(fm, widthData);

        statusBar()->setMaximumWidth(32767);
      } else {
#ifdef HAVE_LINUX
        _memoryBar->setTextWidth(fm, 0);
#endif
        _readyBar->setTextWidth(fm, 0);
        _dataBar->setTextWidth(fm, widthAvailable);
      }
    } else {
#ifdef HAVE_LINUX
      _memoryBar->setFullText();
#endif
      _readyBar->setFullText();
      _dataBar->setFullText();
    }
  }
*/
}


void KstApp::slotUpdateStatusMsg(const QString& msg) {
  _readyBar->setFullText( msg );
  updateStatusBarText();
}


void KstApp::slotUpdateDataMsg(const QString& msg) {
  _dataBar->setFullText( msg );
  updateStatusBarText();
}


void KstApp::slotUpdateMemoryMsg(const QString& msg) {
#ifdef HAVE_LINUX
  _memoryBar->setFullText( msg );
  updateStatusBarText();
#endif
}


void KstApp::slotUpdateProgress(int total, int step, const QString &msg) {
  if (step == 0 && msg.isNull()) {
    slotUpdateStatusMsg(QObject::tr("Ready"));
    _progressBar->hide();
    updateStatusBarText();
    
    return;
  }

  _progressBar->show();
  if (step > 0) {
    if (!_progressBar->isTextVisible()) {
      _progressBar->setTextVisible(true);
    }
    if (total != _progressBar->maximum()) {
      _progressBar->setMaximum(total);
    }
    if (_progressBar->value() != step) {
      _progressBar->setValue(step);
    }
  } else {
    _progressBar->setTextVisible(false);
    _progressBar->setValue(_progressBar->minimum());
  }

  if (msg.isEmpty()) {
    slotUpdateStatusMsg(QObject::tr("Ready"));
  } else {
    slotUpdateStatusMsg(msg);
  }

  updateStatusBarText();

// xxx  kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
}


bool KstApp::dataMode() const {
  return _actionDataMode->isChecked();
}


void KstApp::toggleDataMode() {
  KstTopLevelViewPtr pView;

  pView = activeView();
  if (pView) {
    pView->widget()->paint();
  }
  slotUpdateDataMsg(QString::null);
}


void KstApp::toggleMouseMode() {
  KstTopLevelView::ViewMode mode = KstTopLevelView::DisplayMode;
  QAction *action = 0L;
  QString createType;

  if (_actionGfxLine->isChecked()) {
    action = _actionGfxLine;
    mode = KstTopLevelView::CreateMode;
    createType = "Line";
  } else if (_actionGfxRectangle->isChecked()) {
    action = _actionGfxRectangle;
    mode = KstTopLevelView::CreateMode;
    createType = "Box";
  } else if (_actionGfxEllipse->isChecked()) {
    action = _actionGfxEllipse;
    mode = KstTopLevelView::CreateMode;
    createType = "Ellipse";
  } else if (_actionGfxLabel->isChecked()) {
    action = _actionGfxLabel;
    mode = KstTopLevelView::LabelMode;
    createType = "Label";
  } else if (_actionGfxPicture->isChecked()) {
    action = _actionGfxPicture;
    mode = KstTopLevelView::CreateMode;
    createType = "Picture";
  } else if (_actionGfx2DPlot->isChecked()) {
    action = _actionGfx2DPlot;
    mode = KstTopLevelView::CreateMode;
    createType = "Plot";
  } else if (_actionGfxArrow->isChecked()) {
    action = _actionGfxArrow;
    mode = KstTopLevelView::CreateMode;
    createType = "Arrow";
  } else if (_actionGfxLegend->isChecked()) {
    action = _actionGfxLegend;
    mode = KstTopLevelView::CreateMode;
    createType = "Legend";
  } else if (_actionLayout->isChecked()) {
    action = _actionLayout;
    mode = KstTopLevelView::LayoutMode;
  } else if (_actionZoomXY->isChecked()) {
    action = _actionZoomXY;
    mode = KstTopLevelView::DisplayMode;
  } else if (_actionZoomX->isChecked()) {
    action = _actionZoomX;
    mode = KstTopLevelView::DisplayMode;
  } else if (_actionZoomY->isChecked()) {
    action = _actionZoomY;
    mode = KstTopLevelView::DisplayMode;
  }

  if (_actionZoomX->isChecked() ||
      _actionZoomY->isChecked() ||
      _actionZoomXY->isChecked()) {
    _actionDataMode->setEnabled(true);
  } else {
    _actionDataMode->setEnabled(false);
  }

  if (action) {
/* xxx
    KToolBarButton* button = toolBar()->getButton(MODE_BUTTON_ID);

    if (button) {
      button->setText(action->toolTip());
      button->setIcon(action->icon());
    }
*/
  }

  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = subWindowList(QMdiArea::CreationOrder);
  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      viewWindow->view()->setViewMode(mode, createType);    
    }
  }

  _viewMode = mode;
  _createType = createType;
}


KstTopLevelView::ViewMode KstApp::currentViewMode() {
  return _viewMode;
}


QString KstApp::currentCreateType() {
  return _createType;
}


KstApp::KstGraphicType KstApp::getGraphicType() {
  return _graphicType;
}


void KstApp::tieAll() {
  int tied = 0;
  Kst2DPlotList pl = Kst2DPlot::globalPlotList();
  for (Kst2DPlotList::ConstIterator i = pl.begin(); i != pl.end(); ++i) {
    if ((*i)->isTied()) {
      ++tied;
    } else {
      --tied;
    }
  }

  for (Kst2DPlotList::Iterator i = pl.begin(); i != pl.end(); ++i) {
    (*i)->setTied(tied <= 0);
  }

  paintAll(KstPainter::P_PAINT);
}


void KstApp::paintAll(KstPainter::PaintType pt) {
  KstViewWindow *viewWindow;

  viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
  if (viewWindow) {
    viewWindow->view()->paint(pt);
  }
}


void KstApp::paintAllFromScript() {
  if (_updatesFromScriptEnabled) {
    paintAll(KstPainter::P_PAINT);
  }
}


void KstApp::setEnableImplicitRepaintsFromScript(bool enable) {
  _updatesFromScriptEnabled = enable;
  if (enable) {
    paintAll(KstPainter::P_PAINT);
  }
}


bool KstApp::getEnableImplicitRepaintsFromScript() {
  return _updatesFromScriptEnabled;
}


void KstApp::newPlot() {
  KstViewWindow *viewWindow;

  viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
  if (!viewWindow) {
    newWindow(false);
    viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
  }

  if (viewWindow) {
    viewWindow->createPlot(KST::suggestPlotName(), false);
    updateDialogsForPlot();
  }
}


void KstApp::showDataManager() {
// xxx  _dataManager->show_I();
}


void KstApp::showViewManager() {
// xxx  _viewManager->show_I();
}


void KstApp::showViewScalarsDialog() {
  _viewScalarsDialog->showViewScalarsDialog();
}


void KstApp::showViewStringsDialog() {
  _viewStringsDialog->showViewStringsDialog();
}


void KstApp::showViewVectorsDialog() {
  _viewVectorsDialog->showViewVectorsDialog();
}


void KstApp::showViewVectorsDialog(const QString &vector) {
  _viewVectorsDialog->showViewVectorsDialog(vector);
}


void KstApp::showViewMatricesDialog() {
  _viewMatricesDialog->showViewMatricesDialog();
}


void KstApp::showViewMatricesDialog(const QString &matrix) {
  _viewMatricesDialog->showViewMatricesDialog(matrix);
}


void KstApp::showViewFitsDialog() {
  _viewFitsDialog->showViewFitsDialog();
}


void KstApp::showChangeFileDialog() {
  _changeFileDialog->showChangeFileDialog();
}


void KstApp::showChooseColorDialog() {
  _chooseColorDialog->showChooseColorDialog();
}


void KstApp::showDifferentiateCurvesDialog() {
  _differentiateCurvesDialog->showCurveDifferentiate();
}


void KstApp::showChangeNptsDialog() {
  _changeNptsDialog->showChangeNptsDialog();
}


void KstApp::showGraphFileDialog() {
  _graphFileDialog->show_I();
}


void KstApp::showDebugDialog() {
  _debugDialog->show_I();
}


void KstApp::showDebugLog() {
  _debugDialog->show_I();
  _debugDialog->_tabs->setCurrentIndex(1);
// xxx  _debugDialog->logWidget()->scrollToBottom();
}


void KstApp::showMonochromeDialog() {
  _monochromeDialog->showMonochromeDialog();
}


void KstApp::samplesUp() {
  setPaused(false);
  _doc->samplesUp();
}


void KstApp::samplesDown() {
  setPaused(false);
  _doc->samplesDown();
}


void KstApp::updateDataNotifier() {
  if (_dataNotifier) {
    _dataNotifier->arrived();
  }
}


void KstApp::updateDataDialogs(bool dm, bool vm) {
  _actionViewScalarsDialog->setEnabled(_viewScalarsDialog->hasContent());
  _actionViewStringsDialog->setEnabled(_viewStringsDialog->hasContent());
  _actionViewVectorsDialog->setEnabled(_viewVectorsDialog->hasContent());
  _actionViewMatricesDialog->setEnabled(_viewMatricesDialog->hasContent());
  _actionViewFitsDialog->setEnabled(_viewFitsDialog->hasContent());

  if (!_viewScalarsDialog->isHidden()) {
    _viewScalarsDialog->updateViewScalarsDialog();
  }
  if (!_viewStringsDialog->isHidden()) {
    _viewStringsDialog->updateViewStringsDialog();
  }
  if (!_viewVectorsDialog->isHidden()) {
    _viewVectorsDialog->updateViewVectorsDialog();
  }
  if (!_viewMatricesDialog->isHidden()) {
    _viewMatricesDialog->updateViewMatricesDialog();
  }
  if (!_viewFitsDialog->isHidden()) {
    _viewFitsDialog->updateViewFitsDialog();
  }

  if (dm) {
// xxx    _dataManager->updateContents();
  }
  if (vm) {
// xxx    _viewManager->updateContents();
  }

  updateMemoryStatus();
}


void KstApp::updateVisibleDialogs() {
  updateDialogs(true);
}


void KstApp::updateDialogs(bool onlyVisible) {
  if (!_stopping) {
    if (!onlyVisible || KstVectorDialog::globalInstance()->isVisible()) {
      KstVectorDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstBasicDialog::globalInstance()->isVisible()) {
      KstBasicDialog::globalInstance()->updateForm();
    }
    if (!onlyVisible || KstPluginDialog::globalInstance()->isVisible()) {
      KstPluginDialog::globalInstance()->updateForm();
    }
    if (!onlyVisible || KstFitDialog::globalInstance()->isVisible()) {
      KstFitDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstFilterDialog::globalInstance()->isVisible()) {
      KstFilterDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstEqDialog::globalInstance()->isVisible()) {
      KstEqDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstHsDialog::globalInstance()->isVisible()) {
      KstHsDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstVvDialog::globalInstance()->isVisible()) {
      KstVvDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstPsdDialog::globalInstance()->isVisible()) {
      KstPsdDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstCsdDialog::globalInstance()->isVisible()) {
      KstCsdDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstCurveDialog::globalInstance()->isVisible()) {
      KstCurveDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstEventMonitor::globalInstance()->isVisible()) {
      KstEventMonitor::globalInstance()->update();
    }
    if (!onlyVisible || KstImageDialog::globalInstance()->isVisible()) {
      KstImageDialog::globalInstance()->update();
    }
    if (!onlyVisible || KstMatrixDialog::globalInstance()->isVisible()) {
      KstMatrixDialog::globalInstance()->update();
    }
/* xxx
    if (!onlyVisible || changeFileDialog->isVisible()) {
      _changeFileDialog->updateChangeFileDialog();
    }
    if (!onlyVisible || chooseColorDialog->isVisible()) {
      _chooseColorDialog->updateChooseColorDialog();
    }
    if (!onlyVisible || differentiateCurvesDialog->isVisible()) {
      _differentiateCurvesDialog->updateCurveDifferentiate();
    }
    if (!onlyVisible || changeNptsDialog->isVisible()) {
      _changeNptsDialog->updateChangeNptsDialog();
    }
    if (!onlyVisible || vectorSaveDialog->isVisible()) {
      _vectorSaveDialog->init();
    }
*/
    updateDataDialogs(false);
    updateDataManager(onlyVisible);
    updateViewManager(onlyVisible);
  }
}


void KstApp::updateDialogsForWindow() {
  if (!_stopping) {
    KstCsdDialog::globalInstance()->updateWindow();
    KstEqDialog::globalInstance()->updateWindow();
    KstHsDialog::globalInstance()->updateWindow();
    KstVvDialog::globalInstance()->updateWindow();
    KstPsdDialog::globalInstance()->updateWindow();
    KstCurveDialog::globalInstance()->updateWindow();
    KstImageDialog::globalInstance()->updateWindow();
    updateDataManager(false);
    updateViewManager(false);
  }
}


void KstApp::updateDialogsForPlot() {
  if (!_stopping) {
    KstCsdDialog::globalInstance()->updateWindow();
    KstEqDialog::globalInstance()->updateWindow();
    KstHsDialog::globalInstance()->updateWindow();
    KstVvDialog::globalInstance()->updateWindow();
    KstPsdDialog::globalInstance()->updateWindow();
    KstCurveDialog::globalInstance()->updateWindow();
    KstImageDialog::globalInstance()->updateWindow();
    updateDataManager(false);
    updateViewManager(false);
  }
}


void KstApp::updateDataManager(bool onlyVisible) {
/* xxx
  if (!onlyVisible || _dataManager->isShown()) {
    _dataManager->update();
  }
*/
}


void KstApp::updateViewManager(bool onlyVisible) {
/* xxx
  if (!onlyVisible || _viewManager->isShown()) {
    _viewManager->update();
  }
*/
}


void KstApp::showPluginManager() {
  KstPluginManager *pm;

  pm = new KstPluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;

  KstPluginDialog::globalInstance()->updatePluginList();
}


void KstApp::showExtensionManager() {
/* xxx
  ExtensionDialog *dlg;

  dlg = new ExtensionDialog(this, "Extension Manager");
  dlg->exec();
  delete dlg;
*/
}


void KstApp::showDataWizard() {
/* xxx
  KstDataWizard *dw;

  dw = new KstDataWizard(this, "DataWizard");
  dw->exec();
  if (dw->result() == QDialog::Accepted) {
    delete dw; // leave this here - releases references
    forceUpdate();
    doc->setModified();
    updateDialogs();
  } else {
    delete dw;
  }
*/
}


void KstApp::showDataWizardWithFile(const QString &input) {
/* xxx
  KstDataWizard *dw;

  dw = new KstDataWizard(this, "DataWizard");
  dw->setInput(input);
  dw->exec();
  if (dw->result() == QDialog::Accepted) {
    delete dw; // leave this here - releases references
    forceUpdate();
    _doc->setModified();
    updateDialogs();
  } else {
    delete dw;
  }
*/
}


void KstApp::registerDocChange() {
  forceUpdate();
  updateVisibleDialogs();
  _doc->setModified();
}


void KstApp::reload() {
  KstVectorList::const_iterator itvl;
  KstMatrixList::const_iterator itml;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QMap<KstDataSourcePtr, KstDataSourcePtr> dataSourcesReset;
  QMap<KstDataSourcePtr, KstDataSourcePtr> dataSourcesReloaded;

  KST::vectorList.lock().readLock();
  for (itvl = KST::vectorList.begin(); itvl != KST::vectorList.end(); ++itvl) {
    KstRVectorPtr r;

    r = kst_cast<KstRVector>(*itvl);
    if (r) {
      KstDataSourcePtr dataSrc;

      r->writeLock();

      dataSrc = r->dataSource();
      if (dataSrc) {
        if (dataSourcesReset.contains(dataSrc)) {
          //
          // if we've already reset the datasource held by this vector then
          //  we only need to reset the vector itself...
          //

          r->reset();
        } else {
          dataSrc->writeLock();

          if (dataSrc->reset()) {
            //
            // if we were successful in resetting the datasource held by this vector then
            //  we remember it and reset the vector itself...
            //

            dataSourcesReset.insert(dataSrc, dataSrc);
            r->reset();
          } else {
            //
            // we have to try and reload the datasource...
            //

            QMap<KstDataSourcePtr, KstDataSourcePtr>::Iterator itsrc;

            itsrc = dataSourcesReloaded.find(dataSrc);
            if (itsrc == dataSourcesReloaded.end()) {
              KstDataSourcePtr newsrc;

              //
              // if we haven't yet reloaded this datasource then do so now...
              //

              newsrc = KstDataSource::loadSource(dataSrc->fileName(), dataSrc->fileType());
              if (newsrc) {
                newsrc->writeLock();

                KST::dataSourceList.lock().writeLock();
                KST::dataSourceList.removeOne(dataSrc);
                KST::dataSourceList.append(newsrc);
                KST::dataSourceList.lock().unlock();

                dataSourcesReloaded.insert(dataSrc, newsrc);

                r->resetFile(newsrc);
                r->reset();

                newsrc->unlock();
              }
            } else {
              //
              // we have already reloaded this datasource, so just set it...
              //

              (*itsrc)->writeLock();
              r->resetFile(*itsrc);
              r->reset();
              (*itsrc)->unlock();
            }
          }

          dataSrc->unlock();
        }
      }

      r->unlock();
    }
  }
  KST::vectorList.lock().unlock();

  KST::matrixList.lock().readLock();
  for (itml = KST::matrixList.begin(); itml != KST::matrixList.end(); ++itml) {
    KstRMatrixPtr r;

    r = kst_cast<KstRMatrix>(*itml);
    if (r) {
      KstDataSourcePtr dataSrc;

      r->writeLock();

      dataSrc = r->dataSource();
      if (dataSrc) {
        if (dataSourcesReset.contains(dataSrc)) {
          //
          // if we've already reset the datasource held by this matrix then
          //  we only need to reset the matrix itself...
          //

          r->reset();
        } else {
          dataSrc->writeLock();

          if (dataSrc->reset()) {
            //
            // if we were successful in resetting the datasource held by this matrix then
            //  we remember it and reset the matrix itself...
            //

            dataSourcesReset.insert(dataSrc, dataSrc);
            r->reset();
          } else {
            //
            // we have to try and reload the datasource...
            //

            QMap<KstDataSourcePtr, KstDataSourcePtr>::Iterator itsrc;

            itsrc = dataSourcesReloaded.find(dataSrc);
            if (itsrc == dataSourcesReloaded.end()) {
              KstDataSourcePtr newsrc;

              //
              // if we haven't yet reloaded this datasource then do so now...
              //

              newsrc = KstDataSource::loadSource(dataSrc->fileName(), dataSrc->fileType());
              if (newsrc) {
                newsrc->writeLock();

                KST::dataSourceList.lock().writeLock();
                KST::dataSourceList.removeOne(dataSrc);
                KST::dataSourceList.append(newsrc);
                KST::dataSourceList.lock().unlock();

                dataSourcesReloaded.insert(dataSrc, newsrc);

                r->resetFile(newsrc);
                r->reset();

                newsrc->unlock();
              }
            } else {
              //
              // we have already reloaded this datasource, so just set it...
              //

              (*itsrc)->writeLock();
              r->resetFile(*itsrc);
              r->reset();
              (*itsrc)->unlock();
            }
          }

          dataSrc->unlock();
        }
      }

      r->unlock();
    }
  }
  KST::matrixList.lock().unlock();

  QApplication::restoreOverrideCursor();
}


void KstApp::slotPreferences() {
  KstSettingsDlg *ksd;

  ksd = new KstSettingsDlg(this, "Kst Settings Dialog");
  connect(ksd, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
  ksd->exec();

  delete ksd;
}


void KstApp::slotSettingsChanged() {
  _updateThread->setUpdateTime(KstSettings::globalSettings()->plotUpdateTimer);
}


void KstApp::slotCopy() {
  if (!_actionLayout->isChecked()) {
    KstTopLevelViewPtr tlv;

    tlv = activeView();
    if (tlv) {
      KstViewWidget *w;
      KstViewObjectPtr o;
      Kst2DPlotPtr p;

      w = tlv->widget();
      o = w->findChildFor(w->mapFromGlobal(QCursor::pos()));
      p = kst_cast<Kst2DPlot>(o);
      if (p) {
        p->copy();
      }
    }
  } else {
    KstViewWindow *vw;

    vw = dynamic_cast<KstViewWindow*>(activeSubWindow());
    if (vw) {
/* xxx
      QApplication::clipboard()->setData(vw->view()->widget()->dragObject(), QClipboard::Clipboard);
*/
    }
  }
}


void KstApp::slotPaste() {
  if (_actionLayout->isChecked()) {
    KstTopLevelViewPtr tlv;

    tlv = activeView();
    if (tlv) {
      QMimeSource* source;

// xxx      source = QApplication::clipboard()->data(QClipboard::Clipboard);
      if (!tlv->paste(source)) {
        KstDebug::self()->log(QObject::tr("Paste operation failed: clipboard data was not found or of the wrong format."));
      }
    } else {
      KstDebug::self()->log(QObject::tr("Paste operation failed: there is currently no active view."));
    }
  } else {
    KstDebug::self()->log(QObject::tr("Paste operation failed: must be in layout mode."));
  }
}


void KstApp::slotFileNewWindow(QWidget *parent) {
  newWindow(true, parent);
  _doc->setModified();
}


void KstApp::slotFileRenameWindow() {
  KstViewWindow *vw;

  vw = dynamic_cast<KstViewWindow*>(activeSubWindow());
  if (vw) {
    renameWindow(vw);
  }
}


void KstApp::renameWindow(KstViewWindow *vw) {
  QString name = windowName(true, vw->windowTitle(), true);

  if (!name.isEmpty() && vw->windowTitle() != name) {
    vw->setWindowTitle(name);
// xxx    vw->setTabCaption(name);
    updateDialogsForWindow();
    _doc->setModified();
  }
}


QString KstApp::newWindow(bool prompt, QWidget *parent) {
  QString nameUsed;
  QString name = windowName(prompt, defaultTag, false, parent);

  if (!name.isEmpty()) {
    nameUsed = newWindow(name);
  }

  return nameUsed;
}


KstViewWindow* KstApp::findWindow(const QString& title) {
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;
  KstViewWindow *viewWindowFound = 0L;

  windows = subWindowList(QMdiArea::CreationOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow;
  
    viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      if (title == viewWindow->windowTitle()) {
        viewWindowFound = viewWindow;
      }
    }
  }

  return viewWindowFound;
}


QString KstApp::newWindow(const QString& name_in) {
  KstViewWindow *viewWindow = new KstViewWindow;
  QString nameToUse;
  QString name  = name_in;

  while (name.isEmpty() || findWindow(name)) {
    name = KST::suggestWinName();
  }
  nameToUse = name;

  viewWindow->setWindowTitle(nameToUse);
// xxx  viewWindow->setTabCaption(nameToUse);
  addSubWindow(viewWindow, Qt::SubWindow);
  viewWindow->activateWindow();
  updateDialogsForWindow();
  
  return nameToUse;
}


QString KstApp::windowName(bool prompt, const QString& nameOriginal, bool rename, QWidget *parent) {
  QString name = nameOriginal;
  bool ok = false;

  do {
    if (prompt) {
/* xxx
      QRegExp exp("\\S+.*");
      QRegExpValidator val(exp, 0L);
*/
      if (rename) {
        name = QInputDialog::getText(parent, QObject::tr("Kst"), QObject::tr("Enter a new name for the tab:"), QLineEdit::Normal, name, &ok, 0L).trimmed();
      } else {
        name = QInputDialog::getText(parent, QObject::tr("Kst"), QObject::tr("Enter a name for the new tab:"), QLineEdit::Normal, name, &ok, 0L).trimmed();
      }
      if (ok && name==defaultTag) {
        name = KST::suggestWinName();
      }
    } else {
      name = KST::suggestWinName();
      ok = true;
    }
    if (ok) {
      if (!findWindow(name)) {
        break;
      }
    } else {
      return QString::null;
    }
    if (prompt) {
      QMessageBox::warning(this, QObject::tr("Kst"), QObject::tr("A window with the same name already exists.  Enter a unique window name."));
    }
  } while(true);

  return name;
}


void KstApp::tiedZoomPrev(KstViewWidget* view, const QString& plotName) {
  if (KstSettings::globalSettings()->tiedZoomGlobal) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
  
    windows = subWindowList(QMdiArea::CreationOrder);
  
    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow) {
       if (viewWindow->view()->tiedZoomPrev(plotName)) {
          viewWindow->view()->widget()->paint();
        }
      }
    }
  } else {
    view->viewObject()->tiedZoomPrev(plotName);
  }
}


void KstApp::tiedZoomMode(int zoom, bool flag, double center, int mode, int modeExtra, KstViewWidget* view, const QString& plotName) {
  if (KstSettings::globalSettings()->tiedZoomGlobal) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
  
    windows = subWindowList(QMdiArea::CreationOrder);
  
    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow) {
        if (viewWindow->view()->tiedZoomMode(zoom, flag, center, mode, modeExtra, plotName)) {
          viewWindow->view()->widget()->paint();
        }
      }
    }
  } else {
    view->viewObject()->tiedZoomMode(zoom, flag, center, mode, modeExtra, plotName);    
  }
}


void KstApp::tiedZoom(bool x, double xmin, double xmax, bool y, double ymin, double ymax, KstViewWidget* view, const QString& plotName) {
  if (KstSettings::globalSettings()->tiedZoomGlobal) {
    QList<QMdiSubWindow*> windows;
    QList<QMdiSubWindow*>::const_iterator i;
  
    windows = subWindowList(QMdiArea::CreationOrder);
  
    for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(*i);
      if (viewWindow) {
        if (viewWindow->view()->tiedZoom(x, xmin, xmax, y, ymin, ymax, plotName)) {
          viewWindow->view()->widget()->paint();
        }
      }
    }
  } else {
    view->viewObject()->tiedZoom(x, xmin, xmax, y, ymin, ymax, plotName);    
  }
}


KstTopLevelViewPtr KstApp::activeView() {
  KstTopLevelViewPtr tlv;
  KstViewWindow *vw = dynamic_cast<KstViewWindow*>(activeSubWindow());

  if (vw) {
    tlv = vw->view();
  }

  return tlv;
}


Kst2DPlotMap* KstApp::plotHolderWhileOpeningDocument() {
  return _plotHolderWhileOpeningDocument;
}


void KstApp::updatePausedState(bool state) {
  _updateThread->setPaused(state);
}


void KstApp::fromEnd() {
  _doc->samplesEnd();
  setPaused(false);
}


void KstApp::updateMemoryStatus() {
#ifdef HAVE_LINUX
  unsigned long mi;

  if (KST::memfree(mi, false)) {
    QString memoryAvailable;

    mi /= 1024;
    if (mi < 1024) {
      memoryAvailable = QObject::tr("abbreviation for kilobytes", "%1 kB").arg(mi);
    } else {
      mi /= 1024;
      if (mi < 1024) {
        memoryAvailable = QObject::tr("abbreviation for megabytes", "%1 MB").arg(mi);
      } else {
        mi /= 1024;
        if (mi < 1024) {
          memoryAvailable = QObject::tr("abbreviation for gigabytes", "%1 GB").arg(mi);
        } else {
          mi /= 1024;
          memoryAvailable = QObject::tr("abbreviation for terabytes", "%1 TB").arg(mi);
        }
      }
    }

    slotUpdateMemoryMsg(QObject::tr("%1 available").arg(memoryAvailable));
  }
#endif
}


const QStringList KstApp::recentFiles() const {
// xxx  return _recent->items();
}


void KstApp::showQuickStartDialog() {
  if (KstSettings::globalSettings()->showQuickStart) {
    _quickStartDialog->show_I();
  }
}


void KstApp::createDebugNotifier() {
/* xxx
  if (!_debugNotifier) {
    _debugNotifier = new KstDebugNotifier(statusBar());
    statusBar()->addWidget(_debugNotifier, 0, true);
  } else {
    _debugNotifier->reanimate();
  }
*/
}


void KstApp::destroyDebugNotifier() {
  delete _debugNotifier;
}


void KstApp::showContextMenu(QWidget *w, const QPoint& pos) {
  KstViewWindow *vw = dynamic_cast<KstViewWindow*>(w);
  QMenu *pm = new QMenu(this);
  QAction *action;

  if (vw) {
    pm->setTitle(vw->windowTitle());
  }

  pm->addAction(QObject::tr("&New..."), this, SLOT(slotFileNewWindow()));
  if (vw) {
/* xxx
    KTabWidget *tw = tabWidget();

    if (tw) { // should always be true, but who knows how KMdi might change
*/
      action = pm->addAction(QObject::tr("Move &Left"), vw, SLOT(moveTabLeft()));
// xxx      action->setEnabled(tw->indexOf(w) > 0);
      action = pm->addAction(QObject::tr("Move &Right"), vw, SLOT(moveTabRight()));
// xxx      action->setEnabled(tw->indexOf(w) < tw->count() - 1);
/* xxx
    }
*/
    pm->addAction(QObject::tr("R&ename..."), vw, SLOT(rename()));
    pm->addAction(QObject::tr("&Close"), vw, SLOT(close()));
  }

  pm->exec(pos);

  delete pm;
}


void KstApp::showContextMenu(const QPoint& pos) {
  QMenu *pm = new QMenu(this);

  pm->addAction(QObject::tr("&New..."), this, SLOT(slotFileNewWindow()));
  pm->exec(pos);

  delete pm;
}


void KstApp::moveTabLeft(KstViewWindow *tab) {
/* xxx
  KTabWidget *tw = tabWidget();

  if (tw) {
    int cur = tw->indexOf(tab);
    if (cur > 0) {
      tw->moveTab(cur, cur - 1);
    }
  }
*/
}


void KstApp::moveTabRight(KstViewWindow *tab) {
/* xxx
  KTabWidget *tw = tabWidget();

  if (tw) {
    int cur = tw->indexOf(tab);
    if (cur >= 0 && cur < tw->count() - 1) {
      tw->moveTab(cur, cur + 1);
    }
  }
*/
}


void KstApp::saveTabs(QTextStream& ts) {
/* xxx
  KTabWidget *tw = tabWidget();

  if (tw) {
    for (int tab=0; tab<tw->count(); ++tab) {
      KstViewWindow *v;

      v = dynamic_cast<KstViewWindow*>(tw->page(tab));
      if (v) {
        ts << "  <window>" << endl;
        v->save(ts, "    ");
        ts << "  </window>" << endl;
      }
    }
  }
*/
}


void KstApp::emitTimezoneChanged(const QString& tz, int utcOffset) {
  emit timezoneChanged(tz, utcOffset);
}

#include "kst.moc"

