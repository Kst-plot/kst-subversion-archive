include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(butterworth_lowpass_LIB_SRCS butterworth_lowpass.cpp)

ADD_LIBRARY(butterworth_lowpass SHARED ${butterworth_lowpass_LIB_SRCS})

target_link_libraries(butterworth_lowpass ${GSL_LIBS})

set_target_properties(butterworth_lowpass PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS butterworth_lowpass 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  butterworth_lowpass.xml DESTINATION lib/kde4/kstplugins)
