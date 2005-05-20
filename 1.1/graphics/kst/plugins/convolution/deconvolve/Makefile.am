installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = deconvolve.la

deconvolve_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
deconvolve_la_LIBADD = $(GSL_LIBS)
deconvolve_la_SOURCES = deconvolve.cpp

METASOURCES=AUTO

install_DATA=deconvolve.xml
