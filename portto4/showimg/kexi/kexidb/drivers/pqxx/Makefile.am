include ../../../Makefile.global

kde_module_LTLIBRARIES = showimg_kexidb_pqxxsqldriver.la

INCLUDES = -I../../.. $(all_includes) -I$(PG_INCDIR) -I$(PQXX_INCDIR)

showimg_kexidb_pqxxsqldriver_la_METASOURCES = AUTO

showimg_kexidb_pqxxsqldriver_la_SOURCES = pqxxdriver.cpp pqxxcursor.cpp pqxxconnection.cpp pqxxkeywords.cpp

showimg_kexidb_pqxxsqldriver_la_LIBADD = $(LIB_KPARTS) $(LIB_QT) -lpqxx ../../libshowimgkexidb.la

showimg_kexidb_pqxxsqldriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) -L$(PQXX_LIBDIR) -L$(PG_LIBDIR) $(VER_INFO) -no-undefined

kde_services_DATA = showimg_kexidb_pqxxsqldriver.desktop

noinst_HEADERS =  pqxxconnection.h

KDE_CXXFLAGS += -DKEXIDB_PGSQL_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(showimg_top_srcdir)/kexi/kexidb/global.h
