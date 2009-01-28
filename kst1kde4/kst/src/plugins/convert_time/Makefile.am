INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_converttime.la

kstobject_converttime_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_converttime_la_SOURCES=converttime.cpp

services_DATA=kstobject_converttime.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
