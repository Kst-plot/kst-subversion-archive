installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES = kstfit_general_levenberg_marquardt.la

kstfit_general_levenberg_marquardt_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kstfit_general_levenberg_marquardt_la_LIBADD = $(GSL_LIBS) $(MUPARSER_LIBS)
kstfit_general_levenberg_marquardt_la_SOURCES = kstfit_general_levenberg_marquardt.cpp

METASOURCES=AUTO

install_DATA=kstfit_general_levenberg_marquardt.xml
