installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstfit_gradient_unweighted.la

kstfit_gradient_unweighted_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstfit_gradient_unweighted_la_LIBADD = $(GSL_LIBS)
kstfit_gradient_unweighted_la_SOURCES = kstfit_gradient_unweighted.cpp

METASOURCES=AUTO

install_DATA=kstfit_gradient_unweighted.xml
