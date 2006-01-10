INCLUDES=-I$(srcdir)/../.. -I$(srcdir)/../../extdate $(CDFINCLUDES) $(all_includes)

kde_module_LTLIBRARIES=kstdata_cdf.la

kstdata_cdf_la_LIBADD=$(CDFLIBS)
kstdata_cdf_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstdata_cdf_la_SOURCES=cdf.cpp

METASOURCES=AUTO

services_DATA=kstdata_cdf.desktop
servicesdir=$(kde_servicesdir)/kst

