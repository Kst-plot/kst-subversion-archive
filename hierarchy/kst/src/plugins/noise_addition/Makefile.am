installdir=$(kde_moduledir)/kstplugins
INCLUDES=-I$(srcdir)/../../../kst $(all_includes)

install_LTLIBRARIES=noise_addition.la

noise_addition_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
noise_addition_la_LIBADD = $(GSL_LIBS)
noise_addition_la_SOURCES = noise_addition.cpp

METASOURCES=AUTO

install_DATA=noise_addition.xml
