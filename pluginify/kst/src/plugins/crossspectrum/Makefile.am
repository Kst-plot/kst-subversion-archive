INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/widgets -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstapp  -I$(top_srcdir)/kst/src/libkstmath $(all_includes)

kde_module_LTLIBRARIES=kst_crossspectrum.la

kst_crossspectrum_la_LDFLAGS=$(all_libraries) -module -avoid-version
kst_crossspectrum_la_SOURCES=crosspowerspectrum.cpp crossspectrumdialog_i.cpp crossspectrumdialogwidget.ui crossspectrum.cpp fftsg_h.c

services_DATA=kst_crossspectrum.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
