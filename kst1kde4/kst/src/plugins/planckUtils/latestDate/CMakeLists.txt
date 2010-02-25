include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_planckLatestDate_PART_SRCS planckLatestDate.cpp)

kde4_add_plugin(kstobject_planckLatestDate ${kstobject_planckLatestDate_PART_SRCS})

target_link_libraries(kstobject_planckLatestDate kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_planckLatestDate 
	LIBRARY DESTINATION lib/kde3
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_planckLatestDate.desktop DESTINATION share/services/kst)

