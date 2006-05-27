if compile_showimgdb
kexidb_dir=kexi
showimgdb_dir=showimgdb
endif

SUBDIRS =  $(kexidb_dir) $(showimgdb_dir) showimg showimgapp showimgpart

EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO

AUTOMAKE_OPTIONS = foreign 

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

COMPILE_FIRST = $(kexidb_dir) $(showimgdb_dir) showimg showimgapp showimgpart
COMPILE_BEFORE_showimgapp = showimg

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui"` > rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/showimg.pot; \
	fi
	rm -f tips.cpp
	rm -f rc.cpp
