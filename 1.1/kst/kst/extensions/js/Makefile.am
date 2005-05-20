if include_kjse_internal
    KJSEMBED_SUBDIR=kjsembed
endif

SUBDIRS=$(KJSEMBED_SUBDIR) .

if include_kjse_internal
INCLUDES=-I$(srcdir)/../.. -I$(srcdir)/kjsembed $(all_includes)
else
INCLUDES=-I$(srcdir)/../.. $(all_includes)
endif

kde_module_LTLIBRARIES=kstextension_js.la
if include_kstcmd
bin_PROGRAMS=kstcmd

kstcmd_SOURCES=kstcmd.cpp
kstcmd_LDADD=$(LIB_KDECORE) $(LIBREADLINE)
kstcmd_LDFLAGS=$(all_libraries)
endif

kstextension_js_la_LDFLAGS=$(all_libraries) -module -avoid-version
if include_kjse_internal
kstextension_js_la_LIBADD=../../libkstapp.la kjsembed/libkjsembed.la
else
kstextension_js_la_LIBADD=../../libkstapp.la -lkjsembed
endif
kstextension_js_la_SOURCES= \
			    js.cpp \
			    jsiface.skel \
			    jsiface_impl.cpp \
			    kstbinding.cpp \
			    bind_vector.cpp \
			    bind_kst.cpp \
			    bind_scalar.cpp \
			    bind_string.cpp \
			    bind_datasource.cpp \
			    bind_datavector.cpp \
			    bind_equation.cpp \
			    bind_powerspectrum.cpp \
			    bind_curve.cpp \
			    bind_point.cpp \
			    bind_window.cpp \
			    bind_plot.cpp \
			    bind_collection.cpp \
			    bind_plotcollection.cpp \
			    bind_curvecollection.cpp \
			    bind_windowcollection.cpp \
			    bind_vectorcollection.cpp \
			    bind_scalarcollection.cpp \
			    bind_stringcollection.cpp \
			    bind_datasourcecollection.cpp \
			    bind_histogram.cpp \
			    bind_debug.cpp \
			    bind_debuglog.cpp \
			    bind_debuglogentry.cpp \
			    bind_label.cpp \
			    bind_plotlabelcollection.cpp \
			    bind_histogramcollection.cpp \
			    bind_equationcollection.cpp \
			    bind_powerspectrumcollection.cpp \
			    bind_dataobject.cpp \
			    bind_dataobjectcollection.cpp \
			    bind_colorsequence.cpp \
			    bind_extension.cpp \
			    bind_extensioncollection.cpp \
			    bind_document.cpp \
			    bind_legend.cpp \
			    bind_plugin.cpp \
			    bind_plugincollection.cpp \
			    bind_pluginmanager.cpp \
			    bind_pluginmodule.cpp \
			    bind_pluginmodulecollection.cpp \
			    bind_pluginio.cpp \
			    bind_pluginiocollection.cpp

#			    js_dataobject.cpp \
#			    bind_jsdataobject.cpp

METASOURCES=AUTO

services_DATA=kstextension_js.desktop
servicesdir=$(kde_servicesdir)/kst

rcdir=$(kde_datadir)/kst
rc_DATA=kstextension_js.rc
