SUBDIRS = kst plugins misc tests

EXTRA_DIST = AUTHORS COPYING COPYING.LGPL ChangeLog INSTALL README TODO kst.lsm 

changes:
	(cd $(srcdir) && svn log --xml | xsltproc svn2cl.xsl - > ChangeLog)

svnchanges: changes
	(cd $(srcdir) && svn commit -m 'update changelog' ChangeLog)

