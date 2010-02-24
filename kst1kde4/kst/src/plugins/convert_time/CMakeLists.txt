include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})
########### next target ###############

set(kstobject_converttime_PART_SRCS converttime.cpp)

kde4_add_plugin(kstobject_converttime ${kstobject_converttime_PART_SRCS})

target_link_libraries(kstobject_converttime kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_converttime 	
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_converttime.desktop DESTINATION share/services/kst)
