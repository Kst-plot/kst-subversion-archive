include($$PWD/../../../../kst.pri)
include($$PWD/../../../../dataobjectplugin.pri)

TARGET = $$qtLibraryTarget(kstplugin_statistics)

SOURCES += \
    statistics.cpp

HEADERS += \
    statistics.h

FORMS += statisticsconfig.ui