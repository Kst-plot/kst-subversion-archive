include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_convolve_PART_SRCS convolve.cpp)

kde4_add_plugin(kstobject_convolve ${kstobject_convolve_PART_SRCS})

target_link_libraries(kstobject_convolve kstbase kstmath ${KDE4_KPARTS_LIBS} ${GSL_LIBS})

install(TARGETS kstobject_convolve 	
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_convolve.desktop DESTINATION share/services/kst)
