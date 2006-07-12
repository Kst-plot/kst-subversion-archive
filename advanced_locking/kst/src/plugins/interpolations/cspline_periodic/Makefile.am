installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstinterp_cspline_periodic.la

kstinterp_cspline_periodic_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstinterp_cspline_periodic_la_LIBADD = $(GSL_LIBS)
kstinterp_cspline_periodic_la_SOURCES = kstinterp_cspline_periodic.cpp

METASOURCES=AUTO

install_DATA=kstinterp_cspline_periodic.xml
