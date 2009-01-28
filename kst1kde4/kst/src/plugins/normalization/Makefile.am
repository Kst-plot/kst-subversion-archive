INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_normalization.la

kstobject_normalization_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_normalization_la_SOURCES=normalization.cpp
kstobject_normalization_la_LIBADD = $(GSL_LIBS)

services_DATA=kstobject_normalization.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
