INCLUDES=-I$(srcdir)/../.. $(all_includes)

kde_module_LTLIBRARIES=kstextension_js.la

kstextension_js_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstextension_js_la_LIBADD=../../libkst.la $(LIB_KPARTS) -lkjsembed
kstextension_js_la_SOURCES=js.cpp jsproxies.cpp

METASOURCES=AUTO

services_DATA=kstextension_js.desktop
servicesdir=$(kde_servicesdir)/kst

rcdir=$(kde_datadir)/kst
rc_DATA=kstextension_js.rc
