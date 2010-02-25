include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstobject_spectral_normalization_PART_SRCS spectral_normalization.cpp)

kde4_add_plugin(kstobject_spectral_normalization ${kstobject_spectral_normalization_PART_SRCS})

target_link_libraries(kstobject_spectral_normalization kstbase kstmath ${GSL_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kstobject_spectral_normalization
 	LIBRARY DESTINATION lib/kde3
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_spectral_normalization.desktop DESTINATION share/services/kst)
