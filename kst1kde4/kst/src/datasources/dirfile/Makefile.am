INCLUDES=-I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/extdate $(all_includes) $(GETDATA_CFLAGS)

if NO_EXTERNAL_GETDATA
GETDATA_C=getdata.c
endif

kde_module_LTLIBRARIES=kstdata_dirfile.la

kstdata_dirfile_la_LDFLAGS=$(all_libraries) $(GETDATA_LIBS) -module -avoid-version
kstdata_dirfile_la_SOURCES=dirfile.cpp $(GETDATA_C)

METASOURCES=AUTO

services_DATA=kstdata_dirfile.desktop
servicesdir=$(kde_servicesdir)/kst

