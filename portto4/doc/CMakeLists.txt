
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(AUTODIRS)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_create_handbook(index.docbook)
kde4_install_handbook()



#original Makefile.am contents follow:

#
#KDE_LANG = en
#KDE_DOCS = AUTO
#SUBDIRS = $(AUTODIRS)
#
