include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_cspline_periodic_PART_SRCS cspline_periodic.cpp)

kde4_add_plugin(kstobject_cspline_periodic ${kstobject_cspline_periodic_PART_SRCS})

target_link_libraries(kstobject_cspline_periodic ${GSL_LIBS} kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_cspline_periodic 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_cspline_periodic.desktop DESTINATION share/services/kst)

