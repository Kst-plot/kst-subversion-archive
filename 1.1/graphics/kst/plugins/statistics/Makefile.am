installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=statistics.la

statistics_la_LDFLAGS=-module $(KDE_PLUGIN) $(all_libraries)
statistics_la_SOURCES=statistics.cpp

METASOURCES=AUTO

install_DATA=statistics.xml
