include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(despike_LIB_SRCS despike.cpp)

ADD_LIBRARY(despike SHARED ${despike_LIB_SRCS})

target_link_libraries(despike)

set_target_properties(despike PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS despike 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  despike.xml DESTINATION lib/kde4/kstplugins)
