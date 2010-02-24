include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(butterworth_bandstop_LIB_SRCS butterworth_bandstop.cpp)

ADD_LIBRARY(butterworth_bandstop SHARED ${butterworth_bandstop_LIB_SRCS})

target_link_libraries(butterworth_bandstop ${GSL_LIBS})

set_target_properties(butterworth_bandstop PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS butterworth_bandstop 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  butterworth_bandstop.xml DESTINATION lib/kde4/kstplugins)
