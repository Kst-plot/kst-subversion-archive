installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../kst $(all_includes)
 
install_LTLIBRARIES = bin.la
 
bin_la_LDFLAGS = -module $(KDE_PLUGIN)
bin_la_SOURCES = bin.cpp
 
METASOURCES=AUTO
 
install_DATA=bin.xml
