INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_trim.la

kstobject_trim_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_trim_la_SOURCES=trim.cpp

services_DATA=kstobject_trim.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
