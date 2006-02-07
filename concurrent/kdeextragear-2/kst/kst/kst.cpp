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

#include <stdlib.h>
#include <stdio.h>

// include files for QT
#include <qdir.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qpushbutton.h>

// include files for KDE
#include <kaccel.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <kprinter.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaction.h>

#include <iostream>

// application specific includes
#include "kst.h"
#include "kstview.h"
#include "kstdoc.h"
#include "kstextension.h"
#include "kstplotdialog_i.h"
#include "kstviewscalarsdialog_i.h"
#include "kstchangefiledialog_i.h"
#include "kstchangenptsdialog_i.h"
#include "kstquickcurvesdialog_i.h"
#include "kstquickpsddialog_i.h"
#include "kstgraphfiledialog_i.h"
#include "kstvectordialog_i.h"
#include "kstcurvedialog_i.h"
#include "ksteqdialog_i.h"
#include "ksthsdialog_i.h"
#include "kstpsddialog_i.h"
#include "kstplugindialog_i.h"
#include "kstdatamanager_i.h"
#include "kstlabeldialog_i.h"
#include "kstiface_impl.h"
#include "kstsettingsdlg.h"
#include "filterlisteditor.h"
#include "updatethread.h"
#include "vectorsavedialog.h"

#define KST_STATUSBAR_FRAME 0
#define KST_STATUSBAR_DATA 1
#define KST_STATUSBAR_STATUS 2

KstApp::KstApp(QWidget *parent, const char* name)
: KMainWindow(parent, name) {

  clearWFlags(WDestructiveClose);

  stopping = false;
  config = kapp->config();
  initStatusBar();

  initDocument();
  initView();

  /* create dialogs */
  plotDialog = new KstPlotDialogI(doc, this);
  dataManager = new KstDataManagerI(doc, this);
  filterListEditor = new FilterListEditor(this);
  //viewScalarsDialog = new KstViewScalarsDialogI(this);
  changeFileDialog = new KstChangeFileDialogI(this);
  changeNptsDialog = new KstChangeNptsDialogI(this);
  quickCurvesDialog = new KstQuickCurvesDialogI(this);
  quickPSDDialog = new KstQuickPSDDialogI(this);
  graphFileDialog = new KstGraphFileDialogI(this);
  vectorSaveDialog = new VectorSaveDialog(this);
  connect(KstVectorDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstCurveDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstEqDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstHsDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstPsdDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));
  connect(KstPluginDialogI::globalInstance(), SIGNAL(modified()), doc, SLOT(wasModified()));

  initActions();

  readOptions();

  _updateThread = new UpdateThread(doc);
  // FIXME XXXX KstSettings::globalSettings()->plotUpdateTimer;
  _updateThread->start();

  connect(doc,  SIGNAL(newFrameMsg(int)),
          this, SLOT(slotUpdateFrameMsg(int)));
  connect(view, SIGNAL(newStatusMsg(const QString &)),
          this, SLOT(slotUpdateStatusMsg(const QString &)));
  connect(view, SIGNAL(newDataMsg(const QString &)),
          this, SLOT(slotUpdateDataMsg(const QString &)));

  /*** Plot Dialog signals */
  connect(changeFileDialog, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(changeNptsDialog, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(quickCurvesDialog,SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(quickPSDDialog,   SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(filterListEditor, SIGNAL(docChanged()),
          this,             SLOT(registerDocChange()));
  connect(graphFileDialog,  SIGNAL(graphFileReq(const QString &,int,int)),
          view,             SLOT(printToGraphicsFile(const QString &,int,int)));

  /*** label dialog ***/
  connect(view->labelDialog, SIGNAL(applied()),
          this, SLOT(registerDocChange()));

  /*** plot dialog ***/
  connect(plotDialog, SIGNAL(docChanged()), this, SLOT(registerDocChange()));

  /*** ***/
  connect(doc, SIGNAL(dataChanged()), this, SLOT(updateDialogs()));

  connect(dataManager, SIGNAL(editDataVector(const QString &)),
          KstVectorDialogI::globalInstance(), SLOT(show_I(const QString &)));

  connect(dataManager->OpenPlotDialog, SIGNAL(clicked()),
          this, SLOT(showPlotDialog()));

  setAutoSaveSettings("KST-KMainWindow", true);
  _dcopIface = new KstIfaceImpl(doc, this);
  view->forceUpdate();

  connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));

  QTimer::singleShot(0, this, SLOT(updateActions()));

  // Load any extensions
  KService::List sl = KServiceType::offers("Kst Extension");
  for (KService::List::ConstIterator it = sl.begin(); it != sl.end(); ++it) {
    KService::Ptr service = *it;
    kdDebug() << "Found Kst Extension " << service->property("Name").toString() << endl;
    int err = 0;
    KstExtension *e = KParts::ComponentFactory::createInstanceFromService<KstExtension>(service, this, 0, QStringList(), &err);
    if (e) {
      kdDebug() << "Kst Extension " << service->property("Name").toString() << " loaded" << endl;
    } else {
      kdDebug() << "Error trying to load extension: " << err << endl;
      if (err == KParts::ComponentFactory::ErrNoLibrary) {
        kdDebug() << "Library error: " << KLibLoader::self()->lastErrorMessage() << endl;
      }
    }
  }
}

void KstApp::updateActions() {
  // Hack
  ToolBarAction->setChecked(toolBar("mainToolBar")->isShown());
  StatusBarAction->setChecked(statusBar()->isShown());
}

KstApp::~KstApp() {
  _updateThread->setFinished(true);
  if (!_updateThread->wait(250)) { // 250ms
    _updateThread->terminate();
    _updateThread->wait();
  }

  delete _updateThread;
  delete _dcopIface;
  _dcopIface = 0L;
}

void KstApp::initActions() {

  fileNewWindow = new KAction(i18n("New &Window"), 0, 0,
                              this, SLOT(slotFileNewWindow()),
                              actionCollection(),"file_new_window");

  fileOpenNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
  fileOpenNew->setWhatsThis(i18n("Open Kst plot file."));

  fileSave = KStdAction::save(   this, SLOT(slotFileSave()), actionCollection());
  fileSave->setWhatsThis(i18n("Save to current Kst plot file."));

  fileSaveAs = KStdAction::saveAs( this, SLOT(slotFileSaveAs()), actionCollection());
  fileSaveAs->setWhatsThis(i18n("Save to new Kst plot file."));

  fileClose = KStdAction::close(  this, SLOT(slotFileClose()), actionCollection());
  fileClose->setWhatsThis(i18n("Close Kst."));

  fileQuit = KStdAction::quit(   this, SLOT(slotFileClose()), actionCollection());
  fileQuit->setWhatsThis(i18n("Quit Kst."));

  fileKeyBindings = KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
  fileKeyBindings->setWhatsThis(i18n("Bring up a dialog box\n"
                                     "to configure shortcuts."));

  filePreferences = KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection());
  filePreferences->setWhatsThis(i18n("Bring up a dialog box\n"
                                     "to configure Kst settings."));

  fileCopy = KStdAction::copy(this, SLOT(slotCopy()), actionCollection());
  fileCopy->setWhatsThis(i18n("Copy cursor position to the clipboard."));

  /************/
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()),
                                actionCollection());
  filePrint->setToolTip(i18n("Print"));
  filePrint->setWhatsThis(i18n("Print current display"));

  /************/
  ToolBarAction = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
                                          actionCollection());
  ToolBarAction->setWhatsThis(i18n("Toggle Toolbar"));
  connect(ToolBarAction, SIGNAL(activated()), this, SLOT(setSettingsDirty()));

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

  /************/
  TiedZoomAction = new KAction(i18n("&Tied Zoom"),"kst_zoomtie",0,
                               view, SLOT(toggleTiedZoom()),
                               actionCollection(), "zoomtie_action");
  TiedZoomAction->setToolTip(i18n("Toggle tied zoom"));
  TiedZoomAction->setWhatsThis(i18n("Apply zoom actions to all plots\n"
                                     "(not just the active one)."));

  /************/
  XYZoomAction = new KRadioAction(i18n("XY Mouse &Zoom"), "kst_zoomxy",0,
                                  actionCollection(), "zoomxy_action");
  XYZoomAction->setExclusiveGroup("zoom");
  XYZoomAction->setToolTip(i18n("XY mouse zoom"));
  XYZoomAction->setWhatsThis(i18n("XY zoom: mouse zooming effects\n"
                                  "both X and Y axis"));
  XYZoomAction->setChecked(true);

  /************/
  XZoomAction = new KRadioAction(i18n("&X Mouse Zoom"), "kst_zoomx",0,
                                 actionCollection(), "zoomx_action");
  XZoomAction->setExclusiveGroup("zoom");
  XZoomAction->setToolTip(i18n("X mouse zoom"));
  XZoomAction->setWhatsThis(i18n("X zoom: Mouse zooming effects only the\n"
                                 "X axis (CTRL-mouse also does this)"));

  /************/
  YZoomAction = new KRadioAction(i18n("&Y Mouse Zoom"), "kst_zoomy",0,
                                  actionCollection(), "zoomy_action");
  YZoomAction->setExclusiveGroup("zoom");
  YZoomAction->setToolTip(i18n("Y mouse zoom"));
  YZoomAction->setWhatsThis(i18n("Y zoom: Mouse zooming effects only the\n"
                                 "Y axis (SHIFT-mouse also does this)"));

  /************/
  TextAction = new KRadioAction(i18n("&Label Editor"), "text",0,
                                  actionCollection(), "label_action");

  TextAction->setExclusiveGroup("zoom");
  TextAction->setToolTip(i18n("Label Editor"));
  TextAction->setWhatsThis(i18n("Use the mouse to create and edit labels."));

  /************/
  FilterDialogAction = new KAction(i18n("Edit &Filters"), 0, 0,
                                 this, SLOT(showFilterListEditor()),
                                 actionCollection(), "filterdialog_action");
  FilterDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to edit filters."));

  /************/
  PlotDialogAction = new KAction(i18n("Edit &Plots"), "kst_editplots", 0,
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
  VectorDialogAction = new KAction(i18n("Edit &Vectors"), 0, 0,
                                 KstVectorDialogI::globalInstance(),
                                 SLOT(show_I()), actionCollection(),
                                 "vectordialog_action");
  VectorDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to edit or create vetors."));

  /************/
  CurveDialogAction = new KAction(i18n("Edit &Curves"), 0, 0,
                                  KstCurveDialogI::globalInstance(),
                                  SLOT(show_I()), actionCollection(),
                                  "curvedialog_action");
  CurveDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                       "to edit or create curves."));

  /************/
  EqDialogAction = new KAction(i18n("Edit &Equations"), 0, 0,
                               KstEqDialogI::globalInstance(), SLOT(show_I()),
                               actionCollection(), "eqdialog_action");
  EqDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                    "to edit or create equations."));

  /************/
  HsDialogAction = new KAction(i18n("Edit &Histograms"), 0, 0,
                               KstHsDialogI::globalInstance(),
                               SLOT(show_I()), actionCollection(),
                               "hsdialog_action");
  HsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                    "to edit or create histograms."));

  /************/
  PsdDialogAction = new KAction(i18n("Edit Power &Spectra"), 0, 0,
                                KstPsdDialogI::globalInstance(),
                                SLOT(show_I()), actionCollection(),
                                "psddialog_action");
  PsdDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                     "to edit or create power spectra."));

  /************/
  PluginDialogAction = new KAction(i18n("Edit Plugins"), 0, 0,
                                   KstPluginDialogI::globalInstance(),
                                   SLOT(show_I()), actionCollection(),
                                   "plugindialog_action");
  PluginDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                        "to use plugins."));

  /************/
  ChangeFileDialogAction = new KAction(i18n("Change Data &File"),
                                       "kst_changefile", 0, this,
                                       SLOT(showChangeFileDialog()),
                                       actionCollection(),
                                       "changefiledialog_action");
  ChangeFileDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to change input files."));

  /************/
#if 0
  ViewScalarsDialogAction = new KAction(i18n("View &Scalars"),
                                       0, 0, this,
                                       SLOT(showViewScalarsDialog()),
                                       actionCollection(),
                                       "viewscalarsdialog_action");
  ViewScalarsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to view scalar values."));
#endif
  
  /************/
  ChangeNptsDialogAction = new KAction(i18n("Change Data Sample &Ranges"),
                                       "kst_changenpts", 0, this,
                                       SLOT(showChangeNptsDialog()),
                                       actionCollection(),
                                       "changenptsdialog_action");
  ChangeNptsDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                            "to change data sample ranges."));

  /************/
  QuickCurvesDialogAction = new KAction(i18n("Quickly Create New Curve"),
                                       "kst_quickcurves", 0, this,
                                       SLOT(showQuickCurvesDialog()),
                                       actionCollection(),
                                       "quickcurvesdialog_action");
  QuickCurvesDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                           "to create a data curve\n"
                                           "and put it in a plot."));
  /************/
  QuickPSDDialogAction = new KAction(i18n("Quickly Create New PSD"),
                                     "kst_quickpsd", 0,
                                     this, SLOT(showQuickPSDDialog()),
                                     actionCollection(),
                                     "quickpsddialog_action");
  QuickPSDDialogAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                          "to create a PSD\n"
                                          "and put it in a plot."));

  /************/
  GraphFileDialogAction = new KAction(i18n("Export to Graphics File..."),
                                  "kst_graphfile", 0,
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
                                  KAccel::stringToKey("Ctrl+Left"),
                                  this, SLOT(samplesDown()),
                                  actionCollection(),
                                  "samplesdown_action");
  //SamplesDownAction->setToolTip(i18n("Back"));
  SamplesDownAction->setWhatsThis(i18n("Reduce the starting frame by\n"
                                       "the current number of frames."));

  /************/
  SamplesUpAction = new KAction(i18n("&Advance 1 Screen"),
                                "kst_advance",
                                KAccel::stringToKey("Ctrl+Right"),
                                this, SLOT(samplesUp()),
                                actionCollection(),
                                "samplesup_action");

  //SamplesUpAction->setToolTip(i18n("Advance"));
  SamplesUpAction->setWhatsThis(i18n("Increase the starting frame by\n"
                                     "the current number of frames."));

  /************/
  SamplesFromEndAction = new KAction(i18n("Read From &End"),
                                     "1rightarrow",
                                     KAccel::stringToKey("Shift+Ctrl+Right"),
                                     this, SLOT(samplesEnd()),
                                     actionCollection(),
                                     "samplesend_action");
  SamplesFromEndAction->setToolTip(i18n("Read from end"));
  SamplesFromEndAction->setWhatsThis(i18n("Read current data from end of file."));

  /************/
  PluginManagerAction = new KAction(i18n("&Plugins"), 0, 0,
                                 this, SLOT(showPluginManager()),
                                 actionCollection(), "pluginmanager_action");
  PluginManagerAction->setWhatsThis(i18n("Bring up a dialog box\n"
                                           "to manage plugins."));


  /************/
  DataMode = new KToggleAction(i18n("Data Mode"), "kst_datamode", 0,
                                 this, SLOT(toggleDataMode()),
                                 actionCollection(), "datamode_action");
  DataMode->setWhatsThis(i18n("Toggle between cursor mode and data mode."));

  /************/
  _reloadAction = new KAction(i18n("Reload"), "reload", Key_F5, this, SLOT(reload()),
                              actionCollection(), "reload");
  _reloadAction->setWhatsThis(i18n("Reload the data from file."));

  createGUI();
}


void KstApp::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
}


void KstApp::setPaused(bool in_paused) {
  PauseAction->setChecked(in_paused);
}

void KstApp::togglePaused() {
  if (PauseAction->isChecked()) {
    PauseAction->setChecked(false);
  } else {
    PauseAction->setChecked(true);
  }
}

KstMouseModeType KstApp::getMouseZoomRadio() {
  if (XZoomAction->isChecked()) {
    return X_ZOOMBOX;
  } else if (YZoomAction->isChecked()) {
    return Y_ZOOMBOX;
  } else if (TextAction->isChecked()) {
    return LABEL_TOOL;
  } else {
    return XY_ZOOMBOX;
  }
}

bool KstApp::getTieZoom() {
  return 0; // (TiedZoomAction->isChecked());
}

void KstApp::initStatusBar() {
  statusBar()->insertFixedItem(i18n("Frame: 00000000"), KST_STATUSBAR_FRAME);
  statusBar()->setItemAlignment(KST_STATUSBAR_FRAME, AlignLeft | AlignVCenter);
  statusBar()->changeItem(i18n("Frame: 0"), KST_STATUSBAR_FRAME);

  statusBar()->insertItem(i18n("Almost Ready"), KST_STATUSBAR_STATUS);
  _dataBar = new QLabel("(00000000, 00000000)", statusBar());
  statusBar()->addWidget(_dataBar, 0, true);

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

void KstApp::initView() {
  view = new KstView(this);
  connect(this, SIGNAL(settingsChanged()), view, SLOT(update()));
  setCentralWidget(view);
  setCaption(doc->getTitle());
}

void KstApp::addRecentFile(const KURL &url) {
  recent->addURL(url);
}

bool KstApp::openDocumentFile(const QString& in_filename,
			      const QString& o_file, int o_n, int o_f, int o_s, bool o_ave) {
  KURL url;
  QFileInfo finfo(in_filename);
  bool rc = false;

  url.setPath(in_filename);
  slotUpdateStatusMsg(i18n("Opening file..."));

  if (doc->openDocument(url, o_file, o_n, o_f, o_s, o_ave)) {
    setCaption(doc->getTitle());
    if (url.isLocalFile()) {
      addRecentFile(finfo.absFilePath());
    } else {
      addRecentFile(url);
    }
    rc = true;
  }
  slotUpdateStatusMsg(i18n("Ready"));
  view->show();
  return rc;
}


KstDoc *KstApp::getDocument() const {
  return doc;
}

void KstApp::saveOptions() {
  config->setGroup("General Options");
  recent->saveEntries(config, "Recent Files");
}

void KstApp::readOptions() {
  config->setGroup("General Options");
  recent->loadEntries(config,"Recent Files");
}

void KstApp::saveProperties(KConfig *config) {
  QString name = doc->getAbsFilePath();
  if (!name.isEmpty() && doc->getTitle() != i18n("Untitled")) {
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
  return doc->saveModified();
}

bool KstApp::queryExit() {
  saveOptions();
  return true;
}

void KstApp::slotFileNewWindow() {
  slotUpdateStatusMsg(i18n("Opening a new application window..."));

  KstApp *new_window= new KstApp;
  new_window->show();

  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::slotFileNew() {
  slotUpdateStatusMsg(i18n("Creating new document..."));

  if (doc->saveModified()) {
    doc->newDocument();
    view->update();
    setCaption(doc->getTitle());
  }

  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::slotFileOpen() {
  slotUpdateStatusMsg(i18n("Opening file..."));

  if(doc->saveModified()) {
    QString fileToOpen=
      KFileDialog::getOpenFileName("::<kstfiledir>",
				   i18n("*.kst|Kst Plot File "
					"(*.kst)\n*|All Files"),
                                   this, i18n("Open File"));
    if(!fileToOpen.isEmpty()) {
      doc->openDocument(fileToOpen);
      view->update();
      setCaption(doc->getTitle());
      addRecentFile(fileToOpen);
    }
  }

  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::slotFileOpenRecent(const KURL &newfile) {
  slotUpdateStatusMsg(i18n("Opening file..."));

  if(doc->saveModified()) {
    doc->openDocument(newfile);
    view->update();
    setCaption(kapp->caption() + ": " + doc->getTitle());
  }

  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::slotFileSave() {
  if (doc->getTitle() == "Untitled") {
    slotFileSaveAs();
  } else {
    slotUpdateStatusMsg(i18n("Saving file..."));

    doc->saveDocument(doc->getAbsFilePath());

    slotUpdateStatusMsg(i18n("Ready"));
  }
}

void KstApp::slotFileSaveAs() {
  slotUpdateStatusMsg(i18n("Saving file with a new filename..."));

  QString newName=
    KFileDialog::getSaveFileName(QDir::currentDirPath(),
                 i18n("*.kst|Kst Plot File (*.kst)\n*|All Files"),
                                 this, i18n("Save As"));
  if(!newName.isEmpty()) {
    if (doc->saveDocument(newName)) {
      addRecentFile(newName);

      QFileInfo saveAsInfo(newName);
      doc->setTitle(saveAsInfo.fileName());
      doc->setAbsFilePath(saveAsInfo.absFilePath());

      setCaption(kapp->caption() + ": " + doc->getTitle());
    }
  }
  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::slotFileClose() {

  if (doc->saveModified()) {
    doc->cancelUpdate();
    stopping = true;
    kapp->processEvents();

    doc->deleteContents();
    close();
  }
}

void KstApp::slotFilePrint() {
  slotUpdateStatusMsg(i18n("Printing..."));

  KPrinter printer;
  printer.setPageSize(KPrinter::Letter);
  printer.setOrientation(KPrinter::Landscape);

  // FIXME: Make a better title for the printingdialog
  if (printer.setup(this, i18n("Print Data")))
  {
    view->print(&printer);
  }

  slotUpdateStatusMsg(i18n("Ready"));
}

void KstApp::immediatePrintToFile(const QString &filename) {
  KPrinter printer;
  printer.setPageSize(KPrinter::Letter);
  printer.setOrientation(KPrinter::Landscape);
  printer.setOutputToFile(true);
  printer.setOutputFileName(filename);

  view->print(&printer);
}

void KstApp::immediatePrintToPng(const QString &filename) {
  view->printToGraphicsFile(filename, 640, 480);
}

void KstApp::slotFileQuit() {
  slotFileClose();
}

void KstApp::slotViewToolBar() {
  slotUpdateStatusMsg(i18n("Toggling toolbar..."));
  // turn Toolbar on or off
  if(ToolBarAction->isChecked()) {
    toolBar("mainToolBar")->show();
  } else {
    toolBar("mainToolBar")->hide();
  }

  slotUpdateStatusMsg(i18n("Ready."));
}

void KstApp::slotViewStatusBar() {
  if (StatusBarAction->isChecked()) {
    statusBar()->show();
  } else {
    statusBar()->hide();
  }
}

void KstApp::slotUpdateStatusMsg(const QString &msg) {
  statusBar()->changeItem(msg, KST_STATUSBAR_STATUS);
}

void KstApp::slotUpdateFrameMsg(int frame) {
  statusBar()->changeItem(i18n("Frame: ")+QString::number(frame),
                               KST_STATUSBAR_FRAME);
}

void KstApp::slotUpdateDataMsg(const QString &msg) {
  _dataBar->setText(msg);
}

void KstApp::toggleDataMode() {
  view->setDataMode(DataMode->isChecked());
}

/** Update everything 
void KstApp::slotTimer() {
  KstObject::UpdateType U;

  if (PauseAction->isChecked()) { // Paused: don't update
    return;
  }

  // pause during mouse zooming
  KstMouseModeType M = view->MouseInfo->getMode();
  if (M == XY_ZOOMBOX || M == Y_ZOOMBOX || M == X_ZOOMBOX) {
    timer->start(KstSettings::globalSettings()->plotUpdateTimer, false);
    return;
  }
*/

void KstApp::showPlotDialog() {
  plotDialog->show_I();
}

void KstApp::showDataManager() {
  dataManager->show_I();
}

void KstApp::showViewScalarsDialog() {
  //viewScalarsDialog->showViewScalarsDialog();
}

void KstApp::showChangeFileDialog() {
  changeFileDialog->showChangeFileDialog();
}

void KstApp::showChangeNptsDialog() {
  changeNptsDialog->showChangeNptsDialog();
}

void KstApp::showQuickCurvesDialog() {
  quickCurvesDialog->showQuickCurvesDialog();
}

void KstApp::showQuickPSDDialog() {
  quickPSDDialog->showQuickPSDDialog();
}

void KstApp::showGraphFileDialog() {
  graphFileDialog->showGraphFileDialog();
}

void KstApp::showFilterListEditor() {
  filterListEditor->show();
}

void KstApp::samplesUp() {
  setPaused(false);
  doc->samplesUp();
}

void KstApp::samplesDown() {
  setPaused(false);
  doc->samplesDown();
}

void KstApp::samplesEnd() {
  doc->samplesEnd();
  setPaused(false);
}


void KstApp::updateDialogs() {
  KstVectorDialogI::globalInstance()->update();
  KstPluginDialogI::globalInstance()->update();
  KstEqDialogI::globalInstance()->update();
  KstHsDialogI::globalInstance()->update();
  KstPsdDialogI::globalInstance()->update();
  KstCurveDialogI::globalInstance()->update();
  plotDialog->update();
  changeFileDialog->updateChangeFileDialog();
  changeNptsDialog->updateChangeNptsDialog();
  quickCurvesDialog->update();
  quickPSDDialog->update();
  dataManager->update();
  filterListEditor->update();
#if 0
  if (!viewScalarsDialog->isHidden()) {
    viewScalarsDialog->updateViewScalarsDialog();
  }
#endif
  vectorSaveDialog->init();
  view->update();
}


#include "pluginmanager.h"

void KstApp::showPluginManager() {
  PluginManager *pm = new PluginManager(this, "Plugin Manager");
  pm->exec();
  delete pm;

  KstPluginDialogI::globalInstance()->updatePluginList();
}


void KstApp::registerDocChange() {
  doc->forceUpdate();
  doc->setModified();
}


void KstApp::reload() {
  KstReadLocker ml(&KST::vectorList.lock());
  for (KstVectorList::Iterator i = KST::vectorList.begin(); i != KST::vectorList.end(); ++i) {
    KstRVector *r = dynamic_cast<KstRVector*>((*i).data());
    if (r) {
      r->reload();
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
  // FIXME XXXX KstSettings::globalSettings()->plotUpdateTimer
}


void KstApp::slotCopy() {
  view->copy();
}

#include "kst.moc"
// vim: ts=2 sw=2 et
