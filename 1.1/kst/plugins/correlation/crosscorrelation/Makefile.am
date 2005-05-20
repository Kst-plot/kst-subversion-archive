installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = crosscorrelate.la

crosscorrelate_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
crosscorrelate_la_LIBADD = $(GSL_LIBS)
crosscorrelate_la_SOURCES = crosscorrelate.cpp

METASOURCES=AUTO

install_DATA=crosscorrelate.xml
