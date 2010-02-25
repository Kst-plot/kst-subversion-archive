include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_syncbin_PART_SRCS syncbin.cpp)

kde4_add_plugin(kstobject_syncbin ${kstobject_syncbin_PART_SRCS})

target_link_libraries(kstobject_syncbin kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_syncbin 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_syncbin.desktop DESTINATION share/services/kst)
