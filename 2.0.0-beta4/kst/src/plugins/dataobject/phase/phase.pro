include($$PWD/../../../../kst.pri)
include($$PWD/../../../../dataobjectplugin.pri)

TARGET = $$qtLibraryTarget(kstplugin_phase)

SOURCES += \
    phase.cpp

HEADERS += \
    phase.h

FORMS += phaseconfig.ui