include ../../../Makefile.global

kde_module_LTLIBRARIES = showimg_kexidb_mysqldriver.la

INCLUDES = -I$(showimg_top_srcdir)/kexi/ -I$(showimg_top_srcdir)/kexi/kexidb/ -I$(MYSQL_INC) \
	 $(all_includes)

showimg_kexidb_mysqldriver_la_METASOURCES = AUTO

showimg_kexidb_mysqldriver_la_SOURCES = \
  mysqldriver.cpp \
  mysqlconnection.cpp \
  mysqlconnection_p.cpp \
  mysqlcursor.cpp \
  mysqlkeywords.cpp

showimg_kexidb_mysqldriver_la_LIBADD = $(LIB_KPARTS) \
  $(LIB_QT) \
  $(MYSQL_LIBS) \
  -lmysqlclient \
  ../../libshowimgkexidb.la

showimg_kexidb_mysqldriver_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(VER_INFO) -no-undefined


kde_services_DATA = showimg_kexidb_mysqldriver.desktop


KDE_CXXFLAGS += -DKEXIDB_MYSQL_DRIVER_EXPORT= -D__KEXIDB__= \
	-include $(showimg_top_srcdir)/kexi/kexidb/global.h

