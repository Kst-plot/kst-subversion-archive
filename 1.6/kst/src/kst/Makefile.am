
#AM_CXXFLAGS = -Wold-style-cast -Wredundant-decls -Wunused-macros -Wfloat-equal -Wno-multichar

SUBDIRS=. pics

bin_PROGRAMS = kst

kst_SOURCES = main.cpp
kst_LDADD = ../libkstapp/libkstapp.la
kst_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)

KDE_ICON = kst

services_DATA = kstplugin.desktop kstdatasourceplugin.desktop kstdataobjectplugin.desktop kstbasicplugin.desktop kstfilter.desktop kstextension.desktop
servicesdir = $(kde_servicetypesdir)/kst

appsright_DATA = kst.desktop
appsrightdir = $(xdg_appsdir)

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Graphics

mimedir = $(kde_mimedir)/application
mime_DATA = x-kst.desktop

# set the include path for X, qt and KDE
INCLUDES=-I$(top_srcdir)/kst/src/extdate -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath -I../libkst -I$(top_srcdir)/kst/src/widgets -I../widgets -I$(top_srcdir)/kst/src/libkstapp -I../libkstapp -I$(top_srcdir) $(KMDI_INCLUDES) $(all_includes)

rcdir = $(kde_datadir)/kst
rc_DATA = kstui.rc

METASOURCES=AUTO

update_DATA = kstautosave11.upd kstrcmisc11.upd
updatedir = $(kde_datadir)/kconf_update

