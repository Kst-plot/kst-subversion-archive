include($$PWD/../../../../kst.pri)
include($$PWD/../../../../dataobjectplugin.pri)

TARGET = $$qtLibraryTarget(kstplugin_effectivebandwidth)

SOURCES += \
    effectivebandwidth.cpp

HEADERS += \
    effectivebandwidth.h

FORMS += effectivebandwidthconfig.ui