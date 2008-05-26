include($$PWD/../../kst.pri)

QT += xml qt3support

TEMPLATE = lib
OBJECTS_DIR = tmp
MOC_DIR = tmp
TARGET = kstmath
DESTDIR = $$OUTPUT_DIR/lib
win32:CONFIG += staticlib

INCLUDEPATH += \
    tmp \
    $$TOPLEVELDIR/src/libkst \
    $$OUTPUT_DIR/src/libkstmath/tmp

LIBS += -lkst

#Don't trigger qmake's lex/yacc handling by default.
#Rather we want to use the files that are pre-generated and checked in.
QMAKE_EXT_LEX = .lex
QMAKE_EXT_YACC = .yacc

SOURCES += \
    basicplugin.cpp \
    builtinobjects.cpp \
    builtinrelations.cpp \
    colorsequence.cpp \
    csd.cpp \
    csdfactory.cpp \
    curve.cpp \
    curvefactory.cpp \
    curvehint.cpp \
    curvepointsymbol.cpp \
    dataobject.cpp \
    dataobjectcollection.cpp \
    defaultnames.cpp \
    dialoglauncher.cpp \
    emailthread.cpp \
    eparse.cpp \
    eparse-eh.cpp \
    enodes.cpp \
    enodefactory.cpp \
    escan.cpp \
    equation.cpp \
    equationfactory.cpp \
    eventmonitorentry.cpp \
    eventmonitorfactory.cpp \
    fftsg_h.c \
    histogram.cpp \
    histogramfactory.cpp \
    image.cpp \
    imagefactory.cpp \
    labelparser.cpp \
    linestyle.cpp \
    objectfactory.cpp \
    painter.cpp \
    palette.cpp \
    psd.cpp \
    psdcalculator.cpp \
    psdfactory.cpp \
    relation.cpp \
    relationfactory.cpp \
    settings.cpp 

HEADERS += \
    basicplugin.h \
    builtinobjects.h \
    builtinrelations.h \
    colorsequence.h \
    csd.h \
    csdfactory.h \
    curve.h \
    curvefactory.h \
    curvehint.h \
    curvepointsymbol.h \
    dialoglauncher.h \
    enodefactory.h \
    enodes.h \
    eparse-eh.h \
    eparse.h \
    dataobjectcollection.h \
    dataobject.h \
    dataobjectplugin.h \
    defaultnames.h \
    emailthread.h \
    equation.h \
    equationfactory.h \
    eventmonitorentry.h \
    eventmonitorfactory.h \
    histogram.h \
    histogramfactory.h \
    image.h \
    imagefactory.h \
    labelparser.h \
    linestyle.h \
    objectfactory.h \
    painter.h \
    palette.h \
    plotdefines.h \
    psd.h \
    psdcalculator.h \
    relation.h \
    relationfactory.h \
    settings.h 

#LEXSOURCES += escan.l
#YACCSOURCES += eparse.y
