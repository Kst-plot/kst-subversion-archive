INCLUDES=-I$(srcdir)/../.. $(all_includes)

kde_module_LTLIBRARIES=kstdata_ascii.la

kstdata_ascii_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstdata_ascii_la_SOURCES=ascii.cpp

METASOURCES=AUTO

services_DATA=kstdata_ascii.desktop
servicesdir=$(kde_servicesdir)/kst

