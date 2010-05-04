include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate ${CMAKE_SOURCE_DIR}/src/libkst ${CMAKE_SOURCE_DIR}/src/libkstmath ${CMAKE_SOURCE_DIR}/src/widgets ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstkde_LIB_SRCS
    kcolorbutton.cpp
    kcolorcollection.cpp
    kcolorcombo.cpp
    kdualcolorbutton.cpp
    kled.cpp
    ksttimezones.cpp
    ktimezonecombo.cpp)

kde4_add_library(kstkde SHARED ${kstkde_LIB_SRCS})

target_link_libraries(kstkde QtCore QtGui)

set_target_properties(kstkde PROPERTIES VERSION ${KST_LIBKST_VERSION} SOVERSION 1)
install(TARGETS kstkde 
	LIBRARY DESTINATION lib
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
