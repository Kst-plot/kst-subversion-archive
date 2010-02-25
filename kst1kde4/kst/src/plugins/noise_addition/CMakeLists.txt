include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_noise_addition_PART_SRCS noise_addition.cpp)

kde4_add_plugin(kstobject_noise_addition ${kstobject_noise_addition_PART_SRCS})

target_link_libraries(kstobject_noise_addition ${GSL_LIBS} kstbase kstmath ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_noise_addition 
	LIBRARY DESTINATION lib/kde4
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_noise_addition.desktop DESTINATION share/services/kst)
