installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=periodogram.la

periodogram_la_LDFLAGS=-module $(KDE_PLUGIN) $(all_libraries)
periodogram_la_SOURCES=periodogram.cpp

METASOURCES=AUTO

install_DATA=periodogram.xml
