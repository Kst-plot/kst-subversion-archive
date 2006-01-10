
AM_YFLAGS = -d
AM_LFLAGS = 

#AM_CXXFLAGS = -Wold-style-cast -Wredundant-decls -Wunused-macros -Wfloat-equal -Wno-multichar

if include_libkstkmdi
KMDI_SUBDIR=libkstkmdi
endif
if include_linux
PROCPS_COPY=sysinfo.c psversion.c
endif

SUBDIRS=extdate $(KMDI_SUBDIR) . datasources extensions pics

bin_PROGRAMS = kst d2asc

lib_LTLIBRARIES = libkst.la libkstmath.la libkstwidgets.la libkstguiapp.la libkstapp.la

kde_widget_LTLIBRARIES = kstwidgets.la

libkst_la_SOURCES = \
	ksdebug.cpp \
	kstobject.cpp \
	kstdatasource.cpp \
	stdinsource.cpp \
	kstextension.cpp \
	kstdatacollection.cpp \
	kstdebug.cpp \
	rwlock.cpp \
	kstdateparser.cpp \
	ksttimezones.cpp \
	kstsettings.cpp \
	kstscalar.cpp $(PROCPS_COPY) \
	kststring.cpp

libkst_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ -no-undefined $(all_libraries)
libkst_la_LIBADD = $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE) extdate/libkstextdate.la

noinst_LTLIBRARIES = libkstplot.la libkstnonguiapp.la
libkstmath_la_SOURCES = \
	labelparser.cpp \
	kstdataobject.cpp \
	kstcsd.cpp \
	kstpsd.cpp \
	kstbasecurve.cpp \
	kstpoint.cpp \
	kstlinestyle.cpp \
	kstvcurve.cpp \
	kstimage.cpp \
	kstvectordefaults.cpp \
	kstmatrixdefaults.cpp \
	kstdefaultnames.cpp \
	ksthistogram.cpp \
	kstequation.cpp \
	kstcurvehint.cpp \
	kstplugin.cpp \
	kstpsdgenerator.cpp \
	fftsg_h.c \
	kstvector.cpp \
	kstrvector.cpp \
	kstsvector.cpp \
	kstavector.cpp \
	kstmatrix.cpp \
	kstrmatrix.cpp \
	kstsmatrix.cpp \
	kstamatrix.cpp \
	enodes.cpp \
	enodefactory.cpp \
	plugincollection.cpp \
	plugin.cpp \
	pluginxmlparser.cpp \
	pluginloader.cpp \
	eparse-eh.cpp \
	eparse.c \
	escan.c

libkstmath_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ $(all_libraries)
libkstmath_la_LIBADD = $(LIB_KDECORE) libkst.la

d2asc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
d2asc_LDADD = libkstmath.la libkst.la libkstnonguiapp.la
d2asc_SOURCES = d2asc.cpp


libkstplot_la_SOURCES = \
	kstbackbuffer.cpp

libkstplot_la_LDFLAGS = -no-undefined $(all_libraries)

libkstwidgets_la_SOURCES = \
	comboboxselection.ui \
	comboboxselection_i.cpp \
	stringeditor.ui \
	scalareditor.ui \
	kstcolorsequence.cpp \
	scalarselector.ui \
	matrixselector.ui \
	vectorselector.ui \
	stringselector.ui \
	editmultiplewidget.ui \
	curveplacementwidget.ui \
	curveappearancewidget.ui \
	fftoptionswidget.ui \
	draggablelistbox.cpp \
	plotlistbox.cpp \
	kstcombobox.cpp \
	datarangewidget.ui

libkstwidgets_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ $(all_libraries)
libkstwidgets_la_LIBADD = $(LIB_KIO) libkstmath.la


libkstguiapp_la_SOURCES = kstdatacollection-gui.cpp \
	dialoglauncher-gui.cpp \
	labelrenderer.cpp \
	kstnumbersequence.cpp \
	kstuinames.cpp \
	editviewobjectdialog.ui \
	vectordialogwidget.ui \
	kstcurvedifferentiate_i.cpp \
	curvedifferentiate.ui \
	csddialogwidget.ui \
	eqdialogwidget.ui \
	histogramdialogwidget.ui \
	psddialogwidget.ui \
	imagedialogwidget.ui \
	matrixdialogwidget.ui \
	plugindialogwidget.ui \
	eventmonitorwidget.ui \
	pluginmanager.ui \
	curvedialogwidget.ui

libkstguiapp_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ $(all_libraries)
libkstguiapp_la_LIBADD = libkstwidgets.la

libkstnonguiapp_la_SOURCES = kstdatacollection-nogui.cpp dialoglauncher-nogui.cpp
libkstnonguiapp_la_LDFLAGS = -no-undefined $(all_libraries)
libkstnonguiapp_la_LIBADD = $(LIB_KIO) libkstmath.la libkst.la

libkstapp_la_SOURCES = \
	kstpainter.cpp \
	kstwaitcondition_unix.cpp \
	kstiface.skel \
	kstiface_impl.cpp \
	statuslabel.cpp \
	plotmimesource.cpp \
	kstplotdrag.cpp \
	kstviewobjectimagedrag.cpp \
	kstscalartable.cpp \
	kstvectortable.cpp \
	kstmatrixtable.cpp \
	kstfittable.cpp \
	kstlogwidget.cpp \
	kstplotbase.cpp \
	viewlabelwidget.ui \
	viewlegendwidget.ui \
	kstviewlabel.cpp \
	kstplotlabel.cpp \
	kst2dplot.cpp \
	kstviewlegend.cpp \
	kstviewbox.cpp \
	kstviewline.cpp \
	kstviewellipse.cpp \
	kstviewpicture.cpp \
	kstviewbezier.cpp \
	kstviewarrow.cpp \
	kstviewwidget.cpp \
	kstviewobjectfactory.cpp \
	kstviewobject.cpp \
	kstplotgroup.cpp \
	kstborderedviewobject.cpp \
	ksttoplevelview.cpp \
	kstgfxmousehandler.cpp \
	kstgfxmousehandlerutils.cpp \
	kstgfxrectanglemousehandler.cpp \
	kstgfxlinemousehandler.cpp \
	kstgfxtextmousehandler.cpp \
	kstgfxellipsemousehandler.cpp \
	kstgfxpicturemousehandler.cpp \
	kstgfxarrowmousehandler.cpp \
	kstmetaplot.cpp \
	kstdrag.cpp \
	kstplugindialog_i.cpp \
	kstplotdialog_i.cpp \
	kstfitdialog_i.cpp \
	kstfilterdialog_i.cpp \
	extensionmgr.cpp \
	ksteditviewobjectdialog_i.cpp \
	kstdatamanager_i.cpp \
	kstvectordialog_i.cpp \
	kstcsddialog_i.cpp \
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
	kstchoosecolordialog_i.cpp \
	kstchangenptsdialog_i.cpp \
	kstchangefiledialog_i.cpp \
	ksteventmonitor_i.cpp \
	kstdebugdialog_i.cpp \
	kstmatrixdialog_i.cpp \
	kstmonochromedialog_i.cpp \
	kstalignment.cpp \
	ksteventmonitorentry.cpp \
	kstcurvedialog_i.cpp \
	kstdoc.cpp \
	kstviewwindow.cpp \
	ktimezonecombo.cpp \
	plotdialog.ui \
	extensiondlg.ui \
	kstdatadialog.ui \
	choosecolordialog.ui \
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
	kstsettingsdlg.ui \
	quickstartdialog.ui \
	monochromedialog.ui \
	kstdebugnotifier.cpp \
	kstdatanotifier.cpp \
	kst.cpp

include_HEADERS = kstdatasource.h kstobject.h kst_export.h kstsharedptr.h rwlock.h kstdateparser.h

libkstapp_la_LIBADD = $(LIB_KDEPRINT) libkstplot.la libkstguiapp.la $(LIB_KMDI)
libkstapp_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ $(all_libraries)

kst_SOURCES = main.cpp
kst_LDADD = libkstapp.la
kst_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kstwidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
kstwidgets_la_LIBADD = libkstapp.la
kstwidgets_la_SOURCES = kstwidgets.cpp

KDE_ICON = kst

services_DATA = kstplugin.desktop kstdatasourceplugin.desktop kstfilter.desktop kstextension.desktop
servicesdir = $(kde_servicetypesdir)/kst

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Applications/Sciences

mimedir = $(kde_mimedir)/application
mime_DATA = x-kst.desktop

# set the include path for X, qt and KDE
INCLUDES= $(KMDI_INCLUDES) -I$(srcdir)/extdate -I$(top_srcdir) $(all_includes)

rcdir = $(kde_datadir)/kst
rc_DATA = kstui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kst.pot; \
	fi

changenptsdialog.lo: datarangewidget.h
csddialogwidget.lo: fftoptionswidget.h vectorselector.h curveplacementwidget.h
curveappearancewidget.lo: curveappearancewidget.ui.h curveappearancewidget.ui
curvedialogwidget.lo: vectorselector.h curveappearancewidget.h curveplacementwidget.h
curveplacementwidget.lo: curveplacementwidget.ui.h curveplacementwidget.ui
datawizard.lo: datawizard.ui.h datawizard.ui vectorselector.h fftoptionswidget.h datarangewidget.h
debugdialog.lo: debugdialog.ui.h debugdialog.ui
editmultiplewidget.lo: editmultiplewidget.ui.h editmultiplewidget.ui
editviewobjectdialog.lo: editviewobjectdialog.ui
eqdialogwidget.lo: curveplacementwidget.h curveappearancewidget.h vectorselector.h scalarselector.h comboboxselection.h
eventmonitorwidget.lo: scalarselector.h comboboxselection.h vectorselector.h
fftoptionswidget.lo: fftoptionswidget.ui.h fftoptionswidget.ui
histogramdialogwidget.lo: vectorselector.h curveappearancewidget.h curveplacementwidget.h
imagedialogwidget.lo: matrixselector.h curveplacementwidget.h
kst.lo: curvedifferentiate.h
kst2dplot.lo: plugindialogwidget.h matrixdialogwidget.h vectordialogwidget.h
kstcsddialog_i.lo: csddialogwidget.h
kstdatadialog.lo: editmultiplewidget.h kstdatadialog.ui.h kstdatadialog.ui
kstdatamanager_i.lo: curvedialogwidget.h histogramdialogwidget.h imagedialogwidget.h
ksteqdialog_i.lo: comboboxselection.h eqdialogwidget.h
ksteventmonitor_i.lo: comboboxselection.h eventmonitorwidget.h
kstfilterdialog_i.lo: comboboxselection.h plugindialogwidget.h
kstfitdialog_i.lo: comboboxselection.h plugindialogwidget.h
kstplotdialog_i.lo: comboboxselection.h
kstplugindialog_i.lo: comboboxselection.h pluginmanager.h plugindialogwidget.h
kstpsddialog_i.lo: psddialogwidget.h
kstsettingsdlg.lo: kstsettingsdlg.ui.h kstsettingsdlg.ui
kstviewbox.lo: editviewobjectdialog.h
kstviewlegend.lo: editviewobjectdialog.h
kstwidgets.lo: comboboxselection.h curveappearancewidget.h curveplacementwidget.h datarangewidget.h editmultiplewidget.h fftoptionswidget.h matrixselector.h scalarselector.h stringselector.h vectorselector.h
matrixselector.lo: matrixselector.ui.h matrixselector.ui
plugindialogwidget.lo: curveappearancewidget.h
pluginmanager.lo: pluginmanager.ui.h pluginmanager.ui
psddialogwidget.lo: fftoptionswidget.h curveappearancewidget.h vectorselector.h curveplacementwidget.h
scalarselector.lo: comboboxselection.h scalareditor.h scalarselector.ui.h scalarselector.ui
stringselector.lo: stringeditor.h stringselector.ui.h stringselector.ui
vectordialogwidget.lo: datarangewidget.h
vectorsavedialog.lo: vectorsavedialog.ui.h vectorsavedialog.ui
vectorselector.lo: vectorselector.ui.h vectorselector.ui
viewlegendwidget.lo: plotlistbox.h
viewmatricesdialog.lo: matrixselector.h
viewvectorsdialog.lo: vectorselector.h
kstdebug.lo: kstrevision.h

METASOURCES=AUTO

update_DATA = kstautosave11.upd kstrcmisc11.upd
updatedir = $(kde_datadir)/kconf_update

kstrevision.h:
	@if (which svn >/dev/null 2>&1); then \
	if (svn info $(srcdir) >/dev/null 2>&1); then \
	REV=`svn info $(srcdir) | grep ^Revision | cut -b 11-`; \
	else REV="dist"; fi; else REV="dist"; fi; \
	echo "#ifndef KSTREVISION" >kstrevision.h.tmp; \
	echo "#define KSTREVISION \""$REV"\"" >>kstrevision.h.tmp; \
	echo "#endif" >>kstrevision.h.tmp; \
	if (diff -N kstrevision.h.tmp kstrevision.h >/dev/null 2>&1); \
	then rm kstrevision.h.tmp; else mv kstrevision.h.tmp kstrevision.h; fi

