include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_cumulative_sum_PART_SRCS cumulative_sum.cpp)

kde4_add_plugin(kstobject_cumulative_sum ${kstobject_cumulative_sum_PART_SRCS})

target_link_libraries(kstobject_cumulative_sum kstbase kstmath ${KDE4_KPARTS_LIBS} ${KDE4_KDECORE_LIBS})

install(TARGETS kstobject_cumulative_sum  
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_cumulative_sum.desktop DESTINATION share/services/kst)
