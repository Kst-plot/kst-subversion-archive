installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = convolve.la

convolve_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
convolve_la_LIBADD = $(GSL_LIBS)
convolve_la_SOURCES = convolve.cpp

METASOURCES=AUTO

install_DATA=convolve.xml
