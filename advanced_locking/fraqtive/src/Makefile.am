SUBDIRS = pics

INCLUDES = $(all_includes)

noinst_HEADERS = fraqtive.h generator.h genthread.h gradbutton.h \
	gradeditor.h gradient.h gradlist.h gradwidget.h presets.h spline.h \
	view2d.h view3d.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/fraqtive.pot

KDE_ICON = fraqtive

bin_PROGRAMS = fraqtive

fraqtive_SOURCES = main.cpp fraqtive.cpp generator.cpp genthread.cpp \
	gradbutton.cpp gradeditor.cpp gradient.cpp gradlist.cpp gradwidget.cpp \
	presets.cpp spline.cpp view2d.cpp view3d.cpp

fraqtive_LDFLAGS = $(KDE_RPATH) $(all_libraries)

fraqtive_LDADD = $(GLLIB) $(LIB_KDEUI) $(LIB_KFILE)

shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = fraqtive.desktop

shellrcdir   = $(kde_datadir)/fraqtive
shellrc_DATA = fraqtiveui.rc

kde_conf_DATA = fraqtiverc
