include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_CURRENT_SOURCE_DIR} ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

link_directories(${CMAKE_SOURCE_DIR}/src/kstlib)

########### next target ###############

set(kstmath_LIB_SRCS
    kstpainter.cpp
    kstcolorsequence.cpp
    kstsettings.cpp
    kstobjectdefaults.cpp
    labelparser.cpp
    kstdataobject.cpp
    kstdataobjectcollection.cpp
    kstcsd.cpp
    kstpsd.cpp
    kstbasecurve.cpp
    kstcurvepointsymbol.cpp
    kstlinestyle.cpp
    kstvcurve.cpp
    kstimage.cpp
    kstdefaultnames.cpp
    ksthistogram.cpp
    kstequation.cpp
    kstcurvehint.cpp
    kstcplugin.cpp
    kstbasicplugin.cpp
    fftsg_h.c
    enodes.cpp
    enodefactory.cpp
    plugincollection.cpp
    plugin.cpp
    pluginxmlparser.cpp
    pluginloader.cpp
    dialoglauncher.cpp
    eparse-eh.cpp
    eparse.c
    psdcalculator.cpp
    escan.c
    kstvectorview.cpp)

kde4_add_library(kstmath SHARED ${kstmath_LIB_SRCS})

target_link_libraries(kstmath ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_LIBRARIES} kstbase)

set_target_properties(kstmath PROPERTIES VERSION ${KST_LIBKST_VERSION} SOVERSION 1)

install(TARGETS kstmath ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
