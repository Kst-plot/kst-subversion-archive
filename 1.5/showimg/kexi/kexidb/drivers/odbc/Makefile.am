include ../../../Makefile.global

kde_module_LTLIBRARIES = showimg_kexidb_odbcdriver.la

INCLUDES = -I$(showimg_top_srcdir)/kexi $(all_includes)

showimg_kexidb_odbcdriver_la_METASOURCES = AUTO

showimg_kexidb_odbcdriver_la_SOURCES = odbcdriver.cpp odbcconnection.cpp 

showimg_kexidb_odbcdriver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) -lodbc $(top_builddir)/kexi/kexidb/libshowimgkexidb.la

showimg_kexidb_odbcdriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined


kde_services_DATA = showimg_kexidb_odbcdriver.desktop


KDE_CXXFLAGS += -DKEXIDB_ODBC_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(showimg_top_srcdir)/kexi/kexidb/global.h

