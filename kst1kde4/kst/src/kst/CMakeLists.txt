include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${CMAKE_SOURCE_DIR}/src/widgets ${CMAKE_SOURCE_DIR}/src/libkstapp ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### next target ###############

set(kst_SRCS 
		kaboutdata.cpp
#		kcmdlineargs.cpp
		main.cpp)
set(kst_RCCS kst.qrc)

qt4_add_resources(kst_RCC_SRCS ${kst_RCCS})

add_executable(kst ${kst_SRCS} ${kst_RCC_SRCS})

target_link_libraries(kst kstapp ${KDE4_KDECORE_LIBS} QtXml QtGui)

install(TARGETS kst 
	DESTINATION bin
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kst.desktop DESTINATION share/applnk/Graphics)
install(FILES  kstui.rc DESTINATION share/apps/kst)
install(FILES  kstplugin.desktop kstdatasourceplugin.desktop kstdataobjectplugin.desktop kstbasicplugin.desktop kstfilter.desktop kstextension.desktop DESTINATION share/servicetypes/kst)
install(FILES  x-kst.desktop DESTINATION share/mimelink/application)
install(FILES  kst.desktop DESTINATION share/applications)
