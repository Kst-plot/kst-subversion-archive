project(kgrab)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories (${KDE4_LIB_DIR})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include (ConfigureChecks.cmake)
configure_file(config-kgrab.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgrab.h)

########### next target ###############

set(kgrab_SRCS
   main.cpp
   kgrab.cpp
   regiongrabber.cpp
   windowgrabber.cpp
   openwith.cpp
   kgrab_settings.cpp)

qt4_add_dbus_adaptor(kgrab_SRCS org.kde.kgrab.xml kgrab.h KGrab)

kde4_automoc(${kgrab_SRCS})

kde4_add_ui_files(kgrab_SRCS kgrabwidget.ui kgrab_settings.ui)

kde4_add_executable(kgrab ${kgrab_SRCS})

target_link_libraries(kgrab ${KDE4_KDEPRINT_LIBS})

install(TARGETS kgrab DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install(FILES kgrab.desktop DESTINATION ${XDG_APPS_DIR})
install(FILES org.kde.kgrab.xml DESTINATION ${DBUS_INTERFACES_DIR})
install(FILES kgrabui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kgrab)

kde4_install_icons(${ICON_INSTALL_DIR} )
