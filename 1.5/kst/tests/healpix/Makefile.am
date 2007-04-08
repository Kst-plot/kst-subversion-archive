
check_PROGRAMS=testhealpix

METASOURCES=AUTO

INCLUDES=-I$(top_srcdir)/kst/src/datasources/healpix -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath -I$(top_srcdir)/kst/tests $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(KDE_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
LDADD = $(LIB_KDECORE) ../../src/libkstmath/libkstmath.la ../../src/libkst/libkst.la ../../src/datasources/healpix/kstdata_healpix.la #../../src/libkstnonguiapp.la 
testhealpix_SOURCES=testhealpix.cpp

