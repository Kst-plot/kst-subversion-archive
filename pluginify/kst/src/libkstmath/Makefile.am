
AM_YFLAGS = -d
AM_LFLAGS = 

#AM_CXXFLAGS = -Wold-style-cast -Wredundant-decls -Wunused-macros -Wfloat-equal -Wno-multichar

lib_LTLIBRARIES = libkstmath.la

libkstmath_la_SOURCES = \
	kstpainter.cpp \
	kstcolorsequence.cpp \
	kstsettings.cpp \
	kstobjectdefaults.cpp \
	labelparser.cpp \
	kstdataobject.cpp \
	kstdataobjectcollection.cpp \
	kstcsd.cpp \
	kstpsd.cpp \
	kstbasecurve.cpp \
	kstcurvepointsymbol.cpp \
	kstlinestyle.cpp \
	kstvcurve.cpp \
	kstimage.cpp \
	kstdefaultnames.cpp \
	ksthistogram.cpp \
	kstequation.cpp \
	kstcurvehint.cpp \
	kstcplugin.cpp \
	kstbasicplugin.cpp \
	fftsg_h.c \
	enodes.cpp \
	enodefactory.cpp \
	plugincollection.cpp \
	plugin.cpp \
	pluginxmlparser.cpp \
	pluginloader.cpp \
	dialoglauncher.cpp \
	eparse-eh.cpp \
	eparse.c \
	psdcalculator.cpp \
	escan.c

libkstmath_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ $(all_libraries) -no-undefined
libkstmath_la_LIBADD = $(LIB_KDECORE) ../libkst/libkst.la

# set the include path for X, qt and KDE
INCLUDES=-I$(top_srcdir)/kst/src/extdate -I$(top_srcdir)/kst/src/libkst -I../libkst -I$(top_srcdir) $(KMDI_INCLUDES) $(all_includes)

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kst.pot; \
	fi

METASOURCES=AUTO

