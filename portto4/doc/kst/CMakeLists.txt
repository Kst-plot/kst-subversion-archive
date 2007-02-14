
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_create_handbook(index.docbook)
kde4_install_handbook()



#original Makefile.am contents follow:

#EXTRA_DIST = index.docbook 
#
#KDE_LANG = en
#KDE_DOCS = kst
#man_MANS = kst.1
