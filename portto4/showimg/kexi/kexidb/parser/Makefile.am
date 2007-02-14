include ../../Makefile.global

lib_LTLIBRARIES = libshowimgkexidbparser.la
libshowimgkexidbparser_la_SOURCES = sqlscanner.cpp sqlparser.cpp parser.cpp parser_p.cpp
libshowimgkexidbparser_la_LIBADD = $(LIB_KPARTS) $(LIB_KDEUI) ../libshowimgkexidb.la
libshowimgkexidbparser_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(VER_INFO)

noinst_HEADERS = parser_p.h

INCLUDES = -I$(showimg_top_srcdir)/kexi -I$(showimg_top_srcdir)/kexi/kexidb $(all_includes)
METASOURCES = AUTO


parser:
	cd $(srcdir); \
	lex -osqlscanner.cpp sqlscanner.l; \
	bison -dv sqlparser.y; \
	echo '#include "field.h"' > sqlparser.h; \
	echo '#include "parser.h"' >> sqlparser.h; \
	echo '#include "sqltypes.h"' >> sqlparser.h; \
	echo '' >> sqlparser.h; \
	echo 'bool parseData(KexiDB::Parser *p, const char *data);' >> sqlparser.h; \
	cat sqlparser.tab.h >> sqlparser.h; \
	cat sqlparser.tab.c > sqlparser.cpp; \
	echo "const char * const tname(int offset) { return yytname[offset]; }" >> sqlparser.cpp; \
	./extract_tokens.sh > tokens.cpp; \
	rm -f sqlparser.tab.h sqlparser.tab.c

coffie:
	echo 'making coffie...'
	sleep 5

KDE_OPTIONS=nofinal
.PHONY: parser coffie

KDE_CXXFLAGS += -DYYERROR_VERBOSE=1 -DMAKE_KEXI_DB_LIB

