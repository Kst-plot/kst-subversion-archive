
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

#include <QClipboard>
#include <QDateTime>
#include <QDesktopWidget>
#include <QEvent>
#include <QEventLoop>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog> 
#include <QProgressBar>
#include <QStatusBar>
#include <QValidator>

// xxx #include "extensiondlg.h"
#include "extensionmgr.h"
#include "kst.h"
#include "kst2dplot.h"
/* xxx
#include "kstchangefiledialog.h"
#include "kstchangenptsdialog.h"
*/
#include "kstchoosecolordialog.h"
#include "kstcurvedifferentiate.h"
#include "kstcurvedialog.h"
#include "kstcsddialog.h"
#include "kstdatamanager.h"
// xxx #include "kstdatanotifier.h"
// xxx #include "kstdatawizard.h"
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
// xxx #include "kstiface_impl.h"
#include "kstimagedialog.h"
#include "kstlegenddefaults.h"
#include "kstlogwidget.h"
#include "kstmatrixdialog.h"
#include "kstmatrixdefaults.h"
#include "kstmonochromedialog.h"
#include "kstplugindialog.h"
#include "kstbasicdialog.h"
#include "kstpluginmanager.h"
// xxx #include "kstprintoptionspage.h"
#include "kstpsddialog.h"
#include "kstquickstartdialog.h"
#include "kstsettingsdlg.h"
#include "kstuinames.h"
#include "kstvectordefaults.h"
#include "kstvectorsavedialog.h"
#include "kstvectordialog.h"
/* xxx
#include "kstviewmanager.h"
#include "kstviewscalarsdialog.h"
#include "kstviewstringsdialog.h"
#include "kstviewvectorsdialog.h"
#include "kstviewmatricesdialog.h"
#include "kstviewfitsdialog.h"
*/
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
static QSettings *qSettingsObject = 0L;
const QString& KstApp::defaultTag = KGlobal::staticQString("<Auto Name>");

void KstApp::doubleCheckCleanup() {
  KstApp *ptr = ::inst; // guard to prevent double delete on recursion
  ::inst = 0L;

  delete ptr;
}


KstApp* KstApp::inst() {
  return ::inst;
}


KstApp::KstApp(QWidget *parent, const char *name) : QMainWindow(parent) {
  setupUi(this);

  ::inst = this;

// xxx  _dataNotifier = 0L;
  _dataBar = 0L;
  _readyBar = 0L;
  _progressBar = 0L;
  _memoryBar = 0L;
  _toolBar = 0L;
  _menuBar = 0L;
  _updateThread = 0L;

  //
  // create the mdi area...
  //

  _mdiArea = new QMdiArea(this);
  _mdiArea->setViewMode(QMdiArea::TabbedView);
  _mdiArea->setDocumentMode(true);
  _mdiArea->setTabShape(QTabWidget::Rounded);
  setCentralWidget(_mdiArea);
  setWindowIcon(QIcon(":/kst.png"));

  _updatesFromScriptEnabled = true;
  _plotHolderWhileOpeningDocument = new Kst2DPlotMap;
// xxx  KGlobal::dirs()->addResourceType("kst", KStandardDirs::kde_default("data") + "kst");

  _dataSourceConfig = qSettingsObject;

// xxx  clearWFlags(WDestructiveClose);

  _stopping = false;

  initDialogs();
  initMenuBar();
  initToolBar();
  initStatusBar();
  initActions();
  initDocument();

// xxx  KstDebug::self()->setHandler(_doc);
// xxx  setWindowTitle(_doc->title());

// xxx  readOptions();
// xxx  toggleMouseMode();

/* xxx
  _updateThread = new UpdateThread(_doc);
  _updateThread->setUpdateTime(KstSettings::globalSettings()->plotUpdateTimer);
  _updateThread->start();
*/
  //
  // plot Dialog signals...
  //
/* xxx
  connect(_changeFileDialog, SIGNAL(docChanged()), this, SLOT(registerDocChange()));
  connect(_changeNptsDialog, SIGNAL(docChanged()), this, SLOT(registerDocChange()));
  connect(_graphFileDialog, SIGNAL(graphFileReq(const QString&,const QString&,int,int,bool,int)), this, SLOT(immediatePrintToPng(const QString&,const QString&,int,int,bool,int)));
  connect(_graphFileDialog, SIGNAL(graphFileEpsReq(const QString&,int,int,bool,int)), this, SLOT(immediatePrintToEps(const QString&,int,int,bool,int)));
*/
  //
  // data manager signals...
  //
/* xxx
  connect(_doc, SIGNAL(updateDialogs()), this, SLOT(updateDialogs()));
  connect(_doc, SIGNAL(dataChanged()), this, SLOT(updateDataDialogs()));
  connect(_dataManager, SIGNAL(docChanged()), this, SLOT(registerDocChange()));
  connect(_dataManager, SIGNAL(editDataVector(const QString&)), KstVectorDialog::globalInstance(), SLOT(showEdit(const QString&)));
  connect(_dataManager, SIGNAL(editStaticVector(const QString&)), KstVectorDialog::globalInstance(), SLOT(showEdit(const QString&)));
  connect(_dataManager, SIGNAL(editDataMatrix(const QString&)), KstMatrixDialog::globalInstance(), SLOT(showEdit(const QString&)));
  connect(_dataManager, SIGNAL(editStaticMatrix(const QString&)), KstMatrixDialog::globalInstance(), SLOT(showEdit(const QString&)));
*/

// xxx  setAutoSaveSettings("KST-KMainWindow", true);
// xxx  _dcopIface = new KstIfaceImpl(_doc, this);

// xxx  connect(this, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));

  QTimer::singleShot(0, this, SLOT(loadExtensions()));

  show();
}


KstApp::~KstApp() {
/* xxx
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

// xxx  delete _dcopIface;
// xxx  _dcopIface = 0L;

  ::inst = 0L;

  if (_dataSourceConfig) {
    _dataSourceConfig->sync();
    _dataSourceConfig = 0L;
  }

  delete qSettingsObject; // must be after cleanupForExit
  qSettingsObject = 0L;
*/
}


QList<QMdiSubWindow*> KstApp::subWindowList(QMdiArea::WindowOrder order) const {
  return _mdiArea->subWindowList(order);
}


QMdiSubWindow *KstApp::activeSubWindow() const {
  return _mdiArea->activeSubWindow();
}


void KstApp::removeSubWindow(QWidget *widget) {
  _mdiArea->removeSubWindow(widget);
}


QMdiSubWindow *KstApp::addSubWindow(QWidget *widget, Qt::WindowFlags windowFlags) {
  return _mdiArea->addSubWindow(widget, windowFlags);
}


void KstApp::initDialogs() {
// xxx  _debugDialog = new KstDebugDialog(this);
// xxx  _dataManager = new KstDataManager(doc, this);
// xxx  _viewManager = new KstViewManager(doc, this);
// xxx  _viewScalarsDialog = new KstViewScalarsDialog(this);
// xxx  _viewStringsDialog = new KstViewStringsDialog(this);
// xxx  _viewVectorsDialog = new KstViewVectorsDialog(this);
// xxx  _viewMatricesDialog = new KstViewMatricesDialog(this);
// xxx  _viewFitsDialog = new KstViewFitsDialog(this);
// xxx  _changeFileDialog = new KstChangeFileDialog(this);
// xxx  _chooseColorDialog = new KstChooseColorDialog(this);
// xxx  _differentiateCurvesDialog = new KstCurveDifferentiate(this);
// xxx  _changeNptsDialog = new KstChangeNptsDialog(this);
// xxx  _graphFileDialog = new KstGraphFileDialog(this);
// xxx  _vectorSaveDialog = new KstVectorSaveDialog(this);
// xxx  _monochromeDialog = new KstMonochromeDialog(this);
// xxx  _quickStartDialog = new KstQuickStartDialog(this, 0 , true);
}


void KstApp::initStatusBar() {
/* xxx
  _dataNotifier = new KstDataNotifier(statusBar());
  statusBar()->addWidget(_dataNotifier, 0, true);
*/
  _dataBar = new StatusLabel(QString::null, statusBar());
  _dataBar->setTextFormat(Qt::PlainText);
  statusBar()->addPermanentWidget(_dataBar, 5);

  _readyBar = new StatusLabel(QObject::tr("Almost Ready"), statusBar());
  _readyBar->setTextFormat(Qt::PlainText);
  _readyBar->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusBar()->addPermanentWidget(_readyBar, 5);

  _progressBar = new QProgressBar(statusBar());
  _progressBar->setTextVisible(true);
  _progressBar->setMaximumHeight( fontMetrics().height() );
  _progressBar->hide();
  statusBar()->addPermanentWidget(_progressBar, 2);
/* xxx
#ifdef HAVE_LINUX
  _memoryBar = new StatusLabel(QObject::tr("0 MB available"), statusBar());
  _memoryBar->setTextFormat(Qt::PlainText);
  statusBar()->addWidget(_memoryBar, 0, true);
  connect(&_memTimer, SIGNAL(timeout()), this, SLOT(updateMemoryStatus()));
  _memTimer.start(5000);
#endif
*/
  slotUpdateMemoryMsg(QObject::tr("0 MB available"));
  slotUpdateStatusMsg(QObject::tr("Ready"));
  slotUpdateDataMsg(QString::null);
}


void KstApp::initToolBar() {
  _toolBar = addToolBar(QObject::tr("Kst\n"));

  if (_toolBar) {
	  _toolBar->addAction(_actionOpen);
	  _toolBar->addAction(_actionSave);
	  _toolBar->addAction(_actionPrint);
	  _toolBar->addSeparator();
  
	  _toolBar->addAction(_actionCopy);
	  _toolBar->addAction(_actionPaste);
	  _toolBar->addSeparator();
  
	  _toolBar->addAction(_actionReload);
	  _toolBar->addSeparator();
  
	  _toolBar->addAction(_actionDialogGraphFile);
	  _toolBar->addSeparator();
  
	  _toolBar->addAction(_actionDataWizard);
	  _toolBar->addAction(_actionManagerData);
	  _toolBar->addAction(_actionManagerView);
	  _toolBar->addAction(_actionDialogChangeFile);
	  _toolBar->addAction(_actionDialogChangeNpts);
	  _toolBar->addSeparator();
  
	  _toolBar->addAction(_actionSamplesDown);
	  _toolBar->addAction(_actionSamplesUp);
	  _toolBar->addAction(_actionPause);
	  _toolBar->addAction(_actionSamplesFromEnd); 
	  _toolBar->addSeparator();
  
	  _toolBar->addAction(_actionTiedZoom);
	  _toolBar->addAction(_actionDataMode);

    _mouseModeMenu = new QMenu(this);
    if (_mouseModeMenu) {
      _mouseModeMenu->addAction(_actionZoomXY);
      _mouseModeMenu->addAction(_actionZoomX);
      _mouseModeMenu->addAction(_actionZoomY);
      _mouseModeMenu->addSeparator();
      _mouseModeMenu->addAction(_actionLayout);
      _mouseModeMenu->addSeparator();
      _mouseModeMenu->addAction(_actionGfxLabel);
      _mouseModeMenu->addAction(_actionGfxRectangle);
      _mouseModeMenu->addAction(_actionGfxEllipse);
      _mouseModeMenu->addAction(_actionGfxLine);
      _mouseModeMenu->addAction(_actionGfxArrow);
      _mouseModeMenu->addAction(_actionGfxPicture);
      _mouseModeMenu->addAction(_actionGfx2DPlot);
      _mouseModeMenu->addAction(_actionGfxLegend);
    
      _mouseToolBarMenu = new QToolButton(this);
      if (_mouseToolBarMenu) {
        _mouseToolBarMenu->setMenu(_mouseModeMenu);
        _mouseToolBarMenu->setPopupMode(QToolButton::InstantPopup);
        _mouseToolBarMenu->setIcon(QIcon((":/kst_zoomxy.png")));

        _toolBar->addWidget(_mouseToolBarMenu);
      }
    }
  }
}


void KstApp::initMenuBar() {
  _actionGroupMouse = new QActionGroup(this);

  _actionGroupMouse->addAction(_actionZoomX);
  _actionGroupMouse->addAction(_actionZoomY);
  _actionGroupMouse->addAction(_actionZoomXY);
  _actionGroupMouse->addAction(_actionLayout);
  _actionGroupMouse->addAction(_actionGfxArrow);
  _actionGroupMouse->addAction(_actionGfxRectangle);
  _actionGroupMouse->addAction(_actionGfxEllipse);
  _actionGroupMouse->addAction(_actionGfxLine);
  _actionGroupMouse->addAction(_actionGfxLabel);
  _actionGroupMouse->addAction(_actionGfxPicture);
  _actionGroupMouse->addAction(_actionGfx2DPlot);
  _actionGroupMouse->addAction(_actionGfxLegend);

  _actionZoomXY->setChecked(true);

  //
  // define the standard icons...
  //

  _actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  _actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
  _actionQuit->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));

  _actionSamplesDown->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
  _actionSamplesUp->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
  _actionSamplesFromEnd->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
  _actionPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

  //
  // define the non-standard icons...
  //

  _actionReload->setIcon(QIcon((":/kst_reload.png")));
  _actionCopy->setIcon(QIcon((":/kst_copy.png")));
  _actionPaste->setIcon(QIcon((":/kst_paste.png")));
  _actionPrint->setIcon(QIcon((":/kst_print.png")));
  _actionDialogGraphFile->setIcon(QIcon((":/kst_graphfile.png")));

  _actionNewPlot->setIcon(QIcon((":/kst_newplot.png")));
  _actionZoomX->setIcon(QIcon((":/kst_zoomx.png")));
  _actionZoomY->setIcon(QIcon((":/kst_zoomy.png")));
  _actionZoomXY->setIcon(QIcon((":/kst_zoomxy.png")));
  _actionLayout->setIcon(QIcon((":/kst_layoutmode.png")));
  _actionTiedZoom->setIcon(QIcon((":/kst_zoomtie.png")));
  _actionDataMode->setIcon(QIcon((":/kst_datamode.png")));

  _actionGfxArrow->setIcon(QIcon((":/kst_gfx_arrow.png")));
  _actionGfxRectangle->setIcon(QIcon((":/kst_gfx_rectangle.png")));
  _actionGfxEllipse->setIcon(QIcon((":/kst_gfx_ellipse.png")));
  _actionGfxLine->setIcon(QIcon((":/kst_gfx_line.png")));
  _actionGfxLabel->setIcon(QIcon((":/kst_gfx_label.png")));
  _actionGfxPicture->setIcon(QIcon((":/kst_gfx_picture.png")));
  _actionGfx2DPlot->setIcon(QIcon((":/kst_newplot.png")));
  _actionGfxLegend->setIcon(QIcon((":/kst_gfx_legend.png")));

  _actionNewVector->setIcon(QIcon((":/kst_vectornew.png")));
  _actionNewCurve->setIcon(QIcon((":/kst_curvenew.png")));
  _actionNewEquation->setIcon(QIcon((":/kst_equationnew.png")));
  _actionNewHistogram->setIcon(QIcon((":/kst_histogramnew.png")));
  _actionNewSpectrum->setIcon(QIcon((":/kst_psdnew.png")));
  _actionNewPlugin->setIcon(QIcon((":/kst_pluginnew.png")));
  _actionNewEventMonitor->setIcon(QIcon((":/kst_eventnew.png")));
  _actionNewMatrix->setIcon(QIcon((":/kst_matrixnew.png")));
  _actionNewImage->setIcon(QIcon((":/kst_imagenew.png")));
  _actionNewSpectrogram->setIcon(QIcon((":/kst_csdnew.png")));

  _actionDataWizard->setIcon(QIcon((":/kst_datawizard.png")));
  _actionManagerData->setIcon(QIcon((":/kst_datamanager.png")));
  _actionManagerView->setIcon(QIcon((":/kst_viewmanager.png")));
  _actionDialogChangeFile->setIcon(QIcon((":/kst_changefile.png")));
  _actionDialogChangeNpts->setIcon(QIcon((":/kst_changenpts.png")));
  _actionDialogDifferentiateCurves->setIcon(QIcon((":/kst_differentiatecurves.png")));
  _actionDialogChooseColor->setIcon(QIcon((":/kst_choosecolor.png")));
}


void KstApp::initActions() {
  //
  // setup the connections...
  //

  connect(_actionOpen, SIGNAL(triggered()), this, SLOT(slotFileOpen()));
// xxx  connect(_actionOpenRecent, SIGNAL(triggered()), this, SLOT(slotFileOpenRecent(const QUrl &));
  connect(_actionSave, SIGNAL(triggered()), this, SLOT(slotFileSave()));
  connect(_actionSaveAs, SIGNAL(triggered()), this, SLOT(slotFileSaveAs()));
  connect(_actionPrint, SIGNAL(triggered()), this, SLOT(slotFilePrint()));
  connect(_actionNewTab, SIGNAL(triggered()), this, SLOT(slotFileNewWindow()));
// xxx  connect(_actionDialogGraphFile, SIGNAL(triggered()), this, SLOT(showGraphFileDialog()));
// xxx  connect(_actionVectorSave, SIGNAL(triggered()), _vectorSaveDialog, SLOT(show()));
  connect(_actionQuit, SIGNAL(triggered()), this, SLOT(slotFileClose()));

  connect(_actionCopy, SIGNAL(triggered()), this, SLOT(slotCopy()));
  connect(_actionPaste, SIGNAL(triggered()), this, SLOT(slotPaste()));
  connect(_actionReload, SIGNAL(triggered()), this, SLOT(reload()));
/* xxx
  connect(_actionManagerData, SIGNAL(triggered()), _dataManager, SLOT(show_I()));
  connect(_actionManagerView, SIGNAL(triggered()), _viewManager, SLOT(show_I())); 
  connect(_actionNewVector, SIGNAL(triggered()), KstVectorDialog::globalInstance(), SLOT(show())); 
  connect(_actionNewCurve, SIGNAL(triggered()), KstCurveDialog::globalInstance(), SLOT(show()));
  connect(_actionNewEquation, SIGNAL(triggered()), KstEqDialog::globalInstance(), SLOT(show())); 
  connect(_actionNewHistogram, SIGNAL(triggered()), KstHsDialog::globalInstance(), SLOT(show()));
  connect(_actionNewSpectrum, SIGNAL(triggered()), KstPsdDialog::globalInstance(), SLOT(show()));
  connect(_actionNewPlugin, SIGNAL(triggered()), this, SLOT(selectDataPlugin()));
  connect(_actionNewEventMonitor, SIGNAL(triggered()), KstEventMonitor::globalInstance(), SLOT(show())); 
  connect(_actionNewMatrix, SIGNAL(triggered()), KstMatrixDialog::globalInstance(), SLOT(show()));
  connect(_actionNewImage, SIGNAL(triggered()), KstImageDialog::globalInstance(), SLOT(show()));
  connect(_actionNewSpectrogram, SIGNAL(triggered()), KstCsdDialog::globalInstance(), SLOT(show()));
*/
  connect(_actionViewScalars, SIGNAL(triggered()), this, SLOT(showViewScalarsDialog()));
  connect(_actionViewVectors, SIGNAL(triggered()), this, SLOT(showViewVectorsDialog()));
  connect(_actionViewMatrices, SIGNAL(triggered()), this, SLOT(showViewMatricesDialog()));
  connect(_actionViewFits, SIGNAL(triggered()), this, SLOT(showViewFitsDialog()));
  connect(_actionViewStrings, SIGNAL(triggered()), this, SLOT(showViewStringsDialog()));

  connect(_actionSamplesDown, SIGNAL(triggered()), this, SLOT(samplesDown()));
  connect(_actionSamplesUp, SIGNAL(triggered()), this, SLOT(samplesUp()));
  connect(_actionSamplesFromEnd, SIGNAL(triggered()), this, SLOT(fromEnd()));
  connect(_actionPause, SIGNAL(toggled(bool)), this, SLOT(updatePausedState(bool)));

  connect(_actionDataWizard, SIGNAL(triggered()), this, SLOT(showDataWizard()));
  connect(_actionDialogChangeFile, SIGNAL(triggered()), this, SLOT(showChangeFileDialog()));
  connect(_actionDialogChooseColor, SIGNAL(triggered()), this, SLOT(showChooseColorDialog()));
  connect(_actionDialogDifferentiateCurves, SIGNAL(triggered()), this, SLOT(showDifferentiateCurvesDialog()));
  connect(_actionDialogChangeNpts, SIGNAL(triggered()), this, SLOT(showChangeNptsDialog()));

  connect(_actionManagerExtension, SIGNAL(triggered()), this, SLOT(showExtensionManager()));
  connect(_actionStatusBar, SIGNAL(toggled(bool)), this, SLOT(slotViewStatusBar()));
  connect(_actionToolBar, SIGNAL(toggled(bool)), this, SLOT(slotViewToolBar()));

  connect(_actionNewPlot, SIGNAL(triggered()), this, SLOT(newPlot()));
  connect(_actionTiedZoom, SIGNAL(toggled(bool)), this, SLOT(tieAll()));
  connect(_actionGroupMouse, SIGNAL(triggered(QAction*)), this, SLOT(toggleMouseMode()));
  connect(_actionDataMode, SIGNAL(toggled(bool)), this, SLOT(toggleDataMode()));

/* xxx
  fileKeyBindings = KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());

  fileKeyBindings->setWhatsThis(QObject::tr("Bring up a dialog box to configure shortcuts."));

  filePreferences = KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection());
  filePreferences->setWhatsThis(QObject::tr("Bring up a dialog box to configure Kst settings."));
*/
}


void KstApp::initDocument() {
  _doc = new KstDoc(this);

  QTimer::singleShot(0, this, SLOT(delayedDocInit()));
}


void KstApp::delayedDocInit() {
  if (!activeSubWindow()) {
    _doc->newDocument();
  }
}


QSize KstApp::sizeHint() const {
  QSize size;
  QRect rect;

  rect = QApplication::desktop()->geometry();

  size.setWidth(3*rect.width()/4);
  size.setHeight(3*rect.height()/4);

  return size;
}


void KstApp::initialize() {
/* xxx
  KstSettings::checkUpdates();
  qSettingsObject = new QSettings("kstdatarc", QSettings::NativeFormat, this);
  KstDataSource::setupOnStartup(qSettingsObject);
  // NOTE: This is leaked in commandline mode if we never create a KstApp.
  //       Not too much of a problem right now, and less messy than hooking in
  //       cleanups in main.
*/
}


void KstApp::loadExtensions() {
/* xxx
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
*/
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
/* xxx
  if (pEvent->type() == KstELOGAliveEvent) {
    KstEventMonitor::globalInstance()->enableELOG();
  } else if (pEvent->type() == KstELOGDeathEvent) {
    KstEventMonitor::globalInstance()->disableELOG();
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
      KstViewWindow *viewWindow;

      viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
      if (viewWindow) {
        vw->immediatePrintToPng(pStream, size);
      }
    }
  }
*/
}


void KstApp::slotConfigureKeys() {
/* xxx
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
*/
}


void KstApp::waitForUpdate() const {
  _updateThread->waitForUpdate();
}


bool KstApp::paused() const {
  return _updateThread->paused();
}


void KstApp::setPaused(bool in_paused) {
  _actionPause->setChecked(in_paused);
  _updateThread->setPaused(in_paused);
}


void KstApp::togglePaused() {
  setPaused(!_actionPause->isChecked());
}


KstApp::KstZoomType KstApp::getZoomRadio() {
  if (_actionZoomXY->isChecked()) {
    return XYZOOM;
  } else if (_actionZoomX->isChecked()) {
    return XZOOM;
  } else if (_actionZoomY->isChecked()) {
    return YZOOM;
  } else if (_actionLayout->isChecked()) {
    return LAYOUT;
  } else {
    return GRAPHICS;
  }
}


void KstApp::selectDataPlugin() {
  const KstPluginInfoList pluginInfo = KstDataObject::pluginInfoList();
  const QMap<QString,QString> readable = PluginCollection::self()->readableNameList();
  QStringList l;
  QStringList dataObjectPlugins;
  QStringList cPlugins;

  //
  // the KstDataObject plugins...
  //
 
  {
    KstPluginInfoList::const_iterator it;

    for (it = pluginInfo.begin(); it != pluginInfo.end(); ++it) {
      dataObjectPlugins << it.key();
    }
  }

  l += dataObjectPlugins;

  //
  // the C-style plugins...
  //

  {
    QMap<QString,QString>::const_iterator it = readable.begin();
    for (; it != readable.end(); ++it) {
      cPlugins << it.key();
    }
  }

  l += cPlugins;

  //
  // list the KstDataObject and C-style plugins together 
  //  in ascending alphabetical order...
  //

  l.sort();

  bool ok = false;
/* xxx
  QStringList plugin = QInputDialog::getItemList(QObject::tr("Data Plugins"), QObject::tr("Create..."), l, 0, false, &ok, this);

  if (ok && !plugin.isEmpty()) {
    const QString p = plugin.join("");

    if (dataObjectPlugins.contains(p)) {
      KstDataObjectPtr ptr;

      ptr = KstDataObject::plugin(p);
      ptr->showDialog(true);
    } else if (cPlugins.contains(p)) {
      KstPluginDialog::globalInstance()->showNew(readable[p]);
    }
  }
*/
}


void KstApp::forceUpdate() {
  _updateThread->forceUpdate();
}


void KstApp::addRecentFile(const QUrl &url) {
// xxx  _recent->addURL(url);
}


void KstApp::selectRecentFile(const QUrl &url) {
/* xxx
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
*/
}


void KstApp::doDelayedOpens() {
  QList<KstOpen> queueCopy;
  QList<KstOpen>::const_iterator i;
  
  queueCopy = _openQueue;
  _openQueue.clear();

  for (i = queueCopy.begin(); i != queueCopy.end(); ++i) {
    openDocumentFile((*i).filename, (*i).file, (*i).n, (*i).f, (*i).s, (*i).ave);
  }
}


bool KstApp::openDocumentFile(const QString& in_filename,
    const QString& o_file, int o_n, int o_f, int o_s, bool o_ave, bool delayed) {
  static bool opening = false;
  QUrl url;
  bool rc = false;

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

  if (QFile::exists(in_filename) && QFileInfo(in_filename).isRelative()) {
    url.setPath(QFileInfo(in_filename).absoluteFilePath());
  } else {
// xxx    url = KURL::fromPathOrURL(in_filename);
  }

  slotUpdateStatusMsg(QObject::tr("Opening %1...").arg(url.path()));

  if (_doc->openDocument(url, o_file, o_n, o_f, o_s, o_ave)) {
    setWindowTitle(_doc->title());
/* xxx
    if (url.isLocalFile()) {
      QFileInfo finfo(in_filename);
      QString fileExport;

      fileExport = finfo.absoluteFilePath();
      addRecentFile(fileExport);

      if (fileExport.endsWith(QString(".kst"), Qt::CaseInsensitive)) {
        fileExport.truncate(fileExport.length() - QString(".kst").length());
      }
      graphFileDlg()->setURL(fileExport);
    } else {
      addRecentFile(url);
    }
*/
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
  _config->beginGroup("General Options");
// xxx  _recent->saveEntries(_config, "Recent Files");

  KST::legendDefaults.writeConfig(_config);
  KST::vectorDefaults.writeConfig(_config);
  KST::matrixDefaults.writeConfig(_config);
  KST::objectDefaults.writeConfig(_config);

  _config->endGroup();
  _config->sync();
}


void KstApp::readOptions() {
  _config->beginGroup("General Options");
// xxx  _recent->loadEntries(config, "Recent Files");

  KST::legendDefaults.readConfig(_config);
  KST::vectorDefaults.readConfig(_config);
  KST::matrixDefaults.readConfig(_config);
  KST::objectDefaults.readConfig(_config);

  _config->endGroup();
}


void KstApp::saveProperties(QSettings *config) {
  QString name;

  name = _doc->absFilePath();
  if (!name.isEmpty() && _doc->title() != "Untitled") {
    config->setValue("Document", name);
    config->setValue("NamedDocument", true);
  } else {
    QString sl;
    int i = 0;

// xxx    sl = KGlobal::dirs()->saveLocation("kst", "kst/");

    do {
      name = sl + QString("unsaved%1.kst").arg(i);
    } while(QFile::exists(name));

    _doc->saveDocument(name, false, false);
    config->setValue("Document", name);
    config->setValue("NamedDocument", false);
  }
}


void KstApp::readProperties(QSettings* config) {
  QString name = config->value("Document").toString();

  if (!name.isEmpty()) {
    if (config->value("NamedDocument", false).toBool()) {
      _doc->openDocument(name);
    } else {
      _doc->openDocument(name);
      QFile::remove(name);
      _doc->setTitle("Untitled");
    }
  }
}


bool KstApp::queryClose() {
  if (_doc->saveModified()) {
     _doc->cancelUpdate();
    _stopping = true;

    QTimer::singleShot(0, _doc, SLOT(deleteContents()));

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

  if (_doc->saveModified()) {
    _doc->newDocument();
    setWindowTitle(_doc->title());
    selectRecentFile(QUrl(""));
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
    setWindowTitle(/* xxx kapp->caption() + */ ": " + _doc->title());
  }

  slotUpdateStatusMsg(QObject::tr("Ready"));

  return returnVal;
}


void KstApp::slotFileSave() {
  if (_doc->title() == "Untitled") {
    slotFileSaveAs();
  } else {
    slotUpdateStatusMsg(QObject::tr("Saving file..."));
    _doc->saveDocument(_doc->absFilePath(), false, false);
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
      folder = _doc->lastFilePath();
    }

    newName = QFileDialog::getSaveFileName(this, QObject::tr("Save As"),
                  folder, QObject::tr("*.kst|Kst Plot File (*.kst)\n*|All Files"));
    if (!newName.isEmpty()) {
      QRegExp extension("*.kst", Qt::CaseInsensitive, QRegExp::Wildcard);
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

    viewWindow->immediatePrintToEps(filename, size);
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
    QPrintDialog printerdlg(&printer);
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
*/
    slotUpdateStatusMsg(QObject::tr("Ready"));
  } else {
    slotUpdateStatusMsg(QObject::tr("Nothing to print"));
  }
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
    statusBar()->hide();
  } else {
    statusBar()->show();
    updateStatusBarText();
  }
}


void KstApp::slotViewToolBar() {
  if (_actionToolBar->isChecked()) {
    _toolBar->hide();
  } else {
    _toolBar->show();
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
  if (_memoryBar) {
    _memoryBar->setFullText( msg );
    updateStatusBarText();
  }
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
    if (_mouseToolBarMenu) {
      _mouseToolBarMenu->setText(action->toolTip());
      _mouseToolBarMenu->setIcon(action->icon());
    }
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
  Kst2DPlotList::const_iterator cit;
  Kst2DPlotList::iterator it;
  Kst2DPlotList pl;
  int tied = 0;

  pl = Kst2DPlot::globalPlotList();
  for (cit = pl.begin(); cit != pl.end(); ++cit) {
    if ((*cit)->isTied()) {
      ++tied;
    } else {
      --tied;
    }
  }

  for (it = pl.begin(); it != pl.end(); ++it) {
    (*it)->setTied(tied <= 0);
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
// xxx  _viewScalarsDialog->showViewScalarsDialog();
}


void KstApp::showViewStringsDialog() {
// xxx  _viewStringsDialog->showViewStringsDialog();
}


void KstApp::showViewVectorsDialog() {
// xxx  _viewVectorsDialog->showViewVectorsDialog();
}


void KstApp::showViewVectorsDialog(const QString &vector) {
// xxx  _viewVectorsDialog->showViewVectorsDialog(vector);
}


void KstApp::showViewMatricesDialog() {
// xxx  _viewMatricesDialog->showViewMatricesDialog();
}


void KstApp::showViewMatricesDialog(const QString &matrix) {
// xxx  _viewMatricesDialog->showViewMatricesDialog(matrix);
}


void KstApp::showViewFitsDialog() {
// xxx  _viewFitsDialog->showViewFitsDialog();
}


void KstApp::showChangeFileDialog() {
// xxx  _changeFileDialog->showChangeFileDialog();
}


void KstApp::showChooseColorDialog() {
// xxx  _chooseColorDialog->showChooseColorDialog();
}


void KstApp::showDifferentiateCurvesDialog() {
// xxx  _differentiateCurvesDialog->showCurveDifferentiate();
}


void KstApp::showChangeNptsDialog() {
// xxx  _changeNptsDialog->showChangeNptsDialog();
}


void KstApp::showGraphFileDialog() {
// xxx  _graphFileDialog->show_I();
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
/* xxx
  if (_dataNotifier) {
    _dataNotifier->arrived();
  }
*/
}


void KstApp::updateDataDialogs(bool dm, bool vm) {
/* xxx
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
*/
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
/* xxx
    KstCsdDialog::globalInstance()->updateWindow();
    KstEqDialog::globalInstance()->updateWindow();
    KstHsDialog::globalInstance()->updateWindow();
    KstVvDialog::globalInstance()->updateWindow();
    KstPsdDialog::globalInstance()->updateWindow();
    KstCurveDialog::globalInstance()->updateWindow();
    KstImageDialog::globalInstance()->updateWindow();
    updateDataManager(false);
    updateViewManager(false);
*/
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
/* xxx
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
    KstViewWindow *viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
    if (viewWindow) {
      QApplication::clipboard()->setData(vw->view()->widget()->dragObject(), QClipboard::Clipboard);
    }
  }
*/
}


void KstApp::slotPaste() {
/* xxx
  if (_actionLayout->isChecked()) {
    KstTopLevelViewPtr tlv;

    tlv = activeView();
    if (tlv) {
      QMimeSource* source;

      source = QApplication::clipboard()->data(QClipboard::Clipboard);
      if (!tlv->paste(source)) {
        KstDebug::self()->log(QObject::tr("Paste operation failed: clipboard data was not found or of the wrong format."));
      }
    } else {
      KstDebug::self()->log(QObject::tr("Paste operation failed: there is currently no active view."));
    }
  } else {
    KstDebug::self()->log(QObject::tr("Paste operation failed: must be in layout mode."));
  }
*/
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
  KstViewWindow *viewWindow;

  viewWindow = dynamic_cast<KstViewWindow*>(activeSubWindow());
  if (viewWindow) {
    tlv = viewWindow->view();
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
  KstViewWindow *viewWindow;
  QAction *action;
  QMenu *pm;

  pm = new QMenu(this);

  viewWindow = dynamic_cast<KstViewWindow*>(w);
  if (viewWindow) {
    pm->setTitle(viewWindow->windowTitle());
  }

  pm->addAction(QObject::tr("&New..."), this, SLOT(slotFileNewWindow()));
  if (viewWindow) {
    action = pm->addAction(QObject::tr("Move &Left"), viewWindow, SLOT(moveTabLeft()));
// xxx    action->setEnabled(tw->indexOf(w) > 0);
    action = pm->addAction(QObject::tr("Move &Right"), viewWindow, SLOT(moveTabRight()));
// xxx    action->setEnabled(tw->indexOf(w) < tw->count() - 1);

    pm->addAction(QObject::tr("R&ename..."), viewWindow, SLOT(rename()));
    pm->addAction(QObject::tr("&Close"), viewWindow, SLOT(close()));
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
  QList<QMdiSubWindow*> windows;
  QList<QMdiSubWindow*>::const_iterator i;

  windows = subWindowList(QMdiArea::StackingOrder);

  for (i = windows.constBegin(); i != windows.constEnd(); ++i) {
    KstViewWindow *viewWindow;

    viewWindow = dynamic_cast<KstViewWindow*>(*i);
    if (viewWindow) {
      ts << "  <window>" << endl;
      viewWindow->save(ts, "    ");
      ts << "  </window>" << endl;
    }
  }
}


void KstApp::emitTimezoneChanged(const QString& tz, int utcOffset) {
  emit timezoneChanged(tz, utcOffset);
}

#include "kst.moc"

