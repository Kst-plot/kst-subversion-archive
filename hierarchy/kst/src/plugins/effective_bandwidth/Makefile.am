installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../kst $(all_includes)

install_LTLIBRARIES = effective_bandwidth.la

effective_bandwidth_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
effective_bandwidth_la_LIBADD = $(GSL_LIBS)
effective_bandwidth_la_SOURCES = effective_bandwidth.cpp

METASOURCES=AUTO

install_DATA=effective_bandwidth.xml
