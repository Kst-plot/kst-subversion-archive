include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_deconvolve_PART_SRCS deconvolve.cpp)

kde4_add_plugin(kstobject_deconvolve ${kstobject_deconvolve_PART_SRCS})

target_link_libraries(kstobject_deconvolve kstbase kstmath ${KDE4_KPARTS_LIBS} ${GSL_LIBS})

install(TARGETS kstobject_deconvolve 	
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_deconvolve.desktop DESTINATION share/services/kst)

