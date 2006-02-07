installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=crosscorrelation.la

crosscorrelation_la_LDFLAGS=-module $(KDE_PLUGIN) $(all_libraries)
crosscorrelation_la_SOURCES=crosscorrelation.cpp

METASOURCES=AUTO

install_DATA=crosscorrelation.xml
