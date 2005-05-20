SUBDIRS = . test
INCLUDES =  -I$(top_srcdir)/showimg -I$(top_srcdir)/kexi -I$(top_srcdir)/kexi/kexidb  $(all_includes)

noinst_LTLIBRARIES = libshowimgdb.la

libshowimgdb_la_METASOURCES = AUTO

libshowimgdb_la_SOURCES = \
	categorynode.cpp \
	categories.cpp\
	imageentry.cpp\
	categoriesdb.cpp

libshowimgdb_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) \
	../kexi/kexidb/libkexidb.la \
	../kexi/kexidb/parser/libkexidbparser.la

libshowimgdb_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)



#noinst_PROGRAMS = sidb_test
#
#noinst_PROGRAMS = sidb_test
# 
#INCLUDES = -I$(top_srcdir)/kexidb $(all_includes)
#
#SUBDIRS = .
#
#METASOURCES = AUTO
#
#sidb_test_SOURCES = \
#	test_ima.cpp \
#	categorynode.cpp \
#	categories.cpp\
#	imageentry.cpp\
#	categoriesdb.cpp
#
#
#sidb_test_LDADD = $(LIB_QT) $(LIB_KDECORE) \
#	$(top_builddir)/kexidb/libkexidb.la \
#	$(top_builddir)/kexidb/parser/libkexidbparser.la
#
#sidb_test_LDFLAGS = $(all_libraries) $(KDE_RPATH)


