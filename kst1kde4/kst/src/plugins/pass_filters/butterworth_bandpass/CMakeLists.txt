include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(butterworth_bandpass_LIB_SRCS butterworth_bandpass.cpp)

ADD_LIBRARY(butterworth_bandpass SHARED ${butterworth_bandpass_LIB_SRCS})

target_link_libraries(butterworth_bandpass ${GSL_LIBS})

set_target_properties(butterworth_bandpass PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS butterworth_bandpass 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  butterworth_bandpass.xml DESTINATION lib/kde4/kstplugins)
