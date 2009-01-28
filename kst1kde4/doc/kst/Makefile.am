EXTRA_DIST = index.docbook 

KDE_LANG = en
KDE_DOCS = kst
man_MANS = kst.1

manpage:
	meinproc index.docbook --stylesheet $(KDEDIR)/share/apps/ksgmltools2/docbook/xsl/manpages/docbook.xsl

htmlfiles:
	meinproc index.docbook --stylesheet $(KDEDIR)/share/apps/ksgmltools2/customization/kde-chunk.xsl

pdffile:
	dblatex -b pdftex -o KstHandbook.pdf -P latex.output.revhistory=0  -P newtbl.use=1 -P imagedata.default.scale=pagebound -P literal.width.ignore=1 -I $(KDEDIR)/share/doc/HTML/en/ -X index.docbook
