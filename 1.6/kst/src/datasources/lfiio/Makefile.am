INCLUDES=-I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/extdate $(all_includes)

kde_module_LTLIBRARIES = kstdata_lfiio.la

kstdata_lfiio_la_LDFLAGS = $(all_libraries) -module -avoid-version 
kstdata_lfiio_la_LIBADD = $(FITSIO_LIBS)
kstdata_lfiio_la_SOURCES = lfiio.cpp

METASOURCES=AUTO

services_DATA=kstdata_lfiio.desktop
servicesdir=$(kde_servicesdir)/kst

