include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR} ${KST_FITSIO_PATH})

########### next target ###############

set(kstdata_wmap_PART_SRCS wmap.cpp)

kde4_add_plugin(kstdata_wmap ${kstdata_wmap_PART_SRCS})

target_link_libraries(kstdata_wmap ${FITSIO_LIBS} kstbase kstextdate ${KDE4_KPARTS_LIBS} ${QT_LIBRARIES})

install(TARGETS kstdata_wmap 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_wmap.desktop DESTINATION share/services/kst)
