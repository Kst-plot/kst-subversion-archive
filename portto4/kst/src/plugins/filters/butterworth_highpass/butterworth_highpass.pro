include($$PWD/../../../../kst.pri)
include($$PWD/../../../../dataobjectplugin.pri)

TARGET = $$qtLibraryTarget(kstplugin_filterbutterworthhighpass)
LIBS += -lgsl

SOURCES += \
    butterworth_highpass.cpp

HEADERS += \
    butterworth_highpass.h

FORMS += filterbutterworthhighpassconfig.ui
