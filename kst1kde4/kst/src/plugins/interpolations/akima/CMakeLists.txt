include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_akima_PART_SRCS akima.cpp)

kde4_add_plugin(kstobject_akima ${kstobject_akima_PART_SRCS})

target_link_libraries(kstobject_akima kstbase kstmath ${KDE4_KPARTS_LIBS} ${GSL_LIBS})

install(TARGETS kstobject_akima 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_akima.desktop DESTINATION share/services/kst)
