include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/extdate ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstdata_indirect_PART_SRCS indirect.cpp)

kde4_add_plugin(kstdata_indirect ${kstdata_indirect_PART_SRCS})

target_link_libraries(kstdata_indirect kstbase ${KDE4_KPARTS_LIBS})

install(TARGETS kstdata_indirect 
	LIBRARY DESTINATION lib/kde4
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstdata_indirect.desktop DESTINATION share/services/kst)
