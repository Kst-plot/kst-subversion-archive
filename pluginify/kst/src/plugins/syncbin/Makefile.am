installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../kst $(all_includes)
 
install_LTLIBRARIES = syncbin.la
 
syncbin_la_LDFLAGS = -module $(KDE_PLUGIN)
syncbin_la_SOURCES = syncbin.cpp
 
METASOURCES=AUTO
 
install_DATA=syncbin.xml
