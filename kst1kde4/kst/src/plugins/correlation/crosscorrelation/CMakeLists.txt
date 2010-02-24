include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_crosscorrelate_PART_SRCS crosscorrelate.cpp)

kde4_add_plugin(kstobject_crosscorrelate ${kstobject_crosscorrelate_PART_SRCS})

target_link_libraries(kstobject_crosscorrelate kstbase kstmath ${KDE4_KPARTS_LIBS} ${GSL_LIBS})

install(TARGETS kstobject_crosscorrelate  
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_crosscorrelate.desktop DESTINATION share/services/kst)
