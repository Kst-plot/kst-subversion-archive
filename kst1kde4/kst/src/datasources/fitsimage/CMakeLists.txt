include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KST_FITSIO_PATH} ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstdata_fitsimage_PART_SRCS fitsimage.cpp)

kde4_add_plugin(kstdata_fitsimage ${kstdata_fitsimage_PART_SRCS})

target_link_libraries(kstdata_fitsimage ${FITSIO_LIBS} kstextdate kstbase ${KDE4_KPARTS_LIBS} ${KDE4_KDECORE_LIBS} ${QT_LIBRARIES})

install(TARGETS kstdata_fitsimage 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_fitsimage.desktop DESTINATION share/services/kst)
