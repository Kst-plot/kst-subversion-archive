installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstinterp_cspline.la

kstinterp_cspline_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstinterp_cspline_la_LIBADD = $(GSL_LIBS)
kstinterp_cspline_la_SOURCES = kstinterp_cspline.cpp

METASOURCES=AUTO

install_DATA=kstinterp_cspline.xml