
#AM_CXXFLAGS = -Wold-style-cast -Wredundant-decls -Wunused-macros -Wfloat-equal -Wno-multichar

SUBDIRS=. pics

bin_PROGRAMS = kst

kst_SOURCES = main.cpp
kst_LDADD = ../libkstapp/libkstapp.la
kst_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined

KDE_ICON = kst

services_DATA = kstplugin.desktop kstdatasourceplugin.desktop kstdataobjectplugin.desktop kstbasicplugin.desktop kstfilter.desktop kstextension.desktop
servicesdir = $(kde_servicetypesdir)/kst

apps_DATA = kst.desktop
appsdir = $(kde_appsdir)/Applications/Sciences

mimedir = $(kde_mimedir)/application
mime_DATA = x-kst.desktop

# set the include path for X, qt and KDE
INCLUDES=-I$(top_srcdir)/kst/src/extdate -I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/libkstmath -I../libkst -I$(top_srcdir)/kst/src/widgets -I../widgets -I$(top_srcdir)/kst/src/libkstapp -I../libkstapp -I$(top_srcdir) $(KMDI_INCLUDES) $(all_includes)

rcdir = $(kde_datadir)/kst
rc_DATA = kstui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kst.pot; \
	fi

METASOURCES=AUTO

update_DATA = kstautosave11.upd kstrcmisc11.upd
updatedir = $(kde_datadir)/kconf_update

