
SUBDIRS=. kstXbench

check_PROGRAMS=testeqparser testhistogram testscalars testlabelparser \
		testvector testrvector

METASOURCES=AUTO

INCLUDES=-I$(top_srcdir)/kst/kst $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(KDE_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
LDADD = $(LIB_KDECORE) ../kst/libkstmath.la ../kst/libkst.la ../kst/libkstnonguiapp.la
testeqparser_SOURCES=testeqparser.cpp
testhistogram_SOURCES=testhistogram.cpp
testscalars_SOURCES=testscalars.cpp
testlabelparser_SOURCES=testlabelparser.cpp
testvector_SOURCES=testvector.cpp
testrvector_SOURCES=testrvector.cpp

