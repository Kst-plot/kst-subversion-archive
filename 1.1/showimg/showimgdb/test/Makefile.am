SUBDIRS = .

bin_PROGRAMS = sidbtest

noinst_PROGRAMS = sidbtest
 
INCLUDES = -I.. -I$(top_srcdir)/kexi -I$(top_srcdir)/kexi/kexidb $(all_includes)

METASOURCES = AUTO

sidbtest_SOURCES = \
	main.cpp 


sidbtest_LDADD =\
	../libshowimgdb.la \
	$(LIB_QT) $(LIB_KDECORE) \
	../../kexi/kexidb/libkexidb.la \
	../../kexi/kexidb/parser/libkexidbparser.la

sidbtest_LDFLAGS = $(all_libraries) $(KDE_RPATH)


