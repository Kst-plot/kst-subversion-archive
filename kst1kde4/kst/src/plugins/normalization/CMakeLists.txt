include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_normalization_PART_SRCS normalization.cpp)

kde4_add_plugin(kstobject_normalization ${kstobject_normalization_PART_SRCS})

target_link_libraries(kstobject_normalization ${KDE4_KDECORE_LIBS} kstbase kstmath ${KDE4_KPARTS_LIBS} ${GSL_LIBS})

install(TARGETS kstobject_normalization 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_normalization.desktop DESTINATION share/services/kst)

