include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${CMAKE_SOURCE_DIR}/src/widgets ${CMAKE_SOURCE_DIR}/src/libkstapp ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

add_subdirectory(pics)

########### next target ###############

set(kst_SRCS main.cpp)

add_executable(kst ${kst_SRCS})

target_link_libraries(kst kstapp kmdi)

install(TARGETS kst 
	RUNTIME DESTINATION bin
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kst.desktop DESTINATION share/applnk/Graphics)
install(FILES  kstui.rc DESTINATION share/apps/kst)
install(FILES  kstplugin.desktop kstdatasourceplugin.desktop kstdataobjectplugin.desktop kstbasicplugin.desktop kstfilter.desktop kstextension.desktop DESTINATION share/servicetypes/kst)
install(FILES  kstautosave11.upd kstrcmisc11.upd DESTINATION share/apps/kconf_update)
install(FILES  x-kst.desktop DESTINATION share/mimelink/application)
install(FILES  kst.desktop DESTINATION share/applications)

# xxx KDE3_INSTALL_ICONS(hicolor)
# xxx KDE3_INSTALL_ICONS(locolor)

