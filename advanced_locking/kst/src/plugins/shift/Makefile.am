installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=shift.la

shift_la_LDFLAGS=-module $(KDE_PLUGIN) $(all_libraries)
shift_la_SOURCES=shift.c

METASOURCES=AUTO

install_DATA=shift.xml
