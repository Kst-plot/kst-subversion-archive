INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/widgets -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstapp  -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kst_linefit.la

kst_linefit_la_LDFLAGS=$(all_libraries) -module -avoid-version
kst_linefit_la_SOURCES=linefit.cpp

services_DATA=kst_linefit.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
