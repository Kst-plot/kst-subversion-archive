SUBDIRS =  khexedit kipiplugins misc . mimetypes  pics icons

INCLUDES =  -I$(showimg_top_srcdir) $(all_includes)

lib_LTLIBRARIES = libshowimgcore.la

VER_INFO = -version-info 0:9:0

libshowimgcore_la_LDFLAGS = $(all_libraries)  $(KDE_RPATH) $(VER_INFO) -no-undefined

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


libshowimgcore_la_METASOURCES = AUTO

libshowimgcore_la_SOURCES = \
	album.cpp \
	albumimagefileiconitem.cpp \
	batchrenamer.cpp \
	categorydbmanager.cpp \
	categoryimagefileiconitem.cpp \
	categorylistitem.cpp \
	categoryview.cpp \
	categoryimageproperties.cpp \
	categoryproperties.cpp \
	cdarchive.cpp \
	cdarchivecreator.cpp \
	cdarchivecreatordialog.cpp \
	cdarchiveview.cpp \
	compressedfileitem.cpp \
	compressedimagefileiconitem.cpp \
	confshowimg.cpp \
	describealbum.cpp \
	describe.cpp \
	directory.cpp \
	directoryview.cpp \
	dirfileiconitem.cpp \
	displaycompare.cpp \
	extract.cpp \
	fileiconitem.cpp \
	formatconversion.cpp \
	history_action.cpp \
	imagefileiconitem.cpp \
	imagefileinfo.cpp \
	imagelistview.cpp \
	imagelistviewsimple.cpp \
	imageloader.cpp \
	imagemetainfo.cpp \
	imageviewer.cpp \
	kexifpropsplugin.cpp \
	khexeditpropsplugin.cpp \
	krar.cpp \
	ksidebar.cpp \
	kstartuplogo.cpp \
	listitem.cpp \
	listitemview.cpp \
	mainwindow.cpp  \
	numslider.cpp \
	osd.cpp \
	printimagedialog.cpp \
	renameseries.cpp \
	tools.cpp \
	viewer.cpp

rcdir = $(kde_datadir)/showimg
rc_DATA = tips

messages:
	$(PREPARETIPS) tips > ../tips.cpp
