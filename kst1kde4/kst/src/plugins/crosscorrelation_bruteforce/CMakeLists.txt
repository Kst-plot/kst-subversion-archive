include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstobject_crosscorrelate_bruteforce_PART_SRCS crosscorrelate.cpp)

KDE3_AUTOMOC(${kstobject_crosscorrelate_bruteforce_PART_SRCS})

KDE3_ADD_KPART(kstobject_crosscorrelate_bruteforce ${kstobject_crosscorrelate_bruteforce_PART_SRCS})

target_link_libraries(kstobject_crosscorrelate_bruteforce ${KDE4_KDECORE_LIBS})

install(TARGETS kstobject_crosscorrelate_bruteforce  
	LIBRARY DESTINATION lib/kde3
 	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstobject_crosscorrelate_bruteforce.desktop DESTINATION share/services/kst)
