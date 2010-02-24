include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_kneefrequency_unweighted_LIB_SRCS kstfit_kneefrequency_unweighted.cpp)

ADD_LIBRARY(kstfit_kneefrequency_unweighted SHARED ${kstfit_kneefrequency_unweighted_LIB_SRCS})

target_link_libraries(kstfit_kneefrequency_unweighted)

set_target_properties(kstfit_kneefrequency_unweighted PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_kneefrequency_unweighted 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_kneefrequency_unweighted.xml DESTINATION lib/kde4/kstplugins)
