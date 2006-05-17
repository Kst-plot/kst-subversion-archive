include ../../../Makefile.global

kde_module_LTLIBRARIES = showimg_kexidb_sqlite3driver.la

INCLUDES = -I$(showimg_top_srcdir)/kexi -I$(showimg_top_srcdir)/kexi/kexidb \
	-I$(showimg_top_srcdir)/kexi/3rdparty/kexisql3/src/ \
	$(all_includes)





showimg_kexidb_sqlite3driver_la_METASOURCES = AUTO

showimg_kexidb_sqlite3driver_la_SOURCES = sqliteconnection.cpp sqlitedriver.cpp sqlitecursor.cpp sqlitekeywords.cpp

showimg_kexidb_sqlite3driver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) \
 $(showimg_top_builddir)/kexi/3rdparty/kexisql3/src/libshowimgkexisql3.la \
 $(showimg_top_builddir)/kexi/kexidb/libshowimgkexidb.la \
 $(showimg_top_builddir)/kexi/kexidb/parser/libshowimgkexidbparser.la

showimg_kexidb_sqlite3driver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) 


kde_services_DATA = showimg_kexidb_sqlite3driver.desktop


KDE_CXXFLAGS += -DKEXIDB_SQLITE_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(showimg_top_srcdir)/kexi/kexidb/global.h

