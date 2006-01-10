INCLUDES=-I$(srcdir)/../.. -I$(srcdir)/../../extdate $(all_includes)

kde_module_LTLIBRARIES=kstdata_qimagesource.la

kstdata_qimagesource_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstdata_qimagesource_la_SOURCES=qimagesource.cpp

METASOURCES=AUTO

services_DATA=kstdata_qimagesource.desktop
servicesdir=$(kde_servicesdir)/kst

