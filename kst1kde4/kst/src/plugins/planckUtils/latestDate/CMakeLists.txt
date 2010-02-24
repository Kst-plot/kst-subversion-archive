include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstobject_planckLatestDate_PART_SRCS planckLatestDate.cpp)

KDE3_AUTOMOC(${kstobject_planckLatestDate_PART_SRCS})

KDE3_ADD_KPART(kstobject_planckLatestDate ${kstobject_planckLatestDate_PART_SRCS})

target_link_libraries(kstobject_planckLatestDate)

install(TARGETS kstobject_planckLatestDate 
	LIBRARY DESTINATION lib/kde3
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_planckLatestDate.desktop DESTINATION share/services/kst)

