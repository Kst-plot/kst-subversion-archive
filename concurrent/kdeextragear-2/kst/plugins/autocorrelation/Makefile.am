installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../kst $(all_includes)

install_LTLIBRARIES = autocorrelation.la

autocorrelation_la_LDFLAGS = -module $(KDE_PLUGIN)
autocorrelation_la_SOURCES = autocorrelation.cpp

METASOURCES=AUTO

install_DATA=autocorrelation.xml
