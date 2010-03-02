include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstdata_frame_PART_SRCS frame.cpp readdata.c creaddata.c)

kde4_add_plugin(kstdata_frame ${kstdata_frame_PART_SRCS})

target_link_libraries(kstdata_frame kstbase ${KDE4_KPARTS_LIBS} )

install(TARGETS kstdata_frame 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_frame.desktop DESTINATION /share/services/kst)
