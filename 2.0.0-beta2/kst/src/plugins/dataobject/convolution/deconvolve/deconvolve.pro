include($$PWD/../../../../../kst.pri)
include($$PWD/../../../../../dataobjectplugin.pri)

TARGET = kstplugin_deconvolve
LIBS += -lgsl

SOURCES += \
    deconvolve.cpp

HEADERS += \
    deconvolve.h

FORMS += deconvolveconfig.ui
