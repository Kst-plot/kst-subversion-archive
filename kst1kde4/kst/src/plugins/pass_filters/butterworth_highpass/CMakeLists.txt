include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(butterworth_highpass_LIB_SRCS butterworth_highpass.cpp)

ADD_LIBRARY(butterworth_highpass SHARED ${butterworth_highpass_LIB_SRCS})

target_link_libraries(butterworth_highpass ${GSL_LIBS})

set_target_properties(butterworth_highpass PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS butterworth_highpass 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  butterworth_highpass.xml DESTINATION lib/kde4/kstplugins)
