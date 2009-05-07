include($$PWD/../../kst.pri)

QT += gui

TEMPLATE = lib
CONFIG += designer plugin
TARGET = kstwidgets
win32:CONFIG += staticlib

INSTALL_DIR = $$(INSTDIR)
! isEmpty(INSTALL_DIR) {
  target.path = $$INSTALL_DIR/plugin
  INSTALLS += target
}

DESTDIR = $$OUTPUT_DIR/plugin

INCLUDEPATH += \
    tmp \
    $$TOPLEVELDIR/src/libkst \
    $$TOPLEVELDIR/src/libkstmath \
    $$OUTPUT_DIR/src/widgets/tmp

LIBS += -L$$OUTPUT_DIR/lib -lkstmath -lkst

SOURCES += \
    colorbutton.cpp \
    colorpalette.cpp \
    combobox.cpp \
    curveappearance.cpp \
    curveplacement.cpp \
    curveselector.cpp \
    datarange.cpp \
    datasourceselectordialog.cpp \
    datasourceselector.cpp \
    dialogdefaults.cpp \
    fftoptions.cpp \
    filerequester.cpp \
    gradienteditor.cpp \
    labelbuilder.cpp \
    labellineedit.cpp \
    matrixselector.cpp \
    scalarlistselector.cpp \
    scalarselector.cpp \
    stringselector.cpp \
    vectorselector.cpp \
    widgets.cpp

HEADERS += \
    colorbutton.h \
    colorpalette.h \
    combobox.h \
    curveappearance.h \
    curveplacement.h \
    curveselector.h \
    datarange.h \
    datasourceselectordialog.h \
    datasourceselector.h \
    dialogdefaults.h \
    fftoptions.h \
    filerequester.h \
    gradienteditor.h \
    labelbuilder.h \
    labellineedit.h \
    matrixselector.h \
    scalarselector.h \
    scalarlistselector.h \
    stringselector.h \
    vectorselector.h \
    widgets.h

FORMS += \
    colorpalette.ui \
    curveappearance.ui \
    curveplacement.ui \
    curveselector.ui \
    datarange.ui \
    fftoptions.ui \
    labelbuilder.ui \
    matrixselector.ui \
    scalarlistselector.ui \
    scalarselector.ui \
    stringselector.ui \
    vectorselector.ui

RESOURCES += \
    $$TOPLEVELDIR/src/images/images.qrc
