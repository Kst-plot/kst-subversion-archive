include(../../plugins_sub.pri)

TARGET = $$kstlib(kstplugin_fitgradient_weighted)
LIBS += -lgsl

SOURCES += \
    fitgradient_weighted.cpp

HEADERS += \
    fitgradient_weighted.h

FORMS += fitgradient_weightedconfig.ui
