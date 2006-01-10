bin_PROGRAMS = showimg

# set the include path for X, qt and KDE
INCLUDES = -I$(showimg_top_srcdir) -I$(showimg_top_srcdir)/kexi $(all_includes)

showimg_SOURCES = main.cpp

showimg_LDADD   = \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KDEPRINT) $(LIB_QT) \
	$(LIBJPEG)  \
	../showimg/libshowimgcore.la

# the library search path.
showimg_LDFLAGS = $(KDE_RPATH)  $(all_libraries)


METASOURCES = AUTO

# this is where the kdelnk file will go
kdelnkdir   = $(kde_appsdir)/Graphics
kdelnk_DATA = showimg.desktop

# file for konqueror integration
konqservicedir = $(kde_datadir)/konqueror/servicemenus
konqservice_DATA = konqshowimg.desktop

KDE_ICON = AUTO

rcdir = $(kde_datadir)/showimg
rc_DATA = showimgui.rc showimgsimpleui.rc
