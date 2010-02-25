include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_statistics_PART_SRCS statistics.cpp)

kde4_add_plugin(kstobject_statistics ${kstobject_statistics_PART_SRCS})

target_link_libraries(kstobject_statistics kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_statistics 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_statistics.desktop DESTINATION share/services/kst)
