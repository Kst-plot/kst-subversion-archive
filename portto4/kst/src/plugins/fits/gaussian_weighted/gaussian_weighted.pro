include(../../plugins_sub.pri)

TARGET = $$qtLibraryTarget(kstplugin_fitgaussian_weighted)
LIBS += -lgsl

SOURCES += \
    fitgaussian_weighted.cpp

HEADERS += \
    fitgaussian_weighted.h

FORMS += fitgaussian_weightedconfig.ui
