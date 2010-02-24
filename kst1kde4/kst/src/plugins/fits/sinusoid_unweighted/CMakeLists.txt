include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_sinusoid_unweighted_LIB_SRCS kstfit_sinusoid_unweighted.cpp)

ADD_LIBRARY(kstfit_sinusoid_unweighted SHARED ${kstfit_sinusoid_unweighted_LIB_SRCS})

target_link_libraries(kstfit_sinusoid_unweighted ${GSL_LIBS})

set_target_properties(kstfit_sinusoid_unweighted PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_sinusoid_unweighted 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_sinusoid_unweighted.xml DESTINATION lib/kde4/kstplugins)
