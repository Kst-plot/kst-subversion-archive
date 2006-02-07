SUBDIRS = kst plugins misc

EXTRA_DIST = AUTHORS COPYING ChangeLog INSTALL README TODO kst.lsm 

changes:
	cvs2cl.pl	--ignore 'ChangeLog' \
			--ignore 'kst/kst.desktop' \
			--prune --day-of-week  --hide-filenames --no-wrap .

cvschanges: changes
	cvs commit -m 'CVS_SILENT update changelog' ChangeLog

