TEMPLATE	= app

include( ../common.pro )
win32 {
CONFIG +=console
CONFIG -=windows
}
TARGET		= showimg

win32:LIBS += $$QKWLIB/dcop$$KDELIB_SUFFIX $$QKWLIB/kio$$KDELIB_SUFFIX \
 $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX $$QKWLIB/kdefx$$KDELIB_SUFFIX \
 $$QKWLIB/showimgcore$$SHOWIMG_SUFFIX

INCLUDEPATH+=$(QKW)/kdelibs/kio/bookmarks $(QKW)/kdeextragear-2/showimg

# icon
#win32:LIBS+=$(QKW)/files/resources/kdialog.res

SOURCES += \
main.cpp

