installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=phase.la

phase_la_LDFLAGS=-module $(KDE_PLUGIN) $(all_libraries)
phase_la_SOURCES=phase.cpp

METASOURCES=AUTO

install_DATA=phase.xml
