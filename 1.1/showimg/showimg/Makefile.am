SUBDIRS =  khexedit kipiplugins misc . mimetypes  pics icons

INCLUDES =  -I../kexidb -I$(top_srcdir)/showimg $(all_includes)

AM_CPPFLAGS = \
		$(LIBKEXIF_CFLAGS) $(LIBEXIF_CFLAGS) \
		$(LIBKIPI_CFLAGS) $(LIBKEXIDB_CPPFLAGS) \
		$(all_includes)

lib_LTLIBRARIES = libshowimgcore.la

## Missing: version number
libshowimgcore_la_LDFLAGS = $(all_libraries) -no-undefined

libshowimgcore_la_LIBADD   =  \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KDEPRINT) \
	-lkonq -lkmdi\
	$(LIB_QT) \
	-lX11  $(LIBPTHREAD) \
	$(LIBJPEG) $(LIBPNG) \
	$(LIBKIPI_LIBS) $(LIBKEXIF_LIBS) $(LIBEXIF_LIBS) \
	misc/libmisc.la \
	khexedit/libkhexedit.la \
	kipiplugins/libkipiplugins.la \
	$(LIBKEXIDB_LDDADD) 
#imageeditor/libdigikamimageplugins.la -ldigikam
	

libshowimgcore_la_METASOURCES = AUTO

libshowimgcore_la_SOURCES = \
	dirfileiconitem.cpp \
	listitem.cpp \
	printimagedialog.cpp \
	khexeditpropsplugin.cpp \
	kstartuplogo.cpp \
	albumimagefileiconitem.cpp \
	compressedfileitem.cpp \
	imageviewer.cpp \
	imagelistview.cpp \
	imagefileiconitem.cpp \
	fileiconitem.cpp \
	compressedimagefileiconitem.cpp \
	album.cpp \
	qtfileicondrag.cpp \
	zipfile.cpp \
	rename.cpp \
	progressdialog.cpp \
	numslider.cpp \
	mybookmarkmanager.cpp \
	mainwindow.cpp  \
	tools.cpp \
	kexifpropsplugin.cpp \
	jpgoptions.cpp \
	imageloader.cpp \
	imagefileinfo.cpp \
	history_action.cpp \
	formatconversion.cpp \
	extract.cpp \
	krar.cpp \
	exif.cpp \
	displaycompare.cpp \
	directoryview.cpp \
	categoryview.cpp \
	cdarchiveview.cpp \
	listitemview.cpp \
	directory.cpp \
	describealbum.cpp \
	describe.cpp \
	confshowimg.cpp \
	batchrenamer.cpp \
	imagemetainfo.cpp \
	cdarchive.cpp \
	cdarchivecreator.cpp \
	cdarchivecreatordialog.cpp \
	categorydbmanager.cpp \
	categorylistitem.cpp \
	categoryimagefileiconitem.cpp \
	ksidebar.cpp \
	imagelistviewsimple.cpp \
	osd.cpp

rcdir = $(kde_datadir)/showimg
rc_DATA = tips

messages:
	$(PREPARETIPS) tips > ../tips.cpp
