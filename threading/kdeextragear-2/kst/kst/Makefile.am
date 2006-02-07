
SUBDIRS=. datasources

METASOURCES=AUTO

bin_PROGRAMS = kst d2asc

lib_LTLIBRARIES = libkst.la
libkst_la_SOURCES = kstobject.cpp kstdatasource.cpp stdinsource.cpp rwlock.cpp
libkst_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
libkst_la_LIBADD = $(LIB_KIO)

d2asc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
d2asc_LDADD = libkst.la

d2asc_SOURCES = \
	d2asc.cpp \
	kstdataobject.cpp \
	kstdatacollection.cpp \
	kstdatacollection-nogui.cpp \
	kstvector.cpp \
	kstrvector.cpp \
	kstscalar.cpp

kst_SOURCES = \
	kstiface.skel \
	kstiface_impl.cpp \
	kstdataobject.cpp \
	kstplugin.cpp \
	kstplugindialog_i.cpp \
	plugin.cpp \
	pluginxmlparser.cpp \
	pluginloader.cpp \
	plugincollection.cpp \
	kstcolorsequence.cpp \
	kstdatacollection.cpp \
	kstdatacollection-gui.cpp \
	kstdatamanager_i.cpp \
	kstvectordialog_i.cpp \
	kstcurvedialog_i.cpp \
	ksteqdialog_i.cpp \
	ksthsdialog_i.cpp \
	kstpsddialog_i.cpp \
	kstgraphfiledialog_i.cpp \
	kstequationcurve.cpp \
	kstquickpsddialog_i.cpp \
	kstquickcurvesdialog_i.cpp \
	kstchangenptsdialog_i.cpp \
	kstchangefiledialog_i.cpp \
	kstplotdialog_i.cpp \
	kstlabeldialog_i.cpp \
	kstpsdcurve.cpp \
	kstbasecurve.cpp \
	kstrvector.cpp \
	kstpoint.cpp \
	kstfilter.cpp \
	kstmouse.cpp \
	kstlabel.cpp \
	kstvcurve.cpp \
	kstvector.cpp \
	kstfilteredvector.cpp \
	kstview.cpp \
	genterStacks.cpp \
	ksthistogram.cpp \
	fftsg_h.c \
	variablelisteditor.ui \
	filterlisteditor.ui \
	scalarselector.ui \
	vectorselector.ui \
	scalareditor.ui \
	curveappearancewidget.ui \
	curveplacementwidget.ui \
	changefiledialog.ui \
	plotdialog.ui \
	graphfiledialog.ui \
	quickpsddialog.ui \
	quickcurvesdialog.ui \
	changenptsdialog.ui \
	vectordialog.ui \
	curvedialog.ui \
	eqdialog.ui \
	hsdialog.ui \
	psddialog.ui \
	datamanager.ui \
	plugindialog.ui \
	pluginmanager.ui \
	vectorsavedialog.ui \
	kstlabeldialog.ui \
	kstsettingsdlg.ui \
	kstplotlist.cpp \
	kstplot.cpp \
	kstscalar.cpp \
        kstsettings.cpp \
	updatethread.cpp \
	kstdoc.cpp \
	kst.cpp \
	main.cpp

include_HEADERS = kstdatasource.h kstobject.h

kst_LDADD = $(LIB_KDEPRINT) libkst.la
kst_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_ICON = kst

services_DATA = kstplugin.desktop kstdatasourceplugin.desktop kstfilter.desktop
servicesdir = $(kde_servicetypesdir)/kst

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Applications/Sciences

iconsdir = $(kde_icondir)/hicolor/22x22/actions/
icons_DATA = kst_zoomtie.png kst_zoomxy.png kst_zoomx.png kst_zoomy.png \
	kst_changefile.png kst_changenpts.png kst_editplots.png \
	kst_datamanager.png kst_back.png kst_advance.png kst_quickcurves.png \
	kst_quickpsd.png kst_graphfile.png kst_datamode.png

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

rcdir = $(kde_datadir)/kst
rc_DATA = kstui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kst.pot; \
	fi

curveappearancewidget.o: curveappearancewidget.ui.h curveappearancewidget.ui
curveplacementwidget.o: curveplacementwidget.ui.h curveplacementwidget.ui
pluginmanager.o: pluginmanager.ui.h pluginmanager.ui
vectorselector.o: vectorselector.ui.h vectorselector.ui
scalarselector.o: scalarselector.ui.h scalarselector.ui
vectorsavedialog.o: vectorsavedialog.ui.h vectorsavedialog.ui
filterlisteditor.o: filterlisteditor.ui.h filterlisteditor.ui
kstsettingsdlg.o: kstsettingsdlg.ui.h kstsettingsdlg.ui

