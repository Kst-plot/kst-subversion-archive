installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=chop.la

chop_la_LDFLAGS=-module $(KDE_PLUGIN) $(all_libraries)
chop_la_SOURCES=chop.cpp

METASOURCES=AUTO

install_DATA=chop.xml
