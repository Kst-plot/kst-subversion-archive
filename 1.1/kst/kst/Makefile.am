
AM_YFLAGS = -d
AM_LFLAGS = 

#AM_CFLAGS=-fprofile-arcs -ftest-coverage -O0 -fno-inline
#AM_CXXFLAGS=-fprofile-arcs -ftest-coverage -O0 -fno-inline

if include_libkstkmdi
KMDI_SUBDIR=libkstkmdi
endif
if include_linux
PROCPS_COPY=sysinfo.c psversion.c
endif

SUBDIRS=$(KMDI_SUBDIR) . datasources extensions pics

bin_PROGRAMS = kst d2asc

lib_LTLIBRARIES = libkst.la libkstmath.la libkstguiapp.la libkstapp.la
libkst_la_SOURCES = \
	kstobject.cpp \
	kstdatasource.cpp \
	stdinsource.cpp \
	kstextension.cpp \
	kstdatacollection.cpp \
	kstdebug.cpp \
	rwlock.cpp \
	kstscalar.cpp $(PROCPS_COPY) \
	kststring.cpp

libkst_la_LDFLAGS = -version-info 1:1:0 -no-undefined $(all_libraries)
libkst_la_LIBADD = $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE)

noinst_LTLIBRARIES = libkstplot.la libkstnonguiapp.la
libkstmath_la_SOURCES = \
	labelparser.cpp \
	kstdataobject.cpp \
	kstpsd.cpp \
	kstbasecurve.cpp \
	kstpoint.cpp \
	kstvcurve.cpp \
	kstmatrix.cpp \
	kstimage.cpp \
	kstvectordefaults.cpp \
	kstdefaultnames.cpp \
	ksthistogram.cpp \
	kstequation.cpp \
	kstcurvehint.cpp \
	kstplugin.cpp \
	fftsg_h.c \
	kstvector.cpp \
	kstrvector.cpp \
	kstsvector.cpp \
	kstavector.cpp \
	enodes.cpp \
	enodefactory.cpp \
	plugincollection.cpp \
	plugin.cpp \
	pluginxmlparser.cpp \
	pluginloader.cpp \
	eparse-eh.cpp \
	eparse.c \
	escan.c

libkstmath_la_LDFLAGS = -version-info 1:1:0 $(all_libraries)
libkstmath_la_LIBADD = $(LIB_KDECORE) libkst.la

d2asc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
d2asc_LDADD = libkstmath.la libkst.la libkstnonguiapp.la
d2asc_SOURCES = d2asc.cpp


libkstplot_la_SOURCES = \
	kstbackbuffer.cpp

libkstplot_la_LDFLAGS = -no-undefined $(all_libraries)

libkstguiapp_la_SOURCES = kstdatacollection-gui.cpp \
	dialoglauncher-gui.cpp \
	kstcolorsequence.cpp \
	kstuinames.cpp \
	vectordialog.ui \
	stringeditor.ui \
	scalarselector.ui \
	scalareditor.ui \
	vectorselector.ui \
	datarangewidget.ui \
	fftoptionswidget.ui \
	matrixselector.ui \
	curveplacementwidget.ui \
	curveappearancewidget.ui \
	eqdialog.ui \
	hsdialog.ui \
	psddialog.ui \
	imagedialog.ui \
	matrixdialog.ui \
	plugindialog.ui \
	pluginmanager.ui \
	curvedialog.ui

libkstguiapp_la_LDFLAGS = -version-info 1:1:0 $(all_libraries)
#libkstguiapp_la_LIBADD = $(LIB_KIO) libkstmath.la libkst.la
libkstguiapp_la_LIBADD = $(LIB_KIO) libkstmath.la

libkstnonguiapp_la_SOURCES = kstdatacollection-nogui.cpp dialoglauncher-nogui.cpp
libkstnonguiapp_la_LDFLAGS = -no-undefined $(all_libraries)
libkstnonguiapp_la_LIBADD = $(LIB_KIO) libkstmath.la libkst.la

libkstapp_la_SOURCES = \
	kstwaitcondition_unix.cpp \
	kstiface.skel \
	kstiface_impl.cpp \
	kstcombobox.cpp \
	draggablelistbox.cpp \
	plotlistbox.cpp \
	plotmimesource.cpp \
	kstplotdrag.cpp \
	kstplotimagedrag.cpp \
	kstscalartable.cpp \
	kstvectortable.cpp \
	kstmatrixtable.cpp \
	kstfittable.cpp \
	kstlogwidget.cpp \
	kstplotbase.cpp \
	kst2dplot.cpp \
	kstviewwidget.cpp \
	kstviewobjectfactory.cpp \
	kstviewobject.cpp \
	kstplotgroup.cpp \
	kstborderedviewobject.cpp \
	ksttoplevelview.cpp \
	kstmetaplot.cpp \
	kstdrag.cpp \
	kstplugindialog_i.cpp \
	kstplotdialog_i.cpp \
	kstfitdialog_i.cpp \
	kstfilterdialog_i.cpp \
	extensionmgr.cpp \
	kstdatamanager_i.cpp \
	kstvectordialog_i.cpp \
	ksteqdialog_i.cpp \
	ksthsdialog_i.cpp \
	kstpsddialog_i.cpp \
	kstgraphfiledialog_i.cpp \
	kstimagedialog_i.cpp \
	kstquickstartdialog_i.cpp \
	updatethread.cpp \
	emailthread.cpp \
	kstprintoptionspage.cpp \
	kstviewscalarsdialog_i.cpp \
	kstviewvectorsdialog_i.cpp \
	kstviewmatricesdialog_i.cpp \
	kstviewfitsdialog_i.cpp \
	kstchangenptsdialog_i.cpp \
	kstchangefiledialog_i.cpp \
	ksteventmonitor_i.cpp \
	kstdebugdialog_i.cpp \
	kstlabeldialog_i.cpp \
	kstmatrixdialog_i.cpp \
	kstlabel.cpp \
	kstlegend.cpp \
	kstalignment.cpp \
	ksteventmonitorentry.cpp \
	kstcurvedialog_i.cpp \
	kstsettings.cpp \
	kstdoc.cpp \
	kstviewwindow.cpp \
	plotdialog.ui \
	extensiondlg.ui \
	kstdatadialog.ui \
	stringselector.ui \
	eventmonitor.ui \
	debugdialog.ui \
	datawizard.ui \
	changefiledialog.ui \
	graphfiledialog.ui \
	viewscalarsdialog.ui \
	viewvectorsdialog.ui \
	viewmatricesdialog.ui \
	viewfitsdialog.ui \
	changenptsdialog.ui \
	datamanager.ui \
	vectorsavedialog.ui \
	kstlabeldialog.ui \
	kstsettingsdlg.ui \
	quickstartdialog.ui \
	kst.cpp

include_HEADERS = kstdatasource.h kstobject.h kst_export.h kstsharedptr.h rwlock.h

libkstapp_la_LIBADD = $(LIB_KDEPRINT) libkstplot.la libkstguiapp.la $(LIB_KMDI)
libkstapp_la_LDFLAGS = -version-info 1:1:0 $(all_libraries)

kst_SOURCES = main.cpp
kst_LDADD = libkstapp.la
kst_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_ICON = kst

services_DATA = kstplugin.desktop kstdatasourceplugin.desktop kstfilter.desktop kstextension.desktop
servicesdir = $(kde_servicetypesdir)/kst

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Applications/Sciences

iconsdir = $(kde_icondir)/hicolor/22x22/actions/
icons_DATA = kst_zoomtie.png kst_zoomxy.png kst_zoomx.png kst_zoomy.png \
	kst_changefile.png kst_changenpts.png kst_editplots.png \
	kst_datamanager.png kst_back.png kst_advance.png \
	kst_graphfile.png kst_layoutmode.png kst_datamode.png \
	kst_datawizard.png kst_vectornew.png kst_matrixnew.png \
	kst_curvenew.png kst_psdnew.png kst_histogramnew.png\
	kst_equationnew.png kst_imagenew.png kst_pluginnew.png \
	kst_eventnew.png

mimedir = $(kde_mimedir)/application
mime_DATA = x-kst.desktop

# set the include path for X, qt and KDE
INCLUDES= $(KMDI_INCLUDES) -I$(top_srcdir) $(all_includes)

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
matrixselector.o: matrixselector.ui.h matrixselector.ui
vectorselector.o: vectorselector.ui.h vectorselector.ui
scalarselector.o: scalarselector.ui.h scalarselector.ui
stringselector.o: stringselector.ui.h stringselector.ui
vectorsavedialog.o: vectorsavedialog.ui.h vectorsavedialog.ui
kstsettingsdlg.o: kstsettingsdlg.ui.h kstsettingsdlg.ui
datawizard.o: datawizard.ui.h datawizard.ui
debugdialog.o: debugdialog.ui.h debugdialog.ui
kstdatadialog.o: kstdatadialog.ui.h kstdatadialog.ui
fftoptionswidget.o: fftoptionswidget.ui.h fftoptionswidget.ui

METASOURCES=AUTO

update_DATA = kstautosave11.upd kstrcmisc11.upd
updatedir = $(kde_datadir)/kconf_update

