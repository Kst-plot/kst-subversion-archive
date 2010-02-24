INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kstobject_crosscorrelate_bruteforce.la

kstobject_crosscorrelate_bruteforce_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_crosscorrelate_bruteforce_la_SOURCES=crosscorrelate.cpp

services_DATA=kstobject_crosscorrelate_bruteforce.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
