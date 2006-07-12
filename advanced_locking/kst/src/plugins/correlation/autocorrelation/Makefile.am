installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = autocorrelate.la

autocorrelate_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
autocorrelate_la_LIBADD = $(GSL_LIBS)
autocorrelate_la_SOURCES = autocorrelate.cpp

METASOURCES=AUTO

install_DATA=autocorrelate.xml
