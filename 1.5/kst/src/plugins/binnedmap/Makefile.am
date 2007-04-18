INCLUDES=-I$(top_srcdir)/kst -I$(top_srcdir)/kst/src/widgets -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstapp -I$(top_srcdir)/kst/src/libkstmath -I$(top_srcdir)/kst/src/extdate -I$(top_builddir)/kst -I$(top_builddir)/kst/src/widgets -I$(top_builddir)/kst/src/libkst -I$(top_builddir)/kst/src/libkstapp -I$(top_builddir)/kst/src/libkstmath -I$(top_builddir)/kst/src/extdate $(all_includes)

kde_module_LTLIBRARIES=kstobject_binnedmap.la

kstobject_binnedmap_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstobject_binnedmap_la_SOURCES=binnedmap.cpp binnedmapdialog_i.cpp binnedmapdialogwidget.ui

services_DATA=kstobject_binnedmap.desktop
servicesdir=$(kde_servicesdir)/kst

METASOURCES=AUTO
