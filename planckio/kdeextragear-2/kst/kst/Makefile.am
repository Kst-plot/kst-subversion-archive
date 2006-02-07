
SUBDIRS=.

METASOURCES=AUTO

bin_PROGRAMS = kst d2asc

d2asc_LDFLAGS   =  $(all_libraries) $(KDE_RPATH)
d2asc_LDADD = $(LIB_KDECORE) $(KST_LIBS)

d2asc_SOURCES = \
	planck.cpp \
	plancktoi.cpp \
	d2asc.cpp \
	kstfile.cpp \
	getdata.c \
	readdata.c \
	creaddata.c \
	kstobject.cpp \
	kstdataobject.cpp \
	kstdatacollection.cpp \
	kstdatacollection-nogui.cpp \
	kstvector.cpp \
	kstrvector.cpp \
	kstscalar.cpp

kst_SOURCES = \
	planck.cpp \
	plancktoi.cpp \
	kstiface.skel \
	kstiface_impl.cpp \
	kstobject.cpp \
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
	kstmouse.cpp \
	kstlabel.cpp \
	kstvcurve.cpp \
	kstvector.cpp \
	kstfile.cpp \
	kstview.cpp \
	genterStacks.cpp \
	ksthistogram.cpp \
	getdata.c \
	readdata.c \
	creaddata.c \
	fftsg_h.c \
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
	kstplotlist.cpp \
	kstplot.cpp \
	kstscalar.cpp \
	kstdoc.cpp \
	kst.cpp \
	main.cpp

kst_LDADD = $(LIB_KIO) $(LIB_KDEPRINT) $(KST_LIBS)
kst_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_ICON = kst

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Applications/Sciences

iconsdir = $(kde_icondir)/hicolor/22x22/actions/
icons_DATA = kst_zoomtie.png kst_zoomxy.png kst_zoomx.png kst_zoomy.png \
	kst_changefile.png kst_changenpts.png kst_editplots.png \
	kst_datamanager.png kst_back.png kst_advance.png kst_quickcurves.png \
	kst_quickpsd.png kst_graphfile.png 

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

