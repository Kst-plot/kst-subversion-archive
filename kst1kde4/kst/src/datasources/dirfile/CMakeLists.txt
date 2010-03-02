include_directories(${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

IF (HAVE_EXTERNAL_GETDATA)
    message(STATUS "========== dirfile datasource will use external getdata library")
ELSE (HAVE_EXTERNAL_GETDATA)
    message(STATUS "========== dirfile datasource will use internal getdata implementation")
ENDIF (HAVE_EXTERNAL_GETDATA)

########### next target ###############

set(kstdata_dirfile_PART_SRCS dirfile.cpp ${GETDATA_C})

kde4_add_plugin(kstdata_dirfile ${kstdata_dirfile_PART_SRCS})

target_link_libraries(kstdata_dirfile kstbase kio kstmath ${KDE4_KPARTS_LIBS} ${KDE4_KDECORE_LIBS} ${GETDATA_LIBS} ${QT_LIBRARIES})

install(TARGETS kstdata_dirfile 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_dirfile.desktop DESTINATION share/services/kst)



#original Makefile.am contents follow:

#INCLUDES=-I$(top_srcdir)/kst/src/libkst -I$(top_srcdir)/kst/src/extdate $(all_includes) $(GETDATA_CFLAGS)
#
#if NO_EXTERNAL_GETDATA
#GETDATA_C=getdata.c
#endif
#
#kde_module_LTLIBRARIES=kstdata_dirfile.la
#
#kstdata_dirfile_la_LDFLAGS=$(all_libraries) $(GETDATA_LIBS) -module -avoid-version
#kstdata_dirfile_la_SOURCES=dirfile.cpp $(GETDATA_C)
#
#METASOURCES=AUTO
#
#services_DATA=kstdata_dirfile.desktop
#servicesdir=$(kde_servicesdir)/kst
#
