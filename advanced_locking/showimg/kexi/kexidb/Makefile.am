include ../Makefile.global

lib_LTLIBRARIES = libshowimgkexidb.la

INCLUDES = -I$(showimg_top_srcdir)/kexi $(all_includes)

SUBDIRS =  . parser drivers

libshowimgkexidb_la_METASOURCES = AUTO

libshowimgkexidb_la_SOURCES = drivermanager.cpp driver.cpp driver_p.cpp connection.cpp \
 keywords.cpp object.cpp field.cpp utils.cpp expression.cpp \
 connectiondata.cpp fieldlist.cpp tableschema.cpp cursor.cpp transaction.cpp  \
 indexschema.cpp queryschema.cpp schemadata.cpp global.cpp relationship.cpp \
 roweditbuffer.cpp msghandler.cpp\
 dbobjectnamevalidator.cpp

noinst_HEADERS = drivermanager_p.h utils_p.h

kexidbincludedir=$(includedir)/kexidb
#kexidbinclude_HEADERS=connection.h connectiondata.h cursor.h \
#  driver.h drivermanager.h \
#  error.h expression.h field.h fieldlist.h global.h object.h \
#  schemadata.h tableschema.h queryschema.h indexschema.h \
#  relationship.h parser/parser.h transaction.h \
#  kexidb_export.h

libshowimgkexidb_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KIO) ../kexiutils/libshowimgkexiutils.la
libshowimgkexidb_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kde_servicetypes_DATA = showimg_kexidb_driver.desktop


KDE_CXXFLAGS += -D__KEXIDB__ -DMAKE_KEXI_DB_LIB -include $(showimg_top_srcdir)/kexi/kexidb/global.h

