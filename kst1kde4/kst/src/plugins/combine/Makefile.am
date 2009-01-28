INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_combine.la

kstobject_combine_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_combine_la_SOURCES=combine.cpp

services_DATA=kstobject_combine.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
