installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstinterp_linear.la

kstinterp_linear_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstinterp_linear_la_LIBADD = $(GSL_LIBS)
kstinterp_linear_la_SOURCES = kstinterp_linear.cpp

METASOURCES=AUTO

install_DATA=kstinterp_linear.xml
