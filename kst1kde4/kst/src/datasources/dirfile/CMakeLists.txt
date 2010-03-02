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

