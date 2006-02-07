installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstinterp_akima.la

kstinterp_akima_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstinterp_akima_la_LIBADD = $(GSL_LIBS)
kstinterp_akima_la_SOURCES = kstinterp_akima.cpp

METASOURCES=AUTO

install_DATA=kstinterp_akima.xml