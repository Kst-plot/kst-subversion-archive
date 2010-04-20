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
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QPointer>
#include <QProgressBar>
#include <QSettings>
#include <QTimer>
#include <QToolBar>

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
/* xxx
class KstChangeFileDialog;
class KstChangeNptsDialog;
*/
class KstChooseColorDialog;
class KstCurveDifferentiate;
// xxx class KstDataManager;
// xxx class KstDataNotifier;
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
/* xxx
class KstViewFitsDialog;
class KstViewManager;
class KstViewMatricesDialog;
class KstViewScalarsDialog;
class KstViewStringsDialog;
class KstViewVectorsDialog;
*/
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

class KST_EXPORT KstApp : public QMainWindow {
  Q_OBJECT
  public:
    enum KstZoomType { XYZOOM, YZOOM, XZOOM, TEXT, GRAPHICS, LAYOUT };
    enum KstGraphicType { GFX_LINE, GFX_ARROW, GFX_POLYGON, GFX_POLYLINE, GFX_ELLIPSE, GFX_RECTANGLE, GFX_ROUNDED_RECTANGLE, GFX_CURVE, GFX_LABEL };

    KstApp(QWidget* parent=0, const char* name=0);
    ~KstApp();

    virtual QSize sizeHint() const;

    static KstApp* inst();
    static void doubleCheckCleanup();
    static void initialize();

    void checkFontPresent(const QString& font);
    const QString& defaultFont() const;

    void addRecentFile(const QUrl &file);
    void selectRecentFile(const QUrl &file);

    bool openDocumentFile(const QString& _cmdl = QString::null,
        const QString& o_file = "|", int o_n = -2, int o_f = -2,
        int o_s = -1, bool o_ave = false, bool delayed = false);
    KstDoc *document() const;
    void setPaused(bool paused);
    void togglePaused();

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

// xxx    KstGraphFileDialog *graphFileDlg() const { return _graphFileDialog; }
// xxx    KstChooseColorDialog *chooseColorDlg() const { return _chooseColorDialog; }
    KstMonochromeDialog *monochromeDialog() const { return _monochromeDialog; }

    void waitForUpdate() const;
    bool paused() const;

    void moveTabLeft(KstViewWindow*);
    void moveTabRight(KstViewWindow*);
    void saveTabs(QTextStream& ts);

  protected:
    void customEvent(QEvent *e);

    void saveOptions();
    void readOptions();
    void initActions();
    void initStatusBar();
    void initToolBar();
    void initMenuBar();
    void initDocument();
    bool queryClose();
    bool queryExit();
    void saveProperties(QSettings *cfg);
    void readProperties(QSettings *cfg);

  private slots:
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

    void immediatePrintToFile(const QString& filename, bool revert = true);
    void immediatePrintWindowToFile(QMdiSubWindow *window, const QString& filename);
    void immediatePrintActiveWindowToFile(const QString& filename);

    void immediatePrintToPng(const QString& filename, const QString& format = "PNG", int width = 640, int height = 480, bool all = false, int display = 0);
    void immediatePrintWindowToPng(QMdiSubWindow *window, const QString& filename, const QString& format = "PNG", int width = 640, int height = 480, int display = 0);
    void immediatePrintActiveWindowToPng(const QString& filename, const QString& format = "PNG", int width = 640, int height = 480, int display = 0);

    void immediatePrintToEps(const QString& filename, int width = 640, int height = 480, bool all=false, int display = 0);
    void immediatePrintWindowToEps(QMdiSubWindow *win, const QString& filename, int width, int height, int display);    
    void immediatePrintActiveWindowToEps(const QString& filename, int width, int height, int display);

    //void slotCut() {}
    void slotCopy();
    void slotPaste();

    void slotViewStatusBar();
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

    QList<QMdiSubWindow*> subWindowList(QMdiArea::WindowOrder order = QMdiArea::CreationOrder) const;
    QMdiSubWindow *activeSubWindow() const;
    void removeSubWindow(QWidget *widget);
    QMdiSubWindow *addSubWindow(QWidget *widget, Qt::WindowFlags windowFlags = 0 ); 

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
/* xxx
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
    KstDataManager *_dataManager;
    KstViewManager *_viewManager;
*/
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
    QAction *_actionNewPlot;
    QAction *_actionNewTab;
    QAction *_actionVectorSave;

    QAction *_actionDialogPlot;
    QAction *_actionDialogVector;
    QAction *_actionDialogCurveD;
    QAction *_actionDialogCsd;
    QAction *_actionDialogEq;
    QAction *_actionDialogHs;
    QAction *_actionDialogVv;
    QAction *_actionDialogPsd;
    QAction *_actionDialogMatrix;
    QAction *_actionDialogImage;
    QAction *_actionDialogPlugin;
    QAction *_actionDialogViewScalars;
    QAction *_actionDialogViewStrings;
    QAction *_actionDialogViewVectors;
    QAction *_actionDialogViewMatrices;
    QAction *_actionDialogViewFits;
    QAction *_actionDialogDebug;
    QAction *_actionDialogChangeFile;
    QAction *_actionDialogChooseColor;
    QAction *_actionDialogDifferentiateCurves;
    QAction *_actionDialogChangeNpts;
    QAction *_actionDialogGraphFile;

    QAction *_actionDataWizard;

    QAction *_actionManagerData;  
    QAction *_actionManagerView;
    QAction *_actionManagerPluginr;
    QAction *_actionManagerExtension;

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
    QToolBar *_toolBar;
    QMenuBar *_menuBar;
    QMdiArea *_mdiArea;

// xxx    KstIfaceImpl *_dcopIface;
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
// xxx    QPointer<KstDataNotifier> _dataNotifier;
};

#endif
