include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_discretizing_filters_PART_SRCS genericfilter.cpp)

kde4_add_plugin(kstobject_discretizing_filters ${kstobject_discretizing_filters_PART_SRCS})

target_link_libraries(kstobject_discretizing_filters kstbase kstmath ${KDE4_KPARTS_LIBS} ${KDE4_KDECORE_LIBS})

install(TARGETS kstobject_discretizing_filters 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_discretizing_filters.desktop DESTINATION share/services/kst)
