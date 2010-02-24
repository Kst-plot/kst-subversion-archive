include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_akima_periodic_PART_SRCS akima_periodic.cpp)

kde4_add_plugin(kstobject_akima_periodic ${kstobject_akima_periodic_PART_SRCS})

target_link_libraries(kstobject_akima_periodic ${GSL_LIBS} kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_akima_periodic 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_akima_periodic.desktop DESTINATION share/services/kst)
