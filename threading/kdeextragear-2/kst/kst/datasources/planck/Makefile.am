INCLUDES=-I$(srcdir)/../.. $(all_includes)

kde_module_LTLIBRARIES=kstdata_planck.la

kstdata_planck_la_LIBADD=$(PIOLIBS)
kstdata_planck_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstdata_planck_la_SOURCES=planck.cpp planckdata.cpp plancktoi.cpp pparser.cpp

METASOURCES=AUTO

services_DATA=kstdata_planck.desktop
servicesdir=$(kde_servicesdir)/kst

