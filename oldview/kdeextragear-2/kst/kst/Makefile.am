
AM_YFLAGS = -d
AM_LFLAGS = 

SUBDIRS=. datasources extensions

METASOURCES=AUTO

bin_PROGRAMS = kst d2asc

lib_LTLIBRARIES = libkst.la
libkst_la_SOURCES = kstobject.cpp kstdatasource.cpp stdinsource.cpp kstextension.cpp kstdatacollection.cpp kstdebug.cpp rwlock.cpp
libkst_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
libkst_la_LIBADD = $(LIB_KIO)

d2asc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
d2asc_LDADD = libkst.la

d2asc_SOURCES = \
	d2asc.cpp \
	kstdataobject.cpp \
	kstdatacollection-nogui.cpp \
	kstvector.cpp \
	kstrvector.cpp \
	ksteventmonitorentry.cpp \
	kstscalar.cpp \
	enodes.cpp \
	enodefactory.cpp \
	exparse.c \
	exscan.c

kst_SOURCES = \
	kstiface.skel \
	kstiface_impl.cpp \
	variableeditor.cpp \
	kstdataobject.cpp \
	kstscalartable.cpp \
	kstvectortable.cpp \
	ksteventtable.cpp \
	kstlogtable.cpp \
	kstplugin.cpp \
	kstplugindialog_i.cpp \
	plugin.cpp \
	pluginxmlparser.cpp \
	pluginloader.cpp \
	plugincollection.cpp \
	extensiondlg.ui \
	extensionmgr.cpp \
	kstcolorsequence.cpp \
	kstdatacollection-gui.cpp \
	kstdatamanager_i.cpp \
	kstvectordialog_i.cpp \
	kstcurvedialog_i.cpp \
	ksteqdialog_i.cpp \
	ksthsdialog_i.cpp \
	kstpsddialog_i.cpp \
	kstgraphfiledialog_i.cpp \
	updatethread.cpp \
	enodes.cpp \
	enodefactory.cpp \
	kstequationcurve.cpp \
	kstquickpsddialog_i.cpp \
	kstquickcurvesdialog_i.cpp \
	kstviewscalarsdialog_i.cpp \
	kstviewvectorsdialog_i.cpp \
	kstchangenptsdialog_i.cpp \
	kstchangefiledialog_i.cpp \
	ksteventmonitor_i.cpp \
	kstplotdialog_i.cpp \
 	kstdebugdialog_i.cpp \
	kstlabeldialog_i.cpp \
	kstpsdcurve.cpp \
	kstbasecurve.cpp \
	kstrvector.cpp \
	kstpoint.cpp \
	kstfilter.cpp \
	kstmouse.cpp \
	kstlabel.cpp \
	kstlegend.cpp \
	kstvcurve.cpp \
	kstvector.cpp \
	kstfilteredvector.cpp \
	ksteventmonitorentry.cpp \
	kstview.cpp \
	ksthistogram.cpp \
	fftsg_h.c \
	eventmonitor.ui \
	debugdialog.ui \
	datawizard.ui \
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
	viewscalarsdialog.ui \
	viewvectorsdialog.ui \
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
	kstdoc.cpp \
	kst.cpp \
	main.cpp \
	exparse.c \
	eparse.c \
	escan.c \
	exscan.c
#	exparse.y \
#	eparse.y \
#	escan.l

include_HEADERS = kstdatasource.h kstobject.h

kst_LDADD = $(LIB_KDEPRINT) libkst.la
kst_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_ICON = kst

services_DATA = kstplugin.desktop kstdatasourceplugin.desktop kstfilter.desktop kstextension.desktop
servicesdir = $(kde_servicetypesdir)/kst

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Applications/Sciences

iconsdir = $(kde_icondir)/hicolor/22x22/actions/
icons_DATA = kst_zoomtie.png kst_zoomxy.png kst_zoomx.png kst_zoomy.png \
	kst_changefile.png kst_changenpts.png kst_editplots.png \
	kst_datamanager.png kst_back.png kst_advance.png kst_quickcurves.png \
	kst_quickpsd.png kst_graphfile.png kst_datamode.png kst_datawizard.png

mimedir = $(kde_mimedir)/application
mime_DATA = x-kst.desktop

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


