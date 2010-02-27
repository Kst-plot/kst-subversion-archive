include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstdata_qimagesource_PART_SRCS qimagesource.cpp)

kde4_add_plugin(kstdata_qimagesource ${kstdata_qimagesource_PART_SRCS})

target_link_libraries(kstdata_qimagesource kstbase kstextdate ${KDE4_KPARTS_LIBS} ${QT_LIBRARIES})

install(TARGETS kstdata_qimagesource 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_qimagesource.desktop DESTINATION share/services/kst)
