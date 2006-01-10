installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=cumulative_sum.la

cumulative_sum_la_LDFLAGS=-module $(KDE_PLUGIN)
cumulative_sum_la_SOURCES=cumulative_sum.c

METASOURCES=AUTO

install_DATA=cumulative_sum.xml
