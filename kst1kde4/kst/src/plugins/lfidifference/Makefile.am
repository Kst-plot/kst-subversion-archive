INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_lfidifference.la

kstobject_lfidifference_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_lfidifference_la_SOURCES=lfidifference.cpp

services_DATA=kstobject_lfidifference.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
