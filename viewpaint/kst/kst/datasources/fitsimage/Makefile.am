INCLUDES=-I$(srcdir)/../.. -I$(srcdir)/../../extdate $(all_includes)

kde_module_LTLIBRARIES=kstdata_fitsimage.la

kstdata_fitsimage_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstdata_fitsimage_la_LIBADD = $(FITSIO_LIBS)
kstdata_fitsimage_la_SOURCES=fitsimage.cpp

METASOURCES=AUTO

services_DATA=kstdata_fitsimage.desktop
servicesdir=$(kde_servicesdir)/kst

