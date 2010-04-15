/***************************************************************************
                          kst.h  -  description
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

#ifndef KST_H
#define KST_H

#include <config.h>

#include <QActionGroup>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPointer>
#include <QProgressBar>
#include <QSettings>
#include <QTimer>

#include "kst_export.h"
#include "kstpainter.h"
#include "ksttoplevelview.h"

enum LegendType { LegendOn, LegendOff, LegendAuto };
enum DataType { DataOnly, DataPSD, PSDOnly };
enum LineType { LineOnly, PointOnly, LinePoint };
enum PlotType { OnePlot, MultiplePlots, CycleThrough, ExistingPlot, ExistingCycle};

// forward declaration of the Kst classes
class GraphicEditor;
class Kst2DPlot;
typedef KstObjectMap<QExplicitlySharedDataPointer<Kst2DPlot> > Kst2DPlotMap;
class KstChangeFileDialog;
class KstChangeNptsDialog;
class KstChooseColorDialog;
class KstCurveDifferentiate;
// xxx class KstDataManager;
class KstDataNotifier;
class KstDebugDialog;
class KstDebugNotifier;
class KstDoc;
class KstEventMonitorEntry;
class KstEventMonitor;
class KstGraphFileDialog;
class KstIfaceImpl;
class KstMonochromeDialog;
class KstPlotDialog;
class KstQuickStartDialog;
class KstVectorSaveDialog;
class KstViewFitsDialog;
// xxx class KstViewManager;
class KstViewMatricesDialog;
class KstViewScalarsDialog;
class KstViewStringsDialog;
class KstViewVectorsDialog;
class KstViewWindow;
class StatusLabel;
class UpdateThread;

class QLabel;
class QProgressBar;
class KProgress;


class KstOpen {
  public:
    QString filename, file;
    int n, f, s;
    bool ave;
};

class KST_EXPORT KstApp : public QMdiArea {
  Q_OBJECT
  public:
    static void initialize(); // for main to call

    KstApp(QWidget* parent=0, const char* name=0);
    ~KstApp();

    virtual QSize sizeHint() const;

    static KstApp* inst();
    static void doubleCheckCleanup();

    void checkFontPresent(const QString& font);
    const QString& defaultFont() const;

    /** add a file to the recent file list */
    void addRecentFile(const KUrl &file);
    void selectRecentFile(const KUrl &file);

    /** opens a file specified by commandline option */
    bool openDocumentFile(const QString& _cmdl = QString::null,
        const QString& o_file = "|", int o_n = -2, int o_f = -2,
        int o_s = -1, bool o_ave = false, bool delayed = false);

    /** returns a pointer to the current document connected to the
     * KMainWindow instance and is used by the View class to access
     * the document object's methods */
    KstDoc *document() const;

    /** returns a pointer to the monochrome settings dialog */

    /** pause the updating of data */
    void setPaused(bool paused);
    void togglePaused();

    enum KstZoomType { XYZOOM, YZOOM, XZOOM, TEXT, GRAPHICS, LAYOUT };
    enum KstGraphicType { GFX_LINE, GFX_ARROW, GFX_POLYGON, GFX_POLYLINE, GFX_ELLIPSE, GFX_RECTANGLE, GFX_ROUNDED_RECTANGLE, GFX_CURVE, GFX_LABEL };
    /** Get XY zoom radio button state */
    KstZoomType getZoomRadio();
    KstGraphicType getGraphicType();
    bool saveData() const { return _actionSaveData->isChecked(); }

    KstTopLevelView::ViewMode currentViewMode();
    QString currentCreateType();

    KstTopLevelViewPtr activeView();
    Kst2DPlotMap* plotHolderWhileOpeningDocument();

    QString windowName(bool prompt, const QString& nameOriginal, bool rename = false, QWidget *parent = 0L);
    QString newWindow(const QString& name);
    QString newWindow(bool prompt = true, QWidget *parent=0L);
    KstViewWindow* findWindow(const QString& title);
    void renameWindow(KstViewWindow *);
    void tiedZoomPrev(KstViewWidget* view, const QString& plotName);
    void tiedZoomMode(int zoom, bool flag, double center, int mode, int modeExtra, KstViewWidget* view, const QString& plotName);
    void tiedZoom(bool x, double xmin, double xmax, bool y, double ymin, double ymax, KstViewWidget* view, const QString& plotName);

    bool dataMode() const;

    void paintAll(KstPainter::PaintType = KstPainter::P_PAINT);
    void paintAllFromScript();

    void EventELOGSubmitEntry(const QString& message);

    const QStringList recentFiles() const;

    QSettings *dataSourceConfig() const { return _dataSourceConfig; }

    KstGraphFileDialog *graphFileDlg() const { return _graphFileDialog; }
    KstChooseColorDialog *chooseColorDlg() const { return _chooseColorDialog; }
    KstMonochromeDialog *monochromeDialog() const { return _monochromeDialog; }

    void waitForUpdate() const;
    bool paused() const;

    void moveTabLeft(KstViewWindow*);
    void moveTabRight(KstViewWindow*);
    void saveTabs(QTextStream& ts);

  protected:
    void customEvent(QEvent *e);

    /** save options to the configuration file
     *  Geometry, Toolbar status, Statusbar status */
    void saveOptions();

    /** read options from configuration file
     *  Geometry, Toolbar status, Statusbar status */
    void readOptions();

    /** setup kde2 actions and build the GUI */
    void initActions();

    /** sets up the statusbar for the main window */
    void initStatusBar();

    /** initializes the document object */
    void initDocument();

    /** queryClose is called by KMainWindow on each closeEvent of a
     * window. This calles saveModified() on the document object to ask
     * if the document should be saved if Modified; on cancel the
     * closeEvent is rejected.
     * @see saveModified()
     * @see KMainWindow#queryClose
     * @see KMainWindow#closeEvent */
    bool queryClose();

    /** Calls queryClose */
    bool queryExit();

    /** saves the window properties for each open window during session
     * end to the session config file, including saving the currently
     * opened file by a temporary filename provided by KApplication.
     * @see KMainWindow#saveProperties */
    void saveProperties(QSettings *cfg);

    /** reads the session config file and restores the application's
     * state including the last opened files and documents by reading
     * the temporary files saved by saveProperties()
     * @see KMainWindow#readProperties */
    void readProperties(QSettings *cfg);

  private slots:
    // Hack to update KStdActions
    void updateActions();
    void loadExtensions();
    void toggleDataMode();
    void toggleMouseMode();
    void slotSettingsChanged();
    void showContextMenu(QWidget *w, const QPoint& pos);
    void showContextMenu(const QPoint& pos);
    void delayedDocInit();
    void selectDataPlugin();

  public slots:
    void fromEnd();
    void updatePausedState(bool);
    void tieAll();
    void forceUpdate();
    void slotConfigureKeys();
    void slotFileNewWindow(QWidget *parent = 0L);
    void slotFileRenameWindow();
    void slotFileNew();
    void slotFileOpen();
    bool slotFileOpenRecent(const QUrl &);
    void slotFileSave();
    bool slotFileSaveAs();
    void slotFileClose();
    void slotFileQuit();
    void slotFilePrint();

    /** print without querying */
    void immediatePrintToFile(const QString& filename, bool revert = true);
    void immediatePrintWindowToFile(QMdiSubWindow *window, const QString& filename);
    void immediatePrintActiveWindowToFile(const QString& filename);

    /** export to png without querying */
    void immediatePrintToPng(const QString& filename, const QString& format = "PNG", int width = 640, int height = 480, bool all = false, int display = 0);
    void immediatePrintWindowToPng(QMdiSubWindow *window, const QString& filename, const QString& format = "PNG", int width = 640, int height = 480, int display = 0);
    void immediatePrintActiveWindowToPng(const QString& filename, const QString& format = "PNG", int width = 640, int height = 480, int display = 0);

    /** export to eps without querying */
    void immediatePrintToEps(const QString& filename, int width = 640, int height = 480, bool all=false, int display = 0);
    void immediatePrintWindowToEps(QMdiSubWindow *win, const QString& filename, int width, int height, int display);    
    void immediatePrintActiveWindowToEps(const QString& filename, int width, int height, int display);

    //void slotCut() {}
    void slotCopy();
    void slotPaste();

    /** toggles the statusbar */
    void slotViewStatusBar();

    /** changes the statusbar contents */
    void updateStatusBarText();
    void slotUpdateStatusMsg(const QString &msg);
    void slotUpdateDataMsg(const QString &msg);
    void slotUpdateMemoryMsg(const QString &msg);
    void slotUpdateProgress(int total, int step, const QString &msg);

    void newPlot();

    void showDataManager();
    void showViewManager();
    void showChangeFileDialog();
    void showMonochromeDialog();
    void showChooseColorDialog();
    void showDifferentiateCurvesDialog();
    void showViewScalarsDialog();
    void showViewStringsDialog();
    void showViewVectorsDialog();
    void showViewVectorsDialog(const QString &vector);
    void showViewMatricesDialog();
    void showViewMatricesDialog(const QString& matrix);
    void showViewFitsDialog();
    void showChangeNptsDialog();
    void showGraphFileDialog();
    void showDebugDialog();
    void showDebugLog();
    void showPluginManager();
    void showExtensionManager();
    void showDataWizardWithFile(const QString &input);
    void showDataWizard();
    void showQuickStartDialog();

    void samplesDown();
    void samplesUp();

    void updateVisibleDialogs();
    void updateDialogs(bool onlyVisible = true);
    void updateDataDialogs(bool dataManager = true, bool viewManager = true);
    void updateDialogsForWindow();
    void updateDialogsForPlot();
    void updateDataNotifier();
    void updateDataManager(bool onlyVisible);
    void updateViewManager(bool onlyVisible);

    void registerDocChange();
    void reload();
    void slotPreferences();
    void EventELOGConfigure();
    void createDebugNotifier();
    void destroyDebugNotifier();
    void emitTimezoneChanged(const QString& tz, int utcOffset);
    void setEnableImplicitRepaintsFromScript(bool enable);
    bool getEnableImplicitRepaintsFromScript();

  signals:
    void timezoneChanged(const QString& tz, int utcOffset);
    void settingsChanged();
    void ELOGConfigure();
    void ELOGSubmitEntry(const QString& strMessage);

  private slots:
    void updateMemoryStatus();
    void doDelayedOpens();

  private:
    static const QString& defaultTag;

    QSettings *_config;
    KstDoc *_doc;

    KstViewScalarsDialog *_viewScalarsDialog;
    KstViewStringsDialog *_viewStringsDialog;
    KstViewVectorsDialog *_viewVectorsDialog;
    KstViewMatricesDialog *_viewMatricesDialog;
    KstViewFitsDialog *_viewFitsDialog;
    KstChangeFileDialog *_changeFileDialog;
    KstChooseColorDialog *_chooseColorDialog;
    KstCurveDifferentiate *_differentiateCurvesDialog;
    KstChangeNptsDialog *_changeNptsDialog;
    KstGraphFileDialog *_graphFileDialog;
    KstVectorSaveDialog *_vectorSaveDialog;
// xxx    KstDataManager *_dataManager;
// xxx    KstViewManager *_viewManager;
    KstDebugDialog *_debugDialog;
    GraphicEditor *_graphicDialog;
    KstMonochromeDialog *_monochromeDialog; 
    KstQuickStartDialog *_quickStartDialog;
/* xxx
    KRecentFilesAction *_recent;
    KRecentFilesAction *_fileOpenRecent;
*/
    QAction *_actionStatusBar;
    QAction *_actionPause;
    QAction *_actionDataMode;
    QAction *_actionSaveData;

    QAction *_actionSamplesDown;
    QAction *_actionSamplesUp;
    QAction *_actionSamplesFromEnd;
    QAction *_actionTiedZoom;
    QAction *_actionReload;
    QAction *_actionMatrixDialog;
    QAction *_actionImageDialog;
    QAction *_actionNewPlot;
    QAction *_actionPlotDialog;
    QAction *_actionVectorDialog;
    QAction *_actionCurveDialog;
    QAction *_actionCsdDialog;
    QAction *_actionEqDialog;
    QAction *_actionHsDialog;
    QAction *_actionVvDialog;
    QAction *_actionPsdDialog;
    QAction *_actionDataManager;  
    QAction *_actionViewManager;
    QAction *_actionVectorSave;
    QAction *_actionPluginDialog;
    QAction *_actionViewScalarsDialog;
    QAction *_actionViewStringsDialog;
    QAction *_actionViewVectorsDialog;
    QAction *_actionViewMatricesDialog;
    QAction *_actionViewFitsDialog;
    QAction *_actionDataWizard;
    QAction *_actionDebugDialog;
    QAction *_actionChangeFileDialog;
    QAction *_actionChooseColorDialog;
    QAction *_actionDifferentiateCurvesDialog;
    QAction *_actionChangeNptsDialog;
    QAction *_actionGraphFileDialog;
    QAction *_actionPluginManager;
    QAction *_actionExtensionManager;
    QAction *_actionEventMonitor;

    QAction *_actionFileNew;
    QAction *_actionFilePrint;
    QAction *_actionFileOpenNew;
    QAction *_actionFileOpen;
    QAction *_actionFileSave;
    QAction *_actionFileSaveAs;
    QAction *_actionFileQuit;
    QAction *_actionFileKeyBindings;
    QAction *_actionFilePreferences;
    QAction *_actionFileCopy;
    QAction *_actionFilePaste;

    QActionGroup *_actionGroupZoom;
    QAction *_actionZoomXY;
    QAction *_actionZoomX;
    QAction *_actionZoomY;
    QAction *_actionGfx;
    QAction *_actionLayout;

    QActionGroup *_actionGroupGfx;
    QAction *_actionGfxLine;
    QAction *_actionGfxRectangle;
    QAction *_actionGfxEllipse;
    QAction *_actionGfxLabel;
    QAction *_actionGfxPicture;
    QAction *_actionGfx2DPlot;
    QAction *_actionGfxArrow;
    QAction *_actionGfxLegend;

    StatusLabel *_readyBar;
    StatusLabel *_memoryBar;
    StatusLabel *_dataBar;
    QProgressBar *_progressBar;
    KstIfaceImpl *_dcopIface;
    UpdateThread *_updateThread;
    Kst2DPlotMap *_plotHolderWhileOpeningDocument;
    QTimer _memTimer;
    QString _defaultFont;
    QSettings *_dataSourceConfig;
    QList<KstOpen> _openQueue;
    KstGraphicType _graphicType;
    bool _updatesFromScriptEnabled;
    bool _stopping;

    KstTopLevelView::ViewMode _viewMode;
    QString _createType;
    QPointer<KstDebugNotifier> _debugNotifier;
    QPointer<KstDataNotifier> _dataNotifier;
};

#endif
