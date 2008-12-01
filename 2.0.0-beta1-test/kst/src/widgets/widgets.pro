include($$PWD/../../kst.pri)

QT += gui xml qt3support

TEMPLATE = lib
CONFIG += designer plugin
OBJECTS_DIR = tmp
MOC_DIR = tmp
TARGET = kstwidgets
win32:CONFIG += staticlib

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

DESTDIR = $$OUTPUT_DIR/plugin

INCLUDEPATH += \
    tmp \
    $$TOPLEVELDIR/src/libkst \
    $$TOPLEVELDIR/src/libkstmath \
    $$OUTPUT_DIR/src/widgets/tmp

LIBS += -lkst -lkstmath

SOURCES += \
    colorbutton.cpp \
    colorpalette.cpp \
    combobox.cpp \
    curveappearance.cpp \
    curveplacement.cpp \
    curveselector.cpp \
    datarange.cpp \
    datasourceselectordialog.cpp \
    dialogdefaults.cpp \
    fftoptions.cpp \
    filerequester.cpp \
    gradienteditor.cpp \
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
    dialogdefaults.h \
    fftoptions.h \
    filerequester.h \
    gradienteditor.h \
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
    matrixselector.ui \
    scalarlistselector.ui \
    scalarselector.ui \
    stringselector.ui \
    vectorselector.ui

RESOURCES += \
    $$TOPLEVELDIR/src/images/images.qrc