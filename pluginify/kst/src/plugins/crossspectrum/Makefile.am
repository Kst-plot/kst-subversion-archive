installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

install_LTLIBRARIES = crossspectrum.la

crossspectrum_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
crossspectrum_la_SOURCES = crossspectrum.cpp fftsg_h.c

METASOURCES=AUTO

install_DATA=crossspectrum.xml
