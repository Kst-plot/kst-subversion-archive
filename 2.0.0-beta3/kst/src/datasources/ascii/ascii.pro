include($$PWD/../../../kst.pri)
include($$PWD/../../../datasourceplugin.pri)
TARGET = kst2data_ascii
INCLUDEPATH += $$OUTPUT_DIR/src/datasources/ascii/tmp
SOURCES += ascii.cpp
HEADERS += ascii.h \
    ../../libkst/kst_inf.h
FORMS += asciiconfig.ui