SUBDIRS = src misc tests

EXTRA_DIST = AUTHORS COPYING COPYING.LGPL ChangeLog INSTALL README TODO RELEASE.NOTES kst.lsm 

changes:
	(cd $(srcdir) && svn log --xml | xsltproc svn2cl.xsl - > ChangeLog)

svnchanges: changes
	(cd $(srcdir) && svn commit -m 'update changelog' ChangeLog)

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
	cat `find . -name "*.xml"` |perl -e 'while(<STDIN>) { if (/\<modulename .* readableName="(.*)"/) { print "i18n(\"$$1\");\n"; }}' >> rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kst.pot; \
	fi
