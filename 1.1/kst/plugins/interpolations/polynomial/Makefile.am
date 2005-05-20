installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstinterp_polynomial.la

kstinterp_polynomial_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstinterp_polynomial_la_LIBADD = $(GSL_LIBS)
kstinterp_polynomial_la_SOURCES = kstinterp_polynomial.cpp

METASOURCES=AUTO

install_DATA=kstinterp_polynomial.xml
