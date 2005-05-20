INCLUDES=-I$(srcdir)/../.. $(all_includes)

kde_module_LTLIBRARIES=kstdata_template.la

kstdata_template_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstdata_template_la_SOURCES=template.cpp

METASOURCES=AUTO

services_DATA=kstdata_template.desktop
servicesdir=$(kde_servicesdir)/kst

