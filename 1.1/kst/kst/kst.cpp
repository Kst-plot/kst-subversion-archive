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

// include files for Qt
#include <qclipboard.h>
#include <qdeepcopy.h>
#include <qeventloop.h>
#include <qpaintdevicemetrics.h>
#include <qprogressbar.h>
#include <qvalidator.h>

// include files for KDE
#include <kaccel.h>
#include "ksdebug.h"
#include <kfiledialog.h>
#include <kkeydialog.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktabwidget.h>
#include <qdatetime.h>

// application specific includes
#include "datawizard.h"
#include "extensiondlg.h"
#include "extensionmgr.h"
#include "kst.h"
#include "kstchangefiledialog_i.h"
#include "kstchangenptsdialog_i.h"
#include "kstcurvedialog_i.h"
#include "kstdatamanager_i.h"
#include "kstdebugdialog_i.h"
#include "kstdoc.h"
#include "ksteqdialog_i.h"
#include "kstevents.h"
#include "ksteventmonitor_i.h"
#include "kstfitdialog_i.h"
#include "kstfilterdialog_i.h"
#include "kstgraphfiledialog_i.h"
#include "ksthsdialog_i.h"
#include "kstiface_impl.h"
#include "kstimagedialog_i.h"
#include "kstlabeldialog_i.h"
#include "kstmatrixdialog_i.h"
#include "kstplotdialog_i.h"
#include "kstplugindialog_i.h"
#include "kstprintoptionspage.h"
#include "kstpsddialog_i.h"
#include "kstquickstartdialog_i.h"
#include "kstsettingsdlg.h"
#include "kstuinames.h"
#include "kstvectordefaults.h"
#include "kstvectordialog_i.h"
#include "kstviewscalarsdialog_i.h"
#include "kstviewvectorsdialog_i.h"
#include "kstviewmatricesdialog_i.h"
#include "kstviewfitsdialog_i.h"
#include "kstviewwindow.h"
#include "plotmimesource.h"
#include "pluginmanager.h"
#include "psversion.h"
#include "sysinfo.h"
#include "updatethread.h"
#include "vectorsavedialog.h"


#define KST_STATUSBAR_DATA   1
#define KST_STATUSBAR_STATUS 2

#define KST_QUICKSTART_DLG

static KstApp *inst = 0L;

const QString& KstApp::defaultTag = KGlobal::staticQString("<Auto Name>");

void KstApp::doubleCheckCleanup() {
  delete ::inst;
  ::inst = 0L;
}

KstApp* KstApp::inst() {
  return ::inst;
}

static KConfig *kConfigObject = 0L;

KstApp::KstApp(QWidget *parent, const char *name)
: KMdiMainFrm(parent, name) {
  assert(!::inst);
  ::inst = this;

  KGlobal::dirs()->addResourceType("kst", KStandardDirs::kde_default("data") + "kst");

  _dataSourceConfig = kConfigObject;

  clearWFlags(WDestructiveClose);

  _stopping = false;
  config = kapp->config();
  initStatusBar();
  setStandardToolBarMenuEnabled(true);

  initDocument();
  KstDebug::self()->setHandler(doc);
  setCaption(doc->title());

  /* create dialogs */
  debugDialog = new KstDebugDialogI(this);
  _plotDialog = new KstPlotDialogI(doc, this);
  dataManager = new KstDataManagerI(doc, this);
  viewScalarsDialog = new KstViewScalarsDialogI(this);
  viewVectorsDialog = new KstViewVectorsDialogI(this);
  viewMatricesDialog = new KstViewMatricesDialogI(this);
  viewFitsDialog = new KstViewFitsDialogI(this);
  changeFileDialog = new KstChangeFileDialogI(this);
  changeNptsDialog = new KstChangeNptsDialogI(this);
  graphFileDialog = new KstGraphFileDialogI(this);
  vectorSaveDialog = new VectorSaveDialog(this);
  _labelDialog = new KstLabelDialogI(this);
#ifdef KST_QUICKSTART_DLG
  _quickStartDialog = new KstQuickStartDialogI(this, 0 , true);
#endif

  connect(_labelDialog, SIGNAL(applied()), doc, SLOT(wasModified()));
  connect(KstVectorDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstCurveDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstEqDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstHsDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstPsdDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstPluginDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstFitDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstFilterDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstEventMonitorI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstImageDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstMatrixDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(this, SIGNAL(mdiModeHasBeenChangedTo(KMdi::MdiMode)), SLOT(fixKMdi()));

  initActions();
  readOptions();

  fixKMdi();

  _updateThread = new UpdateThread(doc);
  _updateThread->setUpdateTime(KstSettings::globalSettings()->plotUpdateTimer);
  _updateThread->start();

  /*** Plot Dialog signals */
  connect(changeFileDialog, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(changeNptsDialog, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(graphFileDialog, SIGNAL(graphFileReq(const QString&,const QString&,int,int,bool,int)), this, SLOT(immediatePrintToPng(const QString&,const QString&,int,int,bool,int)));

  /*** plot dialog ***/
  connect(_plotDialog, SIGNAL(docChanged()), this, SLOT(registerDocChange()));

  // data manager signals
  connect(dataManager, SIGNAL(docChanged()), this, SLOT(registerDocChange()));
  connect(doc, SIGNAL(updateDialogs()), this, SLOT(updateDialogs()));
  connect(doc, SIGNAL(dataChanged()), this, SLOT(updateDataDialogs()));

  connect(dataManager, SIGNAL(editDataVector(const QString&)),
          KstVectorDialogI::globalInstance(), SLOT(show_Edit(const QString&)));
  connect(dataManager, SIGNAL(editStaticVector(const QString&)),
          KstVectorDialogI::globalInstance(), SLOT(show_Edit(const QString&)));

  setAutoSaveSettings("KST-KMainWindow", true);
  _dcopIface = new KstIfaceImpl(doc, this);

  connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));

  QTimer::singleShot(0, this, SLOT(updateActions()));

  // load any extensions
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

  checkFontPresent("Symbol");
  checkFontPresent("Helvetica");

  QFont defaultFont; // QT's current default font
  QFontInfo info(defaultFont);
  _defaultFont = info.family();

  if (!isFakingSDIApplication()) {
    bool addedWindowMenu = false;
    int menuCount = menuBar()->count();
    for (int menuIndex = 0; menuIndex < menuCount; menuIndex++) {
      int menuId = menuBar()->idAt(menuIndex);
      if (menuId != -1) {
        const QString& menuTitle = menuBar()->text(menuId);
        if (menuTitle == i18n("&Help")) {
          menuBar()->insertItem(i18n("&Window"), windowMenu(), 100, menuIndex);
          addedWindowMenu = true;
          break;
        }
      }
    }

    if (!addedWindowMenu) {
      menuBar()->insertItem(i18n("&Window"), windowMenu());
    }
    connect(windowMenu(), SIGNAL(aboutToShow()), this, SLOT(addNewWindowMenu()));
  }
}


KstApp::~KstApp() {
  if (_updateThread) {
    _updateThread->setFinished(true);
    if (!_updateThread->wait(3000)) { // 3s
      _updateThread->terminate();
    }
  }

  KstDataSource::cleanupForExit(); // must be before deletions
  delete _updateThread;
  _updateThread = 0L;
  delete _dcopIface;
  _dcopIface = 0L;
  ::inst = 0L;
  if (_dataSourceConfig) {
    _dataSourceConfig->sync();
    _dataSourceConfig = 0L;
  }
  delete kConfigObject; // must be after cleanupForExit
  kConfigObject = 0L;
}


void KstApp::initialize() {
  KstSettings::checkUpdates();
  kConfigObject = new KConfig("kstdatarc", false, false);
  KstDataSource::setupOnStartup(kConfigObject);
  // NOTE: This is leaked in commandline mode if we never create a KstApp.
  //       Not too much of a problem right now, and less messy than hooking in
  //       cleanups in main.
}


void KstApp::addNewWindowMenu() {
  int id = windowMenu()->insertItem(i18n("&New..."), this, SLOT(slotFileNewWindow()), 0, -1, 0);
  windowMenu()->setWhatsThis(id, i18n("Create a new plot window."));

  id = windowMenu()->insertItem(i18n("&Rename..."), this, SLOT(slotFileRenameWindow()), 0, -1, 0);
  windowMenu()->setWhatsThis(id, i18n("Rename an existing plot window."));
  if (!activeView()) {
    windowMenu()->setItemEnabled(id, false);
  }
}


const QString& KstApp::defaultFont() const {
  return _defaultFont;
}


void KstApp::checkFontPresent(const QString& font) {
  QFont f(font);
  QFontInfo info(f);
  if (info.family().lower() != font.lower()) {
    QString msg = i18n("The %1 font was not found and was replaced by the %2 font; as a result, some labels may not display correctly.").arg(font).arg(info.family());
    KstDebug::self()->log(msg, KstDebug::Warning);
  }
}


void KstApp::EventELOGSubmitEntry(const QString& msg) {
  emit ELOGSubmitEntry(msg);
}


void KstApp::EventELOGConfigure() {
  emit ELOGConfigure();
}


void KstApp::customEvent(QCustomEvent *pEvent) {
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
}


void KstApp::initActions() {
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSave->setWhatsThis(i18n("Save to current Kst plot file."));

  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileSaveAs->setWhatsThis(i18n("Save to new Kst plot file."));

  fileClose = KStdAction::close(this, SLOT(slotFileNew()), actionCollection());
  fileClose->setWhatsThis(i18n("Close current Kst plot file."));

  fileQuit = KStdAction::quit(this, SLOT(slotFileClose()), actionCollection());
  fileQuit->setWhatsThis(i18n("Quit Kst."));

#if 0
  // KDE 3.3 only
  fileKeyBindings = KStdAction::keyBindings(guiFactory(), SLOT(slotConfigureKeys()), actionCollection());
#else
  fileKeyBindings = KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
#endif
  fileKeyBindings->setWhatsThis(i18n("Bring up a dialog box\n"
                                     "to configure shortcuts."));

  filePreferences = KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection());
  filePreferences->setWhatsThis(i18n("Bring up a dialog box\n"
                                     "to configure Kst settings."));

  fileCopy = KStdAction::copy(this, SLOT(slotCopy()), actionCollection());
  fileCopy->setWhatsThis(i18n("Copy cursor position or plots to the clipboard."));

  filePaste = KStdAction::paste(this, SLOT(slotPaste()), actionCollection());
  filePaste->setWhatsThis(i18n("Paste plots from the clipboard."));

  /************/
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()),
                                actionCollection());
  filePrint->setToolTip(i18n("Print"));
  filePrint->setWhatsThis(i18n("Print current display"));

  /************/
  StatusBarAction = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
                                              actionCollection());
  StatusBarAction->setWhatsThis(i18n("Toggle Statusbar"));
  connect(StatusBarAction, SIGNAL(activated()), this, SLOT(setSettingsDirty()));

  /************/
  KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());

  /************/
  recent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL &)),
                           actionCollection());
  recent->setWhatsThis(i18n("Open a recently used Kst plot."));

  /************/
  PauseAction = new KToggleAction(i18n("P&ause"),"player_pause",0,
                                  actionCollection(), "pause_action");
  PauseAction->setToolTip(i18n("Pause"));
  PauseAction->setWhatsThis(i18n("When paused, new data will not be read."));
  connect(PauseAction, SIGNAL(toggled(bool)), this, SLOT(updatePausedState(bool)));

  /************/
  XYZoomAction = new KRadioAction(i18n("XY Mouse &Zoom"), "kst_zoomxy",0,
                                  this, SLOT(toggleMouseMode()),
                                  actionCollection(), "zoomxy_action");
  XYZoomAction->setExclusiveGroup("zoom");
  XYZoomAction->setToolTip(i18n("XY mouse zoom"));
  XYZoomAction->setWhatsThis(i18n("XY zoom: mouse zooming affects\n"
                                  "both X and Y axis"));
  XYZoomAction->setChecked(true);

  /************/
  XZoomAction = new KRadioAction(i18n("&X Mouse Zoom"), "kst_zoomx",0,
                                 this, SLOT(toggleMouseMode()),
                                 actionCollection(), "zoomx_action");
  XZoomAction->setExclusiveGroup("zoom");
  XZoomAction->setToolTip(i18n("X mouse zoom"));
  XZoomAction->setWhatsThis(i18n("X zoom: Mouse zooming affects only the\n"
                                 "X axis (CTRL-mouse also does this)"));

  /************/
  YZoomAction = new KRadioAction(i18n("&Y Mouse Zoom"), "kst_zoomy",0,
                                  actionCollection(), "zoomy_action");
  YZoomAction->setExclusiveGroup("zoom");
  YZoomAction->setToolTip(i18n("Y mouse zoom"));
  YZoomAction->setWhatsThis(i18n("Y zoom: Mouse zooming affects only the\n"
                                 "Y axis (SHIFT-mouse also does this)"));

  /************/
  TextAction = new KRadioAction(i18n("&Label Mode"), "text",0,
                                this, SLOT(toggleMouseMode()),
                                actionCollection(), "label_action");

  TextAction->setExclusiveGroup("zoom");
  TextAction->setToolTip(i18n("Label Editor"));
  TextAction->setWhatsThis(i18n("Use the mouse to create and edit labels."));

  /************/
  LayoutAction = new KRadioAction(i18n("Layout Mode"), "kst_layoutmode", 0,
                                 this, SLOT(toggleMouseMode()),
                                 actionCollection(), "layoutmode_action");
  LayoutAction->setExclusiveGroup("zoom");
  LayoutAction->setToolTip(i18n("Layout mode"));
  LayoutAction->setWhatsThis(i18n("Use this mode to move, resize, and group plots."));

  /************/
  PlotDialogAction = new KAction(i18n("Edit &Plot..."), "kst_editplots", 0,
                                 this, SLOT(showPlotDialog()),
                                 actionCollection(), "plotdialog_action");
  PlotDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                      "to edit plot settings."));

  /************/
  DataManagerAction = new KAction(i18n("&Data Manager"), "kst_datamanager", 0,
                                  dataManager, SLOT(show_I()),
                                  actionCollection(), "datamanager_action");
  DataManagerAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                       "to manage data."));

  /************/
  VectorDialogAction = new KAction(i18n("New &Vector..."), "kst_vectornew", 0,
                                 KstVectorDialogI::globalInstance(),
                                 SLOT(show_New()), actionCollection(),
                                 "vectordialog_action");
  VectorDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to create a new vector."));

  /************/
  CurveDialogAction = new KAction(i18n("New &Curve..."), "kst_curvenew", 0,
                                  KstCurveDialogI::globalInstance(),
                                  SLOT(show_New()), actionCollection(),
                                  "curvedialog_action");
  CurveDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                       "to create a new curve."));

  /************/
  EqDialogAction = new KAction(i18n("New &Equation..."),
			       "kst_equationnew", 0,
                               KstEqDialogI::globalInstance(),
                               SLOT(show_New()),
                               actionCollection(), "eqdialog_action");
  EqDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                    "to create a new equation."));

  /************/
  HsDialogAction = new KAction(i18n("New &Histogram..."),
			       "kst_histogramnew", 0,
                               KstHsDialogI::globalInstance(),
                               SLOT(show_New()), actionCollection(),
                               "hsdialog_action");
  HsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                    "to create a new histogram."));

  /************/
  PsdDialogAction = new KAction(i18n("New Power &Spectrum..."),
				"kst_psdnew", 0,
                                KstPsdDialogI::globalInstance(),
                                SLOT(show_New()), actionCollection(),
                                "psddialog_action");
  PsdDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                     "to create a new power spectrum."));

  /************/
  PluginDialogAction = new KAction(i18n("New &Plugin..."),
				   "kst_pluginnew", 0,
                                   KstPluginDialogI::globalInstance(),
                                   SLOT(show_New()), actionCollection(),
                                   "plugindialog_action");
  PluginDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to create a new plugin instance."));

  /************/
  MatrixDialogAction = new KAction(i18n("New M&atrix..."), "kst_matrixnew", 0,
                                   KstMatrixDialogI::globalInstance(),
                                   SLOT(show_New()), actionCollection(),
                                   "matrixdialog_action");
  MatrixDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to create a new matrix."));

  /************/
  ImageDialogAction = new KAction(i18n("New &Image..."),
				  "kst_imagenew", 0,
                                   KstImageDialogI::globalInstance(),
                                   SLOT(show_New()), actionCollection(),
                                   "imagedialog_action");
  ImageDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to create a new image instance."));

  /************/
  ChangeFileDialogAction = new KAction(i18n("Change Data &File..."),
                                       "kst_changefile", 0, this,
                                       SLOT(showChangeFileDialog()),
                                       actionCollection(),
                                       "changefiledialog_action");
  ChangeFileDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to change input files."));

  /************/
  ViewScalarsDialogAction = new KAction(i18n("View &Scalar Values"),
                                       0, 0, this,
                                       SLOT(showViewScalarsDialog()),
                                       actionCollection(),
                                       "viewscalarsdialog_action");
  ViewScalarsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to view scalar values."));

  /************/
  ViewVectorsDialogAction = new KAction(i18n("View Vec&tor Values"),
                                       0, 0, this,
                                       SLOT(showViewVectorsDialog()),
                                       actionCollection(),
                                       "viewvectorsdialog_action");
  ViewVectorsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to view vector values."));

  /************/
  ViewMatricesDialogAction = new KAction(i18n("View &Matrix Values"),
                                       0, 0, this,
                                       SLOT(showViewMatricesDialog()),
                                       actionCollection(),
                                       "viewmatricesdialog_action");
  ViewMatricesDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to view matrix values."));

  /************/
  ViewFitsDialogAction = new KAction(i18n("View &Fit Results"),
                                       0, 0, this,
                                       SLOT(showViewFitsDialog()),
                                       actionCollection(),
                                       "viewfitsdialog_action");
  ViewFitsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to view fit values."));

  /************/
  ChangeNptsDialogAction = new KAction(i18n("Change Data Sample &Ranges..."),
                                       "kst_changenpts", 0, this,
                                       SLOT(showChangeNptsDialog()),
                                       actionCollection(),
                                       "changenptsdialog_action");
  ChangeNptsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to change data sample ranges."));

  /************/
  EventMonitorAction = new KAction(i18n("New Event &Monitor..."),
                                     "kst_eventnew", 0,
                                     KstEventMonitorI::globalInstance(),
                                     SLOT(show_New()),
                                     actionCollection(),
                                     "eventmonitor_action");
  EventMonitorAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                          "create a new event monitor."));

  /************/
  GraphFileDialogAction = new KAction(i18n("Export to Graphics File..."),
                                  "thumbnail", 0,
                                  this, SLOT(showGraphFileDialog()),
                                  actionCollection(),
                                  "graphfiledialog_action");
  GraphFileDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                           "to export the plot as a\n"
                                           "graphics file."));

  /************/
  _vectorSaveAction = new KAction(i18n("Save Vectors to Disk..."),
                                  0, 0,
                                  vectorSaveDialog, SLOT(show()),
                                  actionCollection(),
                                  "vectorsavedialog_action");
  _vectorSaveAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                       "to save vectors to text files."));

  /************/
  SamplesDownAction = new KAction(i18n("&Back 1 Screen"),
                                  "kst_back",
                                  KShortcut(CTRL + Key_Left),
                                  this, SLOT(samplesDown()),
                                  actionCollection(),
                                  "samplesdown_action");
  //SamplesDownAction->setToolTip(i18n("Back"));
  SamplesDownAction->setWhatsThis(i18n("Reduce the starting frame by\n"
                                       "the current number of frames."));

  /************/
  SamplesUpAction = new KAction(i18n("&Advance 1 Screen"),
                                "kst_advance",
                                KShortcut(CTRL + Key_Right),
                                this, SLOT(samplesUp()),
                                actionCollection(),
                                "samplesup_action");

  //SamplesUpAction->setToolTip(i18n("Advance"));
  SamplesUpAction->setWhatsThis(i18n("Increase the starting frame by\n"
                                     "the current number of frames."));

  /************/
  SamplesFromEndAction = new KAction(i18n("Read From &End"),
                                     "1rightarrow",
                                     KShortcut(SHIFT + CTRL + Key_Right),
                                     this, SLOT(fromEnd()),
                                     actionCollection(),
                                     "samplesend_action");
  SamplesFromEndAction->setToolTip(i18n("Read from end"));
  SamplesFromEndAction->setWhatsThis(i18n("Read current data from end of file."));

  /************/
  PluginManagerAction = new KAction(i18n("&Plugins..."), 0, 0,
                                 this, SLOT(showPluginManager()),
                                 actionCollection(), "pluginmanager_action");
  PluginManagerAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                           "to manage plugins."));


  /************/
  ExtensionManagerAction = new KAction(i18n("&Extensions..."), 0, 0,
                                 this, SLOT(showExtensionManager()),
                                 actionCollection(), "extensionmanager_action");
  ExtensionManagerAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                           "to manage extensions."));


  /************/
  DataWizardAction = new KAction(i18n("Data &Wizard"), "kst_datawizard", 0,
                                 this, SLOT(showDataWizard()),
                                 actionCollection(), "datawizard_action");
  DataWizardAction->setWhatsThis(i18n("Bring up a wizard\n"
                                           "to easily load data."));


  /************/
  DebugDialogAction = new KAction(i18n("Debug Kst..."), 0, 0,
                                 this, SLOT(showDebugDialog()),
                                 actionCollection(), "debug_action");
  DebugDialogAction->setWhatsThis(i18n("Bring up a dialog\n"
                                           "to display debugging information."));


  /************/
  DataMode = new KToggleAction(i18n("Data Mode"), "kst_datamode", 0,
                                 this, SLOT(toggleDataMode()),
                                 actionCollection(), "datamode_action");
  DataMode->setWhatsThis(i18n("Toggle between cursor mode and data mode."));
  DataMode->setToolTip(i18n("Data mode"));
  /************/
  _reloadAction = new KAction(i18n("Reload"), "reload", Key_F5, this, SLOT(reload()),
                              actionCollection(), "reload");
  _reloadAction->setWhatsThis(i18n("Reload the data from file."));

  _tiedZoomAction = new KAction(i18n("&Tied Zoom"),"kst_zoomtie",0,
                               this, SLOT(tieAll()),
                               actionCollection(), "zoomtie_action");
  _tiedZoomAction->setToolTip(i18n("Enable tied zoom"));
  _tiedZoomAction->setWhatsThis(i18n("Apply zoom actions to all plots\n"
                                     "(not just the active one)."));


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
  } else if (TextAction->isChecked()) {
    return TEXT;
  } else if (LayoutAction->isChecked()) {
    return LAYOUT;
  } else {
    return XYZOOM;
  }
}


void KstApp::initStatusBar() {
  _dataBar = new QLabel(QString::null, statusBar());
  statusBar()->addWidget(_dataBar, 5, true);

  _readyBar = new QLabel(i18n("Almost Ready"), statusBar());
  _readyBar->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusBar()->addWidget(_readyBar, 5, true);

  _progressBar = new KProgress(statusBar());
  _progressBar->setPercentageVisible(false);
  _progressBar->setCenterIndicator(true);
  statusBar()->addWidget(_progressBar, 2, true);
  _progressBar->setMaximumHeight( fontMetrics().height() );
  _progressBar->hide();

#ifdef HAVE_LINUX
  _memoryBar = new QLabel(i18n("0 MB available"), statusBar());
  statusBar()->addWidget(_memoryBar, 0, true);
  connect(&_memTimer, SIGNAL(timeout()), this, SLOT(updateMemoryStatus()));
  _memTimer.start(5000);
#endif

  statusBar()->show();
  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::initDocument() {
  doc = new KstDoc(this);
  doc->newDocument();
}

void KstApp::forceUpdate() {
  _updateThread->forceUpdate();
}


void KstApp::addRecentFile(const KURL& url) {
  recent->addURL(url);
}


void KstApp::selectRecentFile(const KURL &url) {
  QStringList urls;
  int count;
  int i;

  if (url.isEmpty()) {
    recent->setCurrentItem(-1);
  } else {
    urls = recent->items();
    count = urls.count();
    for (i=0; i<count; i++) {
      if (KURL(urls[i]) == url) {
        recent->setCurrentItem(i);
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

  url.setPath(in_filename);
  slotUpdateStatusMsg(i18n("Opening file..."));

  if (doc->openDocument(url, o_file, o_n, o_f, o_s, o_ave)) {
    setCaption(doc->title());
    if (url.isLocalFile()) {
      QFileInfo finfo(in_filename);
      addRecentFile(finfo.absFilePath());
    } else {
      addRecentFile(url);
    }
    rc = true;
  }
  slotUpdateStatusMsg(i18n("Ready"));
  opening = false;
  return rc;
}


KstPlotDialogI *KstApp::plotDialog() const {
  return _plotDialog;
}


KstDoc *KstApp::document() const {
  return doc;
}

void KstApp::saveOptions() {
  config->setGroup("General Options");
  recent->saveEntries(config, "Recent Files");

  config->writeEntry("MDIMode", (int)mdiMode());

  KST::vectorDefaults.writeConfig(config);

  config->sync();
}

void KstApp::readOptions() {
  config->setGroup("General Options");
  recent->loadEntries(config, "Recent Files");

  int mdiMode = config->readNumEntry("MDIMode", (int)KMdi::TabPageMode);

  KST::vectorDefaults.readConfig(config);

  switch (mdiMode) {
    case KMdi::ToplevelMode:
      switchToToplevelMode();
      break;
    case KMdi::ChildframeMode:
      switchToChildframeMode();
      break;
    case KMdi::TabPageMode:
      switchToTabPageMode();
      break;
    default:
      switchToTabPageMode();
      break;
  }
}

void KstApp::saveProperties(KConfig *config) {
  QString name = doc->absFilePath();
  if (!name.isEmpty() && doc->title() != i18n("Untitled")) {
    config->writePathEntry("Document", name);
    config->writeEntry("NamedDocument", true);
  } else {
    QString sl = KGlobal::dirs()->saveLocation("kst", "kst/");
    int i = 0;
    do {
      name = sl + QString("unsaved%1.kst").arg(i);
    } while(QFile::exists(name));
    doc->saveDocument(name);
    config->writePathEntry("Document", name);
    config->writeEntry("NamedDocument", false);
  }
}

void KstApp::readProperties(KConfig* config) {
  QString name = config->readPathEntry("Document");

  if (name.isEmpty()) {
    return;
  }

  if (config->readBoolEntry("NamedDocument", false)) {
    doc->openDocument(name);
  } else {
    doc->openDocument(name);
    QFile::remove(name);
    doc->setTitle(i18n("Untitled"));
  }
}

bool KstApp::queryClose() {
  if (doc->saveModified()) {
     doc->cancelUpdate();
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
  slotUpdateStatusMsg(i18n("Creating new document..."));

  if (doc->saveModified()) {
    doc->newDocument();
    setCaption(doc->title());
    selectRecentFile(KURL(""));
  }

  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::slotFileOpen() {
  slotUpdateStatusMsg(i18n("Opening file..."));

  if (doc->saveModified(false)) {
    QString fileToOpen = KFileDialog::getOpenFileName("::<kstfiledir>",
        i18n("*.kst|Kst Plot File (*.kst)\n*|All Files"), this, i18n("Open File"));
    if (!fileToOpen.isEmpty()) {
      doc->deleteContents();
      doc->setModified(false);
      if (doc->openDocument(fileToOpen)) {
        setCaption(doc->title());
        addRecentFile(fileToOpen);
      }
    }
  }

  slotUpdateStatusMsg(i18n("Ready"));
}

bool KstApp::slotFileOpenRecent(const KURL& newfile) {
  bool returnVal = false;
  slotUpdateStatusMsg(i18n("Opening file..."));

  if(doc->saveModified()) {
    if (doc->openDocument(newfile)) {
      returnVal = true;
    }
    setCaption(kapp->caption() + ": " + doc->title());
  }

  slotUpdateStatusMsg(i18n("Ready"));
  return returnVal;
}

void KstApp::slotFileSave() {
  if (doc->title() == "Untitled") {
    slotFileSaveAs();
  } else {
    slotUpdateStatusMsg(i18n("Saving file..."));

    doc->saveDocument(doc->absFilePath());

    slotUpdateStatusMsg(i18n("Ready"));
  }
}

bool KstApp::slotFileSaveAs() {
  slotUpdateStatusMsg(i18n("Saving file with a new filename..."));

  while (true) {
    QString newName=
      KFileDialog::getSaveFileName(QDir::currentDirPath(),
                  i18n("*.kst|Kst Plot File (*.kst)\n*|All Files"),
                                  this, i18n("Save As"));
    if(!newName.isEmpty()) {
      QRegExp extension("*.kst", false, true);
      QString longName = newName;
      if (!extension.exactMatch(newName)) {
        longName = newName + QString(".kst");
      }
      if (doc->saveDocument(longName)) {
        addRecentFile(longName);
        QFileInfo saveAsInfo(longName);
        doc->setTitle(saveAsInfo.fileName());
        doc->setAbsFilePath(saveAsInfo.absFilePath());

        setCaption(kapp->caption() + ": " + doc->title());

        slotUpdateStatusMsg(i18n("Ready"));
        return true;
      }
    } else {
      slotUpdateStatusMsg(i18n("Ready"));
      return false;
    }
  }
}

void KstApp::slotFileClose() {
  if (doc->saveModified()) {
     doc->cancelUpdate();
    _stopping = true;
    QTimer::singleShot(0, doc, SLOT(deleteContents()));
    close();
  }
}

void KstApp::immediatePrintWindowToFile(KMdiChildView *win, const QString& filename) {
  KstViewWindow *view = dynamic_cast<KstViewWindow*>(win);
  if (view && !view->view()->children().isEmpty()) {
    view->immediatePrintToFile(filename);
  }
}

void KstApp::immediatePrintActiveWindowToFile(const QString& filename) {
  KstViewWindow *view = dynamic_cast<KstViewWindow*>(activeWindow());
  if (view) {
    view->immediatePrintToFile(filename);
  }
}

void KstApp::immediatePrintWindowToPng(KMdiChildView *win, const QString& filename, const QString& format, int width, int height, int display) {
  KstViewWindow *view = dynamic_cast<KstViewWindow*>(win);
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

    view->immediatePrintToPng(filename, size, format);
  }
}

void KstApp::immediatePrintActiveWindowToPng(const QString& filename, const QString& format, int width, int height, int display) {
  KMdiChildView *win = activeWindow();

  if (win) {
    immediatePrintWindowToPng(win, filename, format, width, height, display);
  }
}

void KstApp::slotFilePrint() {
  KstViewWindow *currentWin = dynamic_cast<KstViewWindow*>(activeWindow());
  KstViewWindow *win;
  int currentPage = 0;  
  int pages = 0;
  
  KMdiIterator<KMdiChildView*> *it = createIterator();
  if (it) {
    while (it->currentItem()) {
      win = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (win && !win->view()->children().isEmpty()) {
        pages++;
      }
      if (win == currentWin) {
        currentPage = pages;
      }
      it->next();
    }
    deleteIterator(it);
  }

  if (pages > 0) {
    KPrinter printer;
    KstSettings *ks = KstSettings::globalSettings();
        
    printer.setOption("kde-pagesize", ks->printing.pageSize);
    printer.setOption("kde-orientation", ks->printing.orientation);
    printer.setOption("kst-plot-datetime-footer", ks->printing.plotDateTimeFooter);
    printer.setOption("kst-plot-maintain-aspect-ratio", ks->printing.maintainAspect);
    printer.setOption("kst-plot-curve-width-adjust", ks->printing.curveWidthAdjust);
    printer.setOption("kst-plot-monochrome", ks->printing.monochrome);
    printer.setFromTo(0, 0);
    printer.setMinMax(1, pages);
    printer.setCurrentPage(currentPage);
    printer.setPageSelection(KPrinter::ApplicationSide);

    pages = 0;
    printer.addDialogPage(new KstPrintOptionsPage);
    if (printer.setup(this, i18n("Print"))) {
      QPainter paint(&printer);
      QPaintDeviceMetrics metrics(&printer);
      QSize size(metrics.width(), metrics.height());
      int iFromPage = printer.fromPage();
      int iToPage = printer.toPage();
      bool datetimeFooter = printer.option("kst-plot-datetime-footer") == "1";
      bool maintainAspectRatio = printer.option("kst-plot-maintain-aspect-ratio") == "1";
      int lineAdjust = printer.option("kst-plot-curve-width-adjust").toInt();
      bool monochrome = printer.option("kst-plot-monochrome") == "1";

      slotUpdateStatusMsg(i18n("Printing..."));

      ks->printing.pageSize = printer.option("kde-pagesize");
      ks->printing.orientation = printer.option("kde-orientation");
      ks->printing.plotDateTimeFooter = printer.option("kst-plot-datetime-footer");
      ks->printing.maintainAspect = printer.option("kst-plot-maintain-aspect-ratio");
      ks->printing.curveWidthAdjust = printer.option("kst-plot-curve-width-adjust");
      ks->printing.monochrome = printer.option("kst-plot-monochrome");
      ks->save();

#if KDE_VERSION < KDE_MAKE_VERSION(3,3,0)
      int iFromPage = printer.fromPage();
      int iToPage = printer.toPage();
      
      if (iFromPage == 0 && iToPage == 0) {
        printer.setPageSelection(KPrinter::SystemSide);
      }
#else
      QValueList<int> pageList = printer.pageList();
#endif
      
      it = createIterator();
      if (it) {
        bool firstPage = true;
        while (it->currentItem()) {
          win = dynamic_cast<KstViewWindow*>(it->currentItem());
          if (win && !win->view()->children().isEmpty()) {
            pages++;
#if KDE_VERSION < KDE_MAKE_VERSION(3,3,0)
            if ((iFromPage == 0 && iToPage == 0) ||
                (iFromPage <= pages && iToPage >= pages)) {
#else
            if (pageList.contains(pages)) {                   
#endif
              if (!firstPage && !printer.newPage()) {
                break;
              }
              KstTopLevelViewPtr tlv = kst_cast<KstTopLevelView>(win->view());
              if (tlv) {
                if (lineAdjust != 0) {
                  tlv->forEachChild(&Kst2DPlot::pushAdjustLineWidth, lineAdjust, false);
                }
                if (monochrome) {
                  tlv->forEachChild2(&Kst2DPlot::pushPlotColors);
                  tlv->forEachChild<const QColor&>(&Kst2DPlot::pushCurveColor, Qt::black, false);
                }
              }

              if (datetimeFooter) {
                QDateTime dateTime = QDateTime::currentDateTime();
                QString title = i18n("Page: %1  Name: %2  Date: %3").arg(pages).arg(win->caption()).arg(dateTime.toString(Qt::ISODate));
                QRect rect(0, 0, size.width(), size.height());
                QRect rectBounding;

                rectBounding = paint.boundingRect(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
                rect.setTop(size.height() - (2 * rectBounding.height()));
                paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
                size.setHeight(rect.top());
              }

              if (maintainAspectRatio) {
                QRect  rectGeom;
                double dRatioWindow;
                double dRatioPrinter;

                rectGeom      = win->view()->geometry();
                dRatioWindow  = (double)rectGeom.width() / (double)rectGeom.height();
                dRatioPrinter = (double)size.width() / (double)size.height();
                if (dRatioWindow > dRatioPrinter) {
                  size.setHeight((int)((double)size.width() / dRatioWindow));
                } else if (dRatioWindow < dRatioPrinter) {
                  size.setWidth((int)(dRatioWindow * (double)size.height()));
                }
              }

              win->view()->resizeForPrint(size);
              win->view()->paint(P_PRINT, paint);

              if (tlv) {
                if (lineAdjust != 0) {
                  tlv->forEachChild2(&Kst2DPlot::popLineWidth);
                }
                if (monochrome) {
                  tlv->forEachChild2(&Kst2DPlot::popPlotColors);
                  tlv->forEachChild2(&Kst2DPlot::popCurveColor);
                }
              }

              win->view()->revertForPrint();

              firstPage = false;
            }
          }
          it->next();
        }
        deleteIterator(it);
      }

      slotUpdateStatusMsg(i18n("Ready"));
    }
  } else {
    slotUpdateStatusMsg(i18n("Nothing to print"));
  }
}

void KstApp::immediatePrintToFile(const QString& filename) {
  KPrinter printer;

  printer.setPageSize(KPrinter::Letter);
  printer.setOrientation(KPrinter::Landscape);
  printer.setOutputToFile(true);
  printer.setOutputFileName(filename);
  printer.setFromTo(0, 0);

  KMdiIterator<KMdiChildView*> *it = createIterator();
  if (it) {
    bool firstPage = true;
    QPainter paint(&printer);
    QPaintDeviceMetrics metrics(&printer);
    QRect rect;
    QSize size(metrics.width(), metrics.height());

    rect.setLeft(0);
    rect.setRight(size.height());
    rect.setBottom(size.height());
#if 0
    QDateTime dateTime = QDateTime::currentDateTime();
    size.setHeight(9 * size.height() / 10);
#endif
    rect.setTop(size.height());

    while (it->currentItem()) {
      KstViewWindow *view = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (view && !view->view()->children().isEmpty()) {
        if (!firstPage && !printer.newPage()) {
          break;
        }

#if 0
        QString title = i18n("Name: %1  Date: %2").arg(view->caption()).arg(dateTime.toString(Qt::ISODate));
#endif

        view->view()->resizeForPrint(size);
#if 0
        paint.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, title);
#endif
        view->view()->paint(P_PRINT, paint);
        view->view()->revertForPrint();

        firstPage = false;
      }
      it->next();
    }
    deleteIterator(it);
  }
}

void KstApp::immediatePrintToPng(const QString& filename, const QString& format, int width, int height, bool all, int display) {
  if (all) {
    QString filenameSub;
    int pages = 0;

    QString dotFormat = i18n(".%1").arg(format);
    int iPos = filename.findRev(dotFormat, -1, FALSE);
    if (iPos != -1 && iPos == (int)(filename.length() - dotFormat.length())) {
      filenameSub = filename.left(filename.length() - dotFormat.length());
    } else {
      filenameSub = filename;
    }

    KMdiIterator<KMdiChildView*>* it = createIterator();
    if (it) {
      while (it->currentItem()) {
        pages++;
        QString filenameNew = i18n("%1_%2").arg(filenameSub).arg(pages);
        immediatePrintWindowToPng(it->currentItem(), filenameNew, format, width, height, display);
        it->next();
      }
      deleteIterator(it);
    }
  } else {
    immediatePrintActiveWindowToPng(filename, format, width, height, display);
  }
}

void KstApp::slotFileQuit() {
  slotFileClose();
}

void KstApp::slotViewStatusBar() {
  if (StatusBarAction->isChecked()) {
    statusBar()->show();
  } else {
    statusBar()->hide();
  }
}


void KstApp::slotUpdateStatusMsg(const QString& msg) {
  _readyBar->setText(msg);
}


void KstApp::slotUpdateDataMsg(const QString& msg) {
  _dataBar->setText(msg);
}


void KstApp::slotUpdateProgress(int total, int step, const QString &msg) {
  if (step == 0 && msg.isNull()) {
    _readyBar->setText(i18n("Ready"));
    _progressBar->hide();
    return;
  }

  _progressBar->show();
  if (step > 0) {
    if (!_progressBar->percentageVisible()) {
      _progressBar->setPercentageVisible(true);
    }
    if (total != _progressBar->totalSteps()) {
      _progressBar->setTotalSteps(total);
    }
    if (_progressBar->progress() != step) {
      _progressBar->setProgress(step);
    }
  } else {
    _progressBar->setPercentageVisible(false);
    _progressBar->reset();
  }

  if (msg.isEmpty()) {
    _readyBar->setText(i18n("Ready"));
  } else {
    _readyBar->setText(msg);
  }

  kapp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers, 10);
}


bool KstApp::dataMode() const {
  return DataMode->isChecked();
}


void KstApp::toggleDataMode() {
  //DataMode->setChecked(!DataMode->isChecked());
  KstTopLevelViewPtr pView = activeView();
  if (pView) {
    pView->widget()->paint();
  }
  slotUpdateDataMsg(QString::null);
}


void KstApp::toggleMouseMode() {
  KMdiIterator<KMdiChildView*> *it = createIterator();
  if (it) {
    while (it->currentItem()) {
      KstViewWindow *pView = dynamic_cast<KstViewWindow*>(it->currentItem());
      if (pView) {
        pView->view()->setViewMode(LayoutAction->isChecked() ? KstTopLevelView::LayoutMode : KstTopLevelView::DisplayMode);
      }
      it->next();
    }
    deleteIterator(it);
  }
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

  paintAll(P_PAINT);
}

//#define PAINTTIMER
void KstApp::paintAll(KstPaintType pt) {
#ifdef PAINTTIMER
  QTime t;
  t.start();
#endif
  if (mdiMode() == KMdi::TabPageMode || mdiMode() == KMdi::IDEAlMode) { // Optimization
    KstViewWindow *view = dynamic_cast<KstViewWindow*>(activeWindow());
    if (view) {
      view->view()->paint(pt);
    }
  } else {
    KMdiIterator<KMdiChildView*> *it = createIterator();
    if (it) {
      while (it->currentItem()) {
        KstViewWindow *view = dynamic_cast<KstViewWindow*>(it->currentItem());
        if (view) {
          view->view()->paint(pt);
        }
        it->next();
      }
      deleteIterator(it);
    }
  }
#ifdef PAINTTIMER
  int x = t.elapsed();
  kstdDebug() << "paintAll with painttype " << (int)pt << " - " << x << "ms" << endl;
#endif
}

void KstApp::showPlotDialog() {
  KMdiChildView *win = activeWindow();

  if (win && _plotDialog->isHidden()) {
    _plotDialog->show_I(win->caption(), QString::null);
  } else {
    _plotDialog->show_I();
  }
}

void KstApp::showPlotDialog(const QString& window, const QString& plot) {
  _plotDialog->show_I(window, plot);
}

void KstApp::showDataManager() {
  dataManager->show_I();
}

void KstApp::showViewScalarsDialog() {
  viewScalarsDialog->showViewScalarsDialog();
}

void KstApp::showViewVectorsDialog() {
  viewVectorsDialog->showViewVectorsDialog();
}

void KstApp::showViewMatricesDialog() {
  viewMatricesDialog->showViewMatricesDialog();
}

void KstApp::showViewFitsDialog() {
  viewFitsDialog->showViewFitsDialog();
}

void KstApp::showChangeFileDialog() {
  changeFileDialog->showChangeFileDialog();
}

void KstApp::showChangeNptsDialog() {
  changeNptsDialog->showChangeNptsDialog();
}

void KstApp::showGraphFileDialog() {
  graphFileDialog->show_I();
}

void KstApp::showDebugDialog() {
  debugDialog->show_I();
}

void KstApp::samplesUp() {
  setPaused(false);
  doc->samplesUp();
}

void KstApp::samplesDown() {
  setPaused(false);
  doc->samplesDown();
}


void KstApp::updateDataDialogs(bool dm) {
  if (!viewScalarsDialog->isHidden()) {
    viewScalarsDialog->updateViewScalarsDialog();
  }
  if (!viewVectorsDialog->isHidden()) {
    viewVectorsDialog->updateViewVectorsDialog();
  }
  if (!viewMatricesDialog->isHidden()) {
    viewMatricesDialog->updateViewMatricesDialog();
  }
  if (!viewFitsDialog->isHidden()) {
    viewFitsDialog->updateViewFitsDialog();
  }

  if (dm) {
    dataManager->updateContents();
  }

  updateMemoryStatus();
}


void KstApp::updateVisibleDialogs() {
  updateDialogs(true);
}


void KstApp::updateDialogs(bool onlyVisible) {
  if (!_stopping) {
    QTime t;
    t.start();
    if (!onlyVisible || KstVectorDialogI::globalInstance()->isShown()) {
      KstVectorDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstPluginDialogI::globalInstance()->isShown()) {
      KstPluginDialogI::globalInstance()->updateForm();
    }
    if (!onlyVisible || KstFitDialogI::globalInstance()->isShown()) {
      KstFitDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstFilterDialogI::globalInstance()->isShown()) {
      KstFilterDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstEqDialogI::globalInstance()->isShown()) {
      KstEqDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstHsDialogI::globalInstance()->isShown()) {
      KstHsDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstPsdDialogI::globalInstance()->isShown()) {
      KstPsdDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstCurveDialogI::globalInstance()->isShown()) {
      KstCurveDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstEventMonitorI::globalInstance()->isShown()) {
      KstEventMonitorI::globalInstance()->update();
    }
    if (!onlyVisible || KstImageDialogI::globalInstance()->isShown()) {
      KstImageDialogI::globalInstance()->update();
    }
    if (!onlyVisible || KstMatrixDialogI::globalInstance()->isShown()) {
      KstMatrixDialogI::globalInstance()->update();
    }
    if (!onlyVisible || _plotDialog->isShown()) {
      _plotDialog->update();
    }
    if (!onlyVisible || changeFileDialog->isShown()) {
      changeFileDialog->updateChangeFileDialog();
    }
    if (!onlyVisible || changeNptsDialog->isShown()) {
      changeNptsDialog->updateChangeNptsDialog();
    }
    if (!onlyVisible) { // FIXME: might want to make this sensible one day
      updateDataDialogs(false);
    }
    if (!onlyVisible || vectorSaveDialog->isShown()) {
      vectorSaveDialog->init();
    }
    if (!onlyVisible || dataManager->isShown()) {
      dataManager->update();
    }
    kstdDebug() << "Dialogs updated in " << t.elapsed() << "ms" << endl;
  }
}


void KstApp::updateDialogsForWindow() {
  if (!_stopping) {
    KstEqDialogI::globalInstance()->updateWindow();
    KstHsDialogI::globalInstance()->updateWindow();
    KstPsdDialogI::globalInstance()->updateWindow();
    KstCurveDialogI::globalInstance()->updateWindow();
    KstImageDialogI::globalInstance()->updateWindow();
    dataManager->update();
    _plotDialog->updateWindow();
  }
}


void KstApp::showPluginManager() {
  PluginManager *pm = new PluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;

  KstPluginDialogI::globalInstance()->updatePluginList();
}


void KstApp::showExtensionManager() {
  ExtensionDialog *dlg = new ExtensionDialog(this, "Extension Manager");
  dlg->exec();
  delete dlg;
}


void KstApp::showDataWizard() {
  DataWizard *dw = new DataWizard(this, "DataWizard");
  dw->exec();
  if (dw->result() == QDialog::Accepted) {
    delete dw; // leave this here - releases references
    forceUpdate();
    doc->setModified();
    updateDialogs();
  } else {
    delete dw;
  }
}

void KstApp::registerDocChange() {
  kstdDebug() << "register doc changed" << endl;
  forceUpdate();
  updateVisibleDialogs();
  doc->setModified();
}


void KstApp::reload() {
  KstReadLocker ml(&KST::vectorList.lock());
  for (KstVectorList::ConstIterator i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
    KstRVectorPtr r = kst_cast<KstRVector>(*i);
    if (r) {
      r->writeLock();
      r->reload();
      r->writeUnlock();
    }
  }
}


void KstApp::slotPreferences() {
  KstSettingsDlg *ksd = new KstSettingsDlg(this, "Kst Settings Dialog");
  connect(ksd, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
  ksd->exec();
  delete ksd;
}


void KstApp::slotSettingsChanged() {
  _updateThread->setUpdateTime(KstSettings::globalSettings()->plotUpdateTimer);
}


void KstApp::slotCopy() {
  if (!LayoutAction->isChecked()) {
    KstTopLevelViewPtr tlv = activeView();
    if (tlv) {
      KstViewWidget *w = tlv->widget();
      KstViewObjectPtr o = w->findChildFor(w->mapFromGlobal(QCursor::pos()));
      Kst2DPlotPtr p = kst_cast<Kst2DPlot>(o);
      if (p) {
        p->copy();
      }
    }
  } else {
    KstViewWindow *vw = dynamic_cast<KstViewWindow*>(activeWindow());
    if (vw) {
      QApplication::clipboard()->setData(vw->view()->widget()->dragObject(), QClipboard::Clipboard);
    }
  }
}


bool KstApp::paste(QMimeSource* source, KstTopLevelViewPtr tlv) {
  QStringList plotList;
  QString window;
  bool rc = false;

  if (source && source->provides(PlotMimeSource::mimeType())) {
    QDataStream ds(source->encodedData(PlotMimeSource::mimeType()), IO_ReadOnly);
    KstViewWindow *w;

    ds >> window;
    ds >> plotList;

    w = dynamic_cast<KstViewWindow*>(KstApp::inst()->findWindow(window));
    if (w && plotList.size() > 0) {
      for (size_t i=0; i<plotList.size(); i++) {
        KstViewObjectPtr copy = w->view()->findChild(plotList[i]);

        if (copy) {
          QString plotName;
          bool duplicate = true;
          int number = 0;

          while (duplicate) {
            if (number == 0) {
              plotName = copy->tagName();
            } else if (number == 1) {
              plotName = i18n("%1-copy").arg(copy->tagName());
            } else {
              plotName = i18n("%1-copy%2").arg(copy->tagName()).arg(number);
            }
            number++;
            if (tlv->findChild(plotName)) {
              duplicate = true;
            } else {
              duplicate = false;
            }
          }

          copy->copyObjectQuietly(*tlv, plotName);
        }
      }

      rc = true;
    }
  }

  return rc;
}


void KstApp::slotPaste() {
  if (LayoutAction->isChecked()) {
    KstTopLevelViewPtr tlv = activeView();

    if (tlv) {
      QMimeSource* source;

      source = QApplication::clipboard()->data(QClipboard::Clipboard);
      if (!paste(source, tlv)) {
        KstDebug::self()->log(i18n("Paste operation failed: clipboard data was not found or of the wrong format."));
      }
    } else {
      KstDebug::self()->log(i18n("Paste operation failed: there is currently no active view."));
    }
  } else {
    KstDebug::self()->log(i18n("Paste operation failed: must be in layout mode."));
  }
}


void KstApp::slotFileNewWindow(QWidget *parent) {
  newWindow(true, parent);
  doc->setModified();
}


void KstApp::slotFileRenameWindow() {
  KstViewWindow *vw = dynamic_cast<KstViewWindow*>(activeWindow());

  if (vw) {
    QString name = windowName(true, vw->caption(), true);

    if (!name.isEmpty() && vw->caption() != name) {
      vw->setCaption(name);
      vw->setTabCaption(name);
      updateDialogsForWindow();
      doc->setModified();
    }
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

QString KstApp::newWindow(const QString& name_in) {
  KstViewWindow *w = new KstViewWindow;
  QString nameToUse = QString::null;
  QString name  = name_in;

  while (name.isEmpty() || findWindow(name)) {
    name = KST::suggestWinName();
  }
  nameToUse = name;

  w->setCaption(nameToUse);
  w->setTabCaption(nameToUse);
  addWindow(w, KMdi::StandardAdd | KMdi::UseKMdiSizeHint);
  w->activate();
  updateDialogsForWindow();
  return nameToUse;
}


QString KstApp::windowName(bool prompt, const QString& nameOriginal, bool rename, QWidget *parent) {
  bool ok = false;
  QString name = nameOriginal;

  do {
    if (prompt) {
      QRegExp exp("\\S+.*");
      QRegExpValidator val(exp, 0L);

      if (rename) {
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
        name = KInputDialog::getText(i18n("Kst"), i18n("Enter a new name for the window:"), name, &ok, parent, 0L, &val).stripWhiteSpace();
#else
        name = KLineEditDlg::getText(i18n("Enter a new name for the window:"), name, &ok, parent, &val).stripWhiteSpace();
#endif
      } else {
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
        name = KInputDialog::getText(i18n("Kst"), i18n("Enter a name for the new window:"), name, &ok, parent, 0L, &val).stripWhiteSpace();
#else
        name = KLineEditDlg::getText(i18n("Enter a name for the new window:"), name, &ok, parent, &val).stripWhiteSpace();
#endif
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
      KMessageBox::sorry(this, i18n("A window with the same name already exists.  Please enter a unique window name."));
    }
  } while(true);

  return name;
}


KstTopLevelViewPtr KstApp::activeView() {
  KstViewWindow *vw = dynamic_cast<KstViewWindow*>(activeWindow());

  if (!vw) {
    return 0L;
  }

  return vw->view();
}


Kst2DPlotMap& KstApp::plotHolderWhileOpeningDocument() {
  return _plotHolderWhileOpeningDocument;
}


KstLabelDialogI *KstApp::labelDialog() const {
  return _labelDialog;
}


void KstApp::updatePausedState(bool state) {
  _updateThread->setPaused(state);
}


void KstApp::fromEnd() {
  doc->samplesEnd();
  setPaused(false);
}


void KstApp::updateMemoryStatus() {
#ifdef HAVE_LINUX
  meminfo();
  unsigned long mi = S(kb_main_free + kb_main_buffers + kb_main_cached);
  _memoryBar->setText(i18n("%1 MB available").arg(mi / (1024 * 1024)));
#endif
}


const QStringList KstApp::recentFiles() const {
  return recent->items();
}


void KstApp::showQuickStartDialog() {
#ifdef KST_QUICKSTART_DLG
  if (KstSettings::globalSettings()->showQuickStart) {
    _quickStartDialog->show_I();
  }
#endif
}


void KstApp::fixKMdi() {
  KTabWidget *tw = tabWidget();
  if (tw) {
    tw->setHoverCloseButton(false);
  }
}

#include "kst.moc"
// vim: ts=2 sw=2 et
