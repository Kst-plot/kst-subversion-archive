bin_PROGRAMS = sidbtest

noinst_PROGRAMS = sidbtest
 
INCLUDES = -I$(showimg_top_srcdir)/showimgdb -I$(showimg_top_srcdir)/kexi -I$(showimg_top_srcdir)/kexi/kexidb $(all_includes)

METASOURCES = AUTO

sidbtest_SOURCES = \
	main.cpp 


sidbtest_LDADD =\
	../libshowimgdb.la \
	$(LIB_QT) $(LIB_KDECORE) \
	$(LIBKEXIDB_LDDADD)
	
sidbtest_LDFLAGS = $(all_libraries) $(KDE_RPATH)


