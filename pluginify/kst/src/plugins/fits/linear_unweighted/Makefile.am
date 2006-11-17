INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_linear_unweighted.la

kstobject_linear_unweighted_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_linear_unweighted_la_SOURCES=linear_unweighted.cpp

services_DATA=kstobject_linear_unweighted.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
