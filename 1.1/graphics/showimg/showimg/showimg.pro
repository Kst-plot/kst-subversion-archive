TEMPLATE	= lib

include( ../common.pro )
win32 {
CONFIG +=console
CONFIG -=windows
}

DEFINES += MAKE_SHOWIMGCORE_LIB
#QMAKE_CXXFLAGS += /FI$(QKW)/kdeextragear-2/showimg/showimg/showimg_exp.h

TARGET		= showimgcore$$KDEBUG

LIBS += $$QKWLIB/dcop$$KDELIB_SUFFIX $$QKWLIB/kio$$KDELIB_SUFFIX \
 $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX $$QKWLIB/kdefx$$KDELIB_SUFFIX

INCLUDEPATH+=$(QKW)/kdelibs/kio/bookmarks


# icon
#win32:LIBS+=$(QKW)/files/resources/kdialog.res

SOURCES += \
listitem.cpp \
directory.cpp \
desktoplistitem.cpp \
ProgressDialog.cpp \
album.cpp \
albumimagefileiconitem.cpp \
batchrenamer.cpp \
compressedfileitem.cpp \
compressedimagefileiconitem.cpp \
confshowimg.cpp \
describe.cpp \
describeAlbum.cpp \
directoryview.cpp \
dirfileiconitem.cpp \
displayCompare.cpp \
exif.cpp \
extract.cpp \
fileiconitem.cpp \
formatconversion.cpp \
history_action.cpp \
imagefileiconitem.cpp \
imagefileinfo.cpp \
imagelistview.cpp \
imageloader.cpp \
imageviewer.cpp \
jpgoptions.cpp \
kexifpropsplugin.cpp \
kstartuplogo.cpp \
mainwindow.cpp \
mybookmarkmanager.cpp \
numSlider.cpp \
printImageDialog.cpp \
qtfileicondrag.cpp \
rename.cpp \
zipfile.cpp \
imagemetainfo.cpp \
win_utils.cpp

!contains(DEFINES,SHOWIMG_NO_CDARCHIVE) {
	SOURCES += \
	cdarchive.cpp \
	cdarchivecreator.cpp \
	cdarchivecreatordialog.cpp \
}

#replace.cpp \
#digikamplugins/albumitemhandler.cpp \
#digikamplugins/digikampluginmanager.cpp


#khexeditpropsplugin.cpp \
