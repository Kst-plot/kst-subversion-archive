/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mainwindow.h"
#include "boxitem.h"
#include "datamanager.h"
#include "debugdialog.h"
#include "debugnotifier.h"
#include "document.h"
#include "ellipseitem.h"
#include "exportgraphicsdialog.h"
#include "application.h"
#include "debug.h"
#include "labelitem.h"
#include "lineitem.h"
#include "memorywidget.h"
#include "objectstore.h"
#include "pictureitem.h"
#include "plotitem.h"
#include "plotitemmanager.h"
#include "svgitem.h"
#include "tabwidget.h"
#include "ui_aboutdialog.h"
#include "vectoreditordialog.h"
#include "scalareditordialog.h"
#include "matrixeditordialog.h"
#include "stringeditordialog.h"
#include "view.h"
#include "viewmanager.h"

#include "applicationsettingsdialog.h"
#include "differentiatecurvesdialog.h"
#include "choosecolordialog.h"
#include "changedatasampledialog.h"
#include "changefiledialog.h"

#include <QtGui>

//FIXME Temporaries REMOVE!!
#include "editablevector.h"
#include "generatedvector.h"
#include "datacollection.h"
#include "dataobjectcollection.h"
#include "equation.h"

namespace Kst {

MainWindow::MainWindow() {
  _dataManager = 0;
  _exportGraphics = 0;
  _vectorEditor = 0;
  _scalarEditor = 0;
  _stringEditor = 0;
  _matrixEditor = 0;
  _viewManager = 0;
  _doc = new Document(this);
  _tabWidget = new TabWidget(this);
  _undoGroup = new QUndoGroup(this);
  _debugDialog = new DebugDialog(this); // need this early for hookups
  Debug::self()->setHandler(_debugDialog);

  connect(_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentViewChanged()));

  _tabWidget->createView();

  setCentralWidget(_tabWidget);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();

  readSettings();
  QTimer::singleShot(0, this, SLOT(performHeavyStartupActions()));
}


MainWindow::~MainWindow() {
  delete _vectorEditor;
  _vectorEditor = 0;
  delete _dataManager;
  _dataManager = 0;
  delete _viewManager;
  _viewManager = 0;
  delete _doc;
  _doc = 0;
}


void MainWindow::performHeavyStartupActions() {
}


void MainWindow::cleanup() {
  if (document() && document()->objectStore()) {
    document()->objectStore()->clear();
  }
}


void MainWindow::setLayoutMode(bool layoutMode) {
  View *v = tabWidget()->currentView();
  Q_ASSERT(v);

  if (layoutMode)
    v->setViewMode(View::Layout);
  else
    v->setViewMode(View::Data);

  //disable all layout actions
  _createLabelAct->setEnabled(layoutMode);
  _createBoxAct->setEnabled(layoutMode);
  _createEllipseAct->setEnabled(layoutMode);
  _createLineAct->setEnabled(layoutMode);
  _createPictureAct->setEnabled(layoutMode);
  _createPlotAct->setEnabled(layoutMode);
  _createSvgAct->setEnabled(layoutMode);
  _createLayoutAct->setEnabled(layoutMode);
  _breakLayoutAct->setEnabled(layoutMode);

  _layoutToolBar->setVisible(layoutMode);
}


void MainWindow::setTiedZoom(bool tiedZoom) {
  View *v = tabWidget()->currentView();
  QList<PlotItem*> plots = PlotItemManager::plotsForView(v);
  foreach (PlotItem *plotItem, plots) {
    plotItem->setTiedZoom(tiedZoom);
  }
}


bool MainWindow::promptSave() {
  int rc = QMessageBox::warning(this, tr("Kst"), tr("Your document has been modified.\nSave changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
  if (rc == QMessageBox::Save) {
    save();
  } else if (rc == QMessageBox::Cancel) {
    return false;
  }
  return true;
}


void MainWindow::closeEvent(QCloseEvent *e) {
  if (_doc->isChanged() && !promptSave()) {
    return;
  }
  cleanup();
  QMainWindow::closeEvent(e);
}


Document *MainWindow::document() const {
  return _doc;
}


QUndoGroup *MainWindow::undoGroup() const {
  return _undoGroup;
}


TabWidget *MainWindow::tabWidget() const {
  return _tabWidget;
}


void MainWindow::save() {
  if (_doc->isOpen()) {
    _doc->save();
  } else {
    saveAs();
  }
}


void MainWindow::saveAs() {
  QString fn = QFileDialog::getSaveFileName(this, tr("Kst: Save File"), _doc->fileName(), tr("Kst Sessions (*.kst)"));
  if (fn.isEmpty()) {
    return;
  }
  _doc->save(fn);
}


void MainWindow::open() {
  if (_doc->isChanged() && !promptSave()) {
    return;
  }
  QString fn = QFileDialog::getOpenFileName(this, tr("Kst: Open File"), _doc->fileName(), tr("Kst Sessions (*.kst)"));
  if (fn.isEmpty()) {
    return;
  }

  openFile(fn);
}


void MainWindow::openFile(const QString &file) {
  delete _doc;
  _doc = new Document(this);
  bool ok = _doc->open(file);
  if (!ok) {
    QMessageBox::critical(this, tr("Kst"), tr("Error opening document '%1':\n%2").arg(file, _doc->lastError()));
    delete _doc;
    _doc = new Document(this);
  }
}


void MainWindow::print() {
  if (!_doc->isOpen()) {
    return;
  }

  QPrinter printer(QPrinter::HighResolution);

  QPrintDialog pd(&printer, this);
  pd.addEnabledOption(QPrintDialog::PrintToFile);
  pd.addEnabledOption(QPrintDialog::PrintPageRange);

  if (pd.exec() == QDialog::Accepted) {
    QPainter painter(&printer);
    QList<QGraphicsView*> pages;
    switch (printer.printRange()) {
      case QPrinter::PageRange:
        break;
      case QPrinter::AllPages:
        foreach (QGraphicsView *view, _tabWidget->views()) {
          pages.append(view);
        }
        break;
      case QPrinter::Selection:
      default:
        pages.append(_tabWidget->currentView());
        break;
    }

    for (int i = 0; i < printer.numCopies(); ++i) {
      foreach (QGraphicsView *view, pages) {
        view->render(&painter, QRectF(), QRect(), Qt::KeepAspectRatio /* IgnoreAspectRatio */);
        printer.newPage();
      }
    }
  }
}


void MainWindow::currentViewChanged() {
  _undoGroup->setActiveStack(_tabWidget->currentView()->undoStack());
  _layoutModeAct->setChecked(_tabWidget->currentView()->viewMode() == View::Layout);
}


void MainWindow::aboutToQuit() {
  writeSettings();
}


void MainWindow::about() {
  QDialog dlg;
  Ui::AboutDialog ui;
  ui.setupUi(&dlg);
  // Sorted alphabetically, first group is 2.0 contributors
  const QString msg = tr(
  "<qt><h2>Kst 2.0 - A data viewing program.</h2>\n<hr>\n"
  "Copyright &copy; 2000-2007 Barth Netterfield<br>"
  "<a href=\"http://kst.kde.org/\">http://kst.kde.org/</a><br>"
  "Please report bugs to: <a href=\"http://bugs.kde.org/\">http://bugs.kde.org/</a><br>"
  "Authors:<ul>"
  "<li>Barth Netterfield</li>"
  "<li><a href=\"http://www.staikos.net/\">Staikos Computing Services Inc.</a></li>"
  "<li>Ted Kisner</li>"
  "<li>The University of Toronto</li>"
  "</ul><ul>"
  "<li>Matthew Truch</li>"
  "<li>Nicolas Brisset</li>"
  "<li>Rick Chern</li>"
  "<li>Sumus Technology Limited</li>"
  "<li>The University of British Columbia</li>"
  "</ul>"
  );
  ui.text->setText(msg);
  dlg.exec();
}


void MainWindow::createBox() {
  CreateBoxCommand *cmd = new CreateBoxCommand;
  cmd->createItem();
}


void MainWindow::createEllipse() {
  CreateEllipseCommand *cmd = new CreateEllipseCommand;
  cmd->createItem();
}


void MainWindow::createLabel() {
  CreateLabelCommand *cmd = new CreateLabelCommand;
  cmd->createItem();
}


void MainWindow::createLine() {
  CreateLineCommand *cmd = new CreateLineCommand;
  cmd->createItem();
}


void MainWindow::createPicture() {
  CreatePictureCommand *cmd = new CreatePictureCommand;
  cmd->createItem();
}


void MainWindow::createPlot() {
  CreatePlotCommand *cmd = new CreatePlotCommand;
  cmd->createItem();
}


void MainWindow::createSvg() {
  CreateSvgCommand *cmd = new CreateSvgCommand;
  cmd->createItem();
}


void MainWindow::createLayout() {
  View *view = tabWidget()->currentView();
  QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
  if (!selectedItems.isEmpty()) {
    ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(selectedItems.first());
    Q_ASSERT(viewItem);
    viewItem->createLayout();
  } else {
    view->createLayout();
  }
}


void MainWindow::breakLayout() {
  View *view = tabWidget()->currentView();
  QList<QGraphicsItem*> selectedItems = view->scene()->selectedItems();
  if (!selectedItems.isEmpty()) {
    ViewItem *viewItem = qgraphicsitem_cast<ViewItem*>(selectedItems.first());
    Q_ASSERT(viewItem);
    viewItem->breakLayout();
  }
}


void MainWindow::demoModel() {
  Q_ASSERT(document() && document()->objectStore());
  VectorPtr v = kst_cast<Vector>(document()->objectStore()->createObject<Vector>(ObjectTag::fromString("V1")));
  Q_ASSERT(v);
  v->resize(999999);
  VectorPtr v2 = kst_cast<Vector>(document()->objectStore()->createObject<Vector>(ObjectTag::fromString("V2")));
  Q_ASSERT(v2);
  v2->resize(999999);
  EditableVectorPtr v3 = kst_cast<EditableVector>(document()->objectStore()->createObject<EditableVector>(ObjectTag::fromString("Editable V")));
  Q_ASSERT(v3);
  v3->resize(25);
  double *d = const_cast<double *>(v->value()); // yay :)
  double *d2 = const_cast<double *>(v2->value()); // yay :)
  d[0] = 1;
  d2[0] = 1;
  for (int i = 1; i < v->length(); ++i) {
    d[i] = d[i-1] + 0.002;
    d2[i] = d2[i-1] + 0.003;
  }
  GeneratedVectorPtr gv = kst_cast<GeneratedVector>(document()->objectStore()->createObject<GeneratedVector>(ObjectTag::fromString("Generated V")));
  Q_ASSERT(gv);
  gv->changeRange(0, 100, 1000);
  EquationPtr ep = kst_cast<Equation>(document()->objectStore()->createObject<Equation>(ObjectTag::fromString("My Equation")));
  Q_ASSERT(ep);
  ep->setExistingXVector(VectorPtr(gv), false);
  ep->setEquation("x^2");
  ep->writeLock();
  ep->update(0);
  ep->unlock();
//  addDataObjectToList(ep.data());
}


void MainWindow::createActions() {
  _undoAct = _undoGroup->createUndoAction(this);
  _undoAct->setShortcut(tr("Ctrl+Z"));
  _redoAct = _undoGroup->createRedoAction(this);
  _redoAct->setShortcut(tr("Ctrl+Shift+Z"));

  _createLabelAct = new QAction(tr("&Create label"), this);
  _createLabelAct->setStatusTip(tr("Create a label for the current view"));
  _createLabelAct->setIcon(QPixmap(":kst_gfx_label.png"));
  _createLabelAct->setEnabled(false);
  connect(_createLabelAct, SIGNAL(triggered()), this, SLOT(createLabel()));

  _createBoxAct = new QAction(tr("&Create box"), this);
  _createBoxAct->setStatusTip(tr("Create a box for the current view"));
  _createBoxAct->setIcon(QPixmap(":kst_gfx_rectangle.png"));
  _createBoxAct->setEnabled(false);
  connect(_createBoxAct, SIGNAL(triggered()), this, SLOT(createBox()));

  _createEllipseAct = new QAction(tr("&Create ellipse"), this);
  _createEllipseAct->setStatusTip(tr("Create an ellipse for the current view"));
  _createEllipseAct->setIcon(QPixmap(":kst_gfx_ellipse.png"));
  _createEllipseAct->setEnabled(false);
  connect(_createEllipseAct, SIGNAL(triggered()), this, SLOT(createEllipse()));

  _createLineAct = new QAction(tr("&Create line"), this);
  _createLineAct->setStatusTip(tr("Create a line for the current view"));
  _createLineAct->setIcon(QPixmap(":kst_gfx_line.png"));
  _createLineAct->setEnabled(false);
  connect(_createLineAct, SIGNAL(triggered()), this, SLOT(createLine()));

  _createPictureAct = new QAction(tr("&Create picture"), this);
  _createPictureAct->setStatusTip(tr("Create a picture for the current view"));
  _createPictureAct->setIcon(QPixmap(":kst_gfx_picture.png"));
  _createPictureAct->setEnabled(false);
  connect(_createPictureAct, SIGNAL(triggered()), this, SLOT(createPicture()));

  _createPlotAct = new QAction(tr("&Create plot"), this);
  _createPlotAct->setStatusTip(tr("Create a plot for the current view"));
  _createPlotAct->setIcon(QPixmap(":kst_newplot.png"));
  _createPlotAct->setEnabled(false);
  connect(_createPlotAct, SIGNAL(triggered()), this, SLOT(createPlot()));

  _createSvgAct = new QAction(tr("&Create svg"), this);
  _createSvgAct->setStatusTip(tr("Create a svg for the current view"));
  _createSvgAct->setEnabled(false);
  connect(_createSvgAct, SIGNAL(triggered()), this, SLOT(createSvg()));

  _createLayoutAct = new QAction(tr("&Create layout"), this);
  _createLayoutAct->setStatusTip(tr("Create a layout for the current item"));
  _createLayoutAct->setIcon(QPixmap(":kst_gfx_layout.png"));
  _createLayoutAct->setEnabled(false);
  connect(_createLayoutAct, SIGNAL(triggered()), this, SLOT(createLayout()));

  _breakLayoutAct = new QAction(tr("&Break layout"), this);
  _breakLayoutAct->setStatusTip(tr("Break the layout for the current item"));
  _breakLayoutAct->setIcon(QPixmap(":kst_gfx_breaklayout.png"));
  _breakLayoutAct->setEnabled(false);
  connect(_breakLayoutAct, SIGNAL(triggered()), this, SLOT(breakLayout()));

  _layoutModeAct = new QAction(tr("&Layout Mode"), this);
  _layoutModeAct->setStatusTip(tr("Toggle the current view's layout mode"));
  _layoutModeAct->setIcon(QPixmap(":kst_layoutmode.png"));
  _layoutModeAct->setCheckable(true);
  connect(_layoutModeAct, SIGNAL(toggled(bool)), this, SLOT(setLayoutMode(bool)));

  _tiedZoomAct = new QAction(tr("&Tied Zoom"), this);
  _tiedZoomAct->setStatusTip(tr("Toggle the current view's tied zoom"));
  _tiedZoomAct->setIcon(QPixmap(":kst_zoomtie.png"));
  _tiedZoomAct->setCheckable(true);
  connect(_tiedZoomAct, SIGNAL(toggled(bool)), this, SLOT(setTiedZoom(bool)));

  _newTabAct = new QAction(tr("&New tab"), this);
  _newTabAct->setStatusTip(tr("Create a new tab"));
  _newTabAct->setIcon(QPixmap(":kst_newtab.png"));
  connect(_newTabAct, SIGNAL(triggered()), tabWidget(), SLOT(createView()));

  _closeTabAct = new QAction(tr("&Close tab"), this);
  _closeTabAct->setStatusTip(tr("Close the current tab"));
  _closeTabAct->setIcon(QPixmap(":kst_closetab.png"));
  connect(_closeTabAct, SIGNAL(triggered()), tabWidget(), SLOT(closeCurrentView()));

  _saveAct = new QAction(tr("&Save"), this);
  _saveAct->setStatusTip(tr("Save the current session"));
  _saveAct->setShortcut(tr("Ctrl+S"));
  connect(_saveAct, SIGNAL(triggered()), this, SLOT(save()));

  _saveAsAct = new QAction(tr("Save &as..."), this);
  _saveAsAct->setStatusTip(tr("Save the current session"));
  connect(_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  _openAct = new QAction(tr("&Open..."), this);
  _openAct->setStatusTip(tr("Open a new session"));
  _openAct->setShortcut(tr("Ctrl+O"));
  connect(_openAct, SIGNAL(triggered()), this, SLOT(open()));

  _printAct = new QAction(tr("&Print..."), this);
  _printAct->setStatusTip(tr("Print the current view"));
  connect(_printAct, SIGNAL(triggered()), this, SLOT(print()));

  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  _dataManagerAct = new QAction(tr("Data &Manager..."), this);
  _dataManagerAct->setStatusTip(tr("Show Kst's data manager window"));
  _dataManagerAct->setIcon(QPixmap(":kst_datamanager.png"));
  connect(_dataManagerAct, SIGNAL(triggered()), this, SLOT(showDataManager()));

  _viewManagerAct = new QAction(tr("View &Manager..."), this);
  _viewManagerAct->setStatusTip(tr("Show Kst's view manager window"));
  _viewManagerAct->setIcon(QPixmap(":kst_viewmanager.png"));
  connect(_viewManagerAct, SIGNAL(triggered()), this, SLOT(showViewManager()));

  _vectorEditorAct = new QAction(tr("&Edit Vectors..."), this);
  _vectorEditorAct->setStatusTip(tr("Show all vectors in a spreadsheet"));
  connect(_vectorEditorAct, SIGNAL(triggered()), this, SLOT(showVectorEditor()));

  _scalarEditorAct = new QAction(tr("Edit &Scalars..."), this);
  _scalarEditorAct->setStatusTip(tr("Show all scalars in a spreadsheet"));
  connect(_scalarEditorAct, SIGNAL(triggered()), this, SLOT(showScalarEditor()));

  _stringEditorAct = new QAction(tr("Edit S&trings..."), this);
  _stringEditorAct->setStatusTip(tr("Show all strings in a spreadsheet"));
  connect(_stringEditorAct, SIGNAL(triggered()), this, SLOT(showStringEditor()));

  _matrixEditorAct = new QAction(tr("Edit &Matrix..."), this);
  _matrixEditorAct->setStatusTip(tr("Show all matrix in a spreadsheet"));
  connect(_matrixEditorAct, SIGNAL(triggered()), this, SLOT(showMatrixEditor()));

  _exportGraphicsAct = new QAction(tr("&Export..."), this);
  _exportGraphicsAct->setStatusTip(tr("Export graphics to disk"));
  connect(_exportGraphicsAct, SIGNAL(triggered()), this, SLOT(showExportGraphicsDialog()));

  _debugDialogAct = new QAction(tr("&Debug Dialog..."), this);
  _debugDialogAct->setStatusTip(tr("Show the Kst debugging dialog"));
  connect(_debugDialogAct, SIGNAL(triggered()), this, SLOT(showDebugDialog()));

  _aboutAct = new QAction(tr("&About"), this);
  _aboutAct->setStatusTip(tr("Show Kst's About box"));
  connect(_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  _settingsDialogAct = new QAction(tr("&Configure Kst"), this);
  _settingsDialogAct->setStatusTip(tr("Show Kst's Configuration Dialog"));
  connect(_settingsDialogAct, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));

  _differentiateCurvesDialogAct = new QAction(tr("&Differentiate Curves"), this);
  _differentiateCurvesDialogAct->setStatusTip(tr("Show Kst's Differentiate Curves Dialog"));
  connect(_differentiateCurvesDialogAct, SIGNAL(triggered()), this, SLOT(showDifferentiateCurvesDialog()));

  _chooseColorDialogAct = new QAction(tr("Assign &Curve Color per File"), this);
  _chooseColorDialogAct->setStatusTip(tr("Show Kst's Choose Color Dialog"));
  connect(_chooseColorDialogAct, SIGNAL(triggered()), this, SLOT(showChooseColorDialog()));

  _changeDataSampleDialogAct = new QAction(tr("Change Data Sample Range"), this);
  _changeDataSampleDialogAct->setStatusTip(tr("Show Kst's Change Data Sample Range Dialog"));
  connect(_changeDataSampleDialogAct, SIGNAL(triggered()), this, SLOT(showChangeDataSampleDialog()));

  _changeFileDialogAct = new QAction(tr("Change Data &File"), this);
  _changeFileDialogAct->setStatusTip(tr("Show Kst's Change Data File Dialog"));
  connect(_changeFileDialogAct, SIGNAL(triggered()), this, SLOT(showChangeFileDialog()));

}


void MainWindow::createMenus() {
  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_newTabAct);
  _fileMenu->addAction(_openAct);
  _fileMenu->addAction(_saveAct);
  _fileMenu->addAction(_saveAsAct);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_printAct);
  _fileMenu->addAction(_exportGraphicsAct);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_closeTabAct);
  _fileMenu->addAction(_exitAct);

  _editMenu = menuBar()->addMenu(tr("&Edit"));
  _editMenu->addAction(_undoAct);
  _editMenu->addAction(_redoAct);

  _dataMenu = menuBar()->addMenu(tr("&Data"));
  _dataMenu->addAction(_dataManagerAct);
  _dataMenu->addSeparator();
  _dataMenu->addAction(_vectorEditorAct);
  _dataMenu->addAction(_scalarEditorAct);
  _dataMenu->addAction(_matrixEditorAct);
  _dataMenu->addAction(_stringEditorAct);

  _viewMenu = menuBar()->addMenu(tr("&View"));
  _viewMenu->addAction(_viewManagerAct);
  _viewMenu->addAction(_tiedZoomAct);
  _viewMenu->addSeparator();

  _layoutMenu = _viewMenu->addMenu(tr("&Layout"));

  _layoutMenu->setIcon(QPixmap(":kst_layoutmode.png"));

  _layoutMenu->addAction(_layoutModeAct);
  _layoutMenu->addSeparator();
  _layoutMenu->addAction(_createLabelAct);
  _layoutMenu->addAction(_createBoxAct);
  _layoutMenu->addAction(_createEllipseAct);
  _layoutMenu->addAction(_createLineAct);
  _layoutMenu->addAction(_createPictureAct);
  _layoutMenu->addAction(_createPlotAct);
  _layoutMenu->addAction(_createSvgAct);

  _toolsMenu = menuBar()->addMenu(tr("&Tools"));
  _toolsMenu->addAction(_changeFileDialogAct);
  _toolsMenu->addAction(_changeDataSampleDialogAct);
  _toolsMenu->addAction(_chooseColorDialogAct);
  _toolsMenu->addAction(_differentiateCurvesDialogAct);

  _settingsMenu = menuBar()->addMenu(tr("&Settings"));
  _settingsMenu->addAction(_settingsDialogAct);

  menuBar()->addSeparator();

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_debugDialogAct);
  _helpMenu->addAction(_aboutAct);

  // FIXME: remove this later.
  QMenu *demoMenu = menuBar()->addMenu("&Demo");

  QAction *demoModel = new QAction("Vector model", this);
  connect(demoModel, SIGNAL(triggered()), this, SLOT(demoModel()));
  demoMenu->addAction(demoModel);
}


void MainWindow::createToolBars() {
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  _dataToolBar = addToolBar(tr("Data"));
  _dataToolBar->addAction(_dataManagerAct);
//   _dataToolBar->addAction(_vectorEditorAct); //no icon
//   _dataToolBar->addAction(_scalarEditorAct); //no icon
//   _dataToolBar->addAction(_matrixEditorAct); //no icon

  _viewToolBar = addToolBar(tr("View"));
  _viewToolBar->addAction(_viewManagerAct);
  _viewToolBar->addAction(_tiedZoomAct);
  _viewToolBar->addAction(_layoutModeAct);

  _layoutToolBar = new QToolBar(tr("Layout"), this);
//  _layoutToolBar->addAction(_createLabelAct); //no icon
  _layoutToolBar->addAction(_createBoxAct);
  _layoutToolBar->addAction(_createEllipseAct);
  _layoutToolBar->addAction(_createLineAct);
  _layoutToolBar->addAction(_createPictureAct);
  _layoutToolBar->addAction(_createPlotAct);
//  _layoutToolBar->addAction(_createSvgAct); //no icon

  _layoutToolBar->addSeparator();

  _layoutToolBar->addAction(_createLayoutAct);
  _layoutToolBar->addAction(_breakLayoutAct);
  _layoutToolBar->setVisible(false);

  addToolBar(Qt::TopToolBarArea, _layoutToolBar);
}


void MainWindow::createStatusBar() {
  _progressBar = new QProgressBar(statusBar());
  _progressBar->hide();
  statusBar()->addPermanentWidget(_progressBar);
  MemoryWidget *mw = new MemoryWidget(statusBar());
  statusBar()->addPermanentWidget(mw);
  DebugNotifier *dn = new DebugNotifier(statusBar());
  connect(dn, SIGNAL(showDebugLog()), this, SLOT(showDebugDialog()));
  connect(_debugDialog, SIGNAL(notifyOfError()), dn, SLOT(reanimate()));
  connect(_debugDialog, SIGNAL(notifyAllClear()), dn, SLOT(close()));
  statusBar()->addPermanentWidget(dn);
  statusBar()->showMessage(tr("Ready"));
}


QProgressBar *MainWindow::progressBar() const {
  return _progressBar;
}


void MainWindow::showDataManager() {
  if (!_dataManager) {
    _dataManager = new DataManager(this, _doc);
  }
  _dataManager->show();
}


void MainWindow::showViewManager() {
  if (!_viewManager) {
    _viewManager = new ViewManager(this);
  }
  _viewManager->show();
}


void MainWindow::showVectorEditor() {
  if (!_vectorEditor) {
    _vectorEditor = new VectorEditorDialog(this, _doc);
  }
  _vectorEditor->show();
}


void MainWindow::showScalarEditor() {
  if (!_scalarEditor) {
    _scalarEditor = new ScalarEditorDialog(this, _doc);
  }
  _scalarEditor->show();
}


void MainWindow::showStringEditor() {
  if (!_stringEditor) {
    _stringEditor = new StringEditorDialog(this);
  }
  _stringEditor->show();
}

void MainWindow::showMatrixEditor() {
  if (!_matrixEditor) {
    _matrixEditor = new MatrixEditorDialog(this, _doc);
  }
  _matrixEditor->show();
}


void MainWindow::showDebugDialog() {
  if (!_debugDialog) {
    _debugDialog = new DebugDialog(this);
  }
  _debugDialog->show();
}


void MainWindow::showExportGraphicsDialog() {
  if (!_exportGraphics) {
    _exportGraphics = new ExportGraphicsDialog(this);
  }
  _exportGraphics->show();
}


void MainWindow::showSettingsDialog() {
  ApplicationSettingsDialog settingsDialog(this);
  settingsDialog.exec();
}


void MainWindow::showDifferentiateCurvesDialog() {
  DifferentiateCurvesDialog differentiateCurvesDialog(this);
  differentiateCurvesDialog.exec();
}


void MainWindow::showChooseColorDialog() {
  ChooseColorDialog chooseColorDialog(this);
  chooseColorDialog.exec();
}


void MainWindow::showChangeDataSampleDialog() {
  ChangeDataSampleDialog changeDataSampleDialog(this);
  changeDataSampleDialog.exec();
}


void MainWindow::showChangeFileDialog() {
  ChangeFileDialog changeFileDialog(this);
  changeFileDialog.exec();
}


void MainWindow::readSettings() {
  QSettings settings;
  QPoint pos = settings.value("pos", QPoint(20, 20)).toPoint();
  QSize size = settings.value("size", QSize(800, 600)).toSize();
  resize(size);
  move(pos);
}


void MainWindow::writeSettings() {
  QSettings settings;
  settings.setValue("pos", pos());
  settings.setValue("size", size());
}

}

// vim: ts=2 sw=2 et
