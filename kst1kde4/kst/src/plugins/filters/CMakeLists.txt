include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(scalarfilter_LIB_SRCS scalarfilter.c)

ADD_LIBRARY(scalarfilter SHARED ${scalarfilter_LIB_SRCS})

target_link_libraries(scalarfilter)

set_target_properties(scalarfilter PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS scalarfilter 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  scalarfilter.xml DESTINATION lib/kde4/kstplugins)
