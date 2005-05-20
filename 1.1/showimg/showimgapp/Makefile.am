bin_PROGRAMS = showimg

showimg_SOURCES = main.cpp

showimg_LDADD   = \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KDEPRINT) $(LIB_QT) \
	$(LIBJPEG)  \
	../showimg/libshowimgcore.la

# the library search path.
showimg_LDFLAGS = $(all_libraries)


# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/showimg $(all_includes)

METASOURCES = AUTO

# this is where the kdelnk file will go
kdelnkdir   = $(kde_appsdir)/Graphics
kdelnk_DATA = showimg.desktop

# file for konqueror integration
konqservicedir = $(kde_datadir)/konqueror/servicemenus
konqservice_DATA = konqshowimg.desktop

KDE_ICON = AUTO

rcdir = $(kde_datadir)/showimg
rc_DATA = showimgui.rc
