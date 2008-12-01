include($$PWD/../../kst.pri)

QT += gui network svg xml opengl qt3support

TEMPLATE = lib
OBJECTS_DIR = tmp
MOC_DIR = tmp
TARGET = kstapp
DESTDIR = $$OUTPUT_DIR/lib
win32:CONFIG += staticlib

INCLUDEPATH += \
    tmp \
    $$TOPLEVELDIR/src/libkst \
    $$TOPLEVELDIR/src/libkstmath \
    $$TOPLEVELDIR/src/widgets \
    $$OUTPUT_DIR/src/widgets \
    $$OUTPUT_DIR/src/libkstapp/tmp

LIBS += -lkst -lkstmath -lkstwidgets

SOURCES += \
    application.cpp \
    applicationsettings.cpp \
    applicationsettingsdialog.cpp \
    arrowitem.cpp \
    arrowitemdialog.cpp \
    arrowpropertiestab.cpp \
    axis.cpp \
    axistab.cpp \
    basicplugindialog.cpp \
    boxitem.cpp \
    builtingraphics.cpp \
    cartesianrenderitem.cpp \
    changedatasampledialog.cpp \
    changefiledialog.cpp \
    choosecolordialog.cpp \
    circleitem.cpp \
    commandlineparser.cpp \
    contenttab.cpp \
    csddialog.cpp \
    curvedialog.cpp \
    databutton.cpp \
    databuttonaction.cpp \
    datadialog.cpp \
    datagui.cpp \
    datamanager.cpp \
    datasourcedialog.cpp \
    datatab.cpp \
    datawizard.cpp \
    debugdialog.cpp \
    debugnotifier.cpp \
    dialog.cpp \
    dialoglaunchergui.cpp \
    dialogpage.cpp \
    dialogtab.cpp \
    differentiatecurvesdialog.cpp \
    document.cpp \
    editmultiplewidget.cpp \
    ellipseitem.cpp \
    equationdialog.cpp \
    eventmonitordialog.cpp \
    exportgraphicsdialog.cpp \
    filltab.cpp \
    filterfitdialog.cpp \
    generaltab.cpp \
    graphicsfactory.cpp \
    gridlayouthelper.cpp \
    gridtab.cpp \
    histogramdialog.cpp \
    imagedialog.cpp \
    labelitem.cpp \
    labelitemdialog.cpp \
    labelpropertiestab.cpp \
    labelrenderer.cpp \
    labeltab.cpp \
    layoutboxitem.cpp \
    layouttab.cpp \
    legenditem.cpp \
    legenditemdialog.cpp \
    legendtab.cpp \
    lineitem.cpp \
    logwidget.cpp \
    mainwindow.cpp \
    markerstab.cpp \
    matrixdialog.cpp \
    matrixmodel.cpp \
    memorywidget.cpp \
    pictureitem.cpp \
    plotaxis.cpp \
    plotitem.cpp \
    plotitemdialog.cpp \
    plotitemmanager.cpp \
    plotmarkers.cpp \
    plotrenderitem.cpp \
    powerspectrumdialog.cpp \
    scalardialog.cpp \
    scalarmodel.cpp \
    scene.cpp \
    selectionrect.cpp \
    sessionmodel.cpp \
    stringdialog.cpp \
    stringmodel.cpp \
    stroketab.cpp \
    svgitem.cpp \
    tabwidget.cpp \
    vectordialog.cpp \
    vectormodel.cpp \
    viewcommand.cpp \
    view.cpp \
    viewgridlayout.cpp \
    viewitem.cpp \
    viewitemdialog.cpp \
    viewmanager.cpp \
    viewmatrixdialog.cpp \
    viewprimitivedialog.cpp \
    viewvectordialog.cpp

HEADERS += \
    application.h \
    applicationsettings.h \
    applicationsettingsdialog.h \
    arrowitem.h \
    arrowitemdialog.h \
    arrowpropertiestab.h \
    axis.h \
    axistab.h \
    basicplugindialog.h \
    boxitem.h \
    builtingraphics.h \
    cartesianrenderitem.h \
    changedatasampledialog.h \
    changefiledialog.h \
    choosecolordialog.h \
    circleitem.h \
    commandlineparser.h\
    contenttab.h \
    csddialog.h \
    curvedialog.h \
    databutton.h \
    databuttonaction.h \
    datadialog.h \
    datagui.h \
    datamanager.h \
    dataref.h \
    datasourcedialog.h \
    datatab.h \
    datawizard.h \
    debugdialog.h \
    debugnotifier.h \
    dialog.h \
    dialoglaunchergui.h \
    dialogpage.h \
    dialogtab.h \
    differentiatecurvesdialog.h \
    document.h \
    editmultiplewidget.h \
    ellipseitem.h \
    equationdialog.h \
    eventmonitordialog.h \
    exportgraphicsdialog.h \
    filltab.h \
    filterfitdialog.h \
    generaltab.h \
    graphicsfactory.h \
    gridlayouthelper.h \
    gridtab.h \
    histogramdialog.h \
    imagedialog.h \
    labelitem.h \
    labelitemdialog.h \
    labelpropertiestab.h \
    labelrenderer.h \
    labeltab.h \
    layoutboxitem.h \
    layouttab.h \
    legenditem.h \
    legenditemdialog.h \
    legendtab.h \
    lineitem.h \
    logwidget.h \
    mainwindow.h \
    markerstab.h \
    matrixdialog.h \
    matrixmodel.h \
    memorywidget.h \
    pictureitem.h \
    plotaxis.h \
    plotitem.h \
    plotitemdialog.h \
    plotmarkers.h \
    plotitemmanager.h \
    plotrenderitem.h \
    powerspectrumdialog.h \
    scalardialog.h \
    scalarmodel.h \
    scene.h \
    selectionrect.h \
    sessionmodel.h \
    stringdialog.h \
    stringmodel.h \
    svgitem.h \
    stroketab.h \
    tabwidget.h \
    vectordialog.h \
    vectormodel.h \
    viewcommand.h \
    view.h \
    viewgridlayout.h \
    viewitemdialog.h \
    viewitem.h \
    viewitemzorder.h \
    viewmanager.h \
    viewmatrixdialog.h \
    viewprimitivedialog.h \
    viewvectordialog.h

FORMS += \
    aboutdialog.ui \
    arrowpropertiestab.ui \
    axistab.ui \
    basicplugintab.ui \
    changedatasampledialog.ui \
    changefiledialog.ui \
    choosecolordialog.ui \
    contenttab.ui \
    csdtab.ui \
    curvetab.ui \
    datamanager.ui \
    datawizardpagedatapresentation.ui \
    datawizardpagedatasource.ui \
    datawizardpagefilters.ui \
    datawizardpageplot.ui \
    datawizardpagevectors.ui \
    debugdialog.ui \
    dialog.ui \
    differentiatecurvesdialog.ui \
    editmultiplewidget.ui \
    equationtab.ui \
    eventmonitortab.ui \
    exportgraphicsdialog.ui \
    filltab.ui \
    filterfittab.ui \
    generaltab.ui \
    gridtab.ui \
    histogramtab.ui \
    imagetab.ui \
    labeltab.ui \
    labelpropertiestab.ui \
    layouttab.ui \
    legendtab.ui \
    markerstab.ui \
    matrixtab.ui \
    powerspectrumtab.ui \
    scalartab.ui \
    stringtab.ui \
    stroketab.ui \
    vectortab.ui \
    viewmanager.ui \
    viewmatrixdialog.ui \
    viewprimitivedialog.ui \
    viewvectordialog.ui

RESOURCES += \
    $$TOPLEVELDIR/src/images/images.qrc