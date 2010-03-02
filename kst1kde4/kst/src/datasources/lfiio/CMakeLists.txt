include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KST_FITSIO_PATH} ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstdata_lfiio_PART_SRCS lfiio.cpp)

kde4_add_plugin(kstdata_lfiio ${kstdata_lfiio_PART_SRCS})

target_link_libraries(kstdata_lfiio ${FITSIO_LIBS} kstbase ${KDE4_KPARTS_LIBS})

install(TARGETS kstdata_lfiio 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_lfiio.desktop DESTINATION share/services/kst)
