INCLUDES=-I$(srcdir)/../.. $(all_includes)

kde_module_LTLIBRARIES=kstextension_elog.la

kstextension_elog_la_LDFLAGS=$(all_libraries) -module -avoid-version
kstextension_elog_la_LIBADD=../../libkst.la $(LIB_KPARTS)
kstextension_elog_la_SOURCES=elog.cpp elogthread.cpp elogthreadsubmit.cpp elogthreadattrs.cpp elogentry_i.cpp elogconfiguration_i.cpp elogentry.ui elogconfiguration.ui

METASOURCES=AUTO

services_DATA=kstextension_elog.desktop
servicesdir=$(kde_servicesdir)/kst

#rcdir=$(kde_datadir)/kst
#rc_DATA=kstextension_elog.rc
