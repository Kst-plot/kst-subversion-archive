include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

########### next target ###############

set(kstfit_gradient_unweighted_LIB_SRCS kstfit_gradient_unweighted.cpp)

ADD_LIBRARY(kstfit_gradient_unweighted SHARED ${kstfit_gradient_unweighted_LIB_SRCS})

target_link_libraries(kstfit_gradient_unweighted ${GSL_LIBS})

set_target_properties(kstfit_gradient_unweighted PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_gradient_unweighted 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_gradient_unweighted.xml DESTINATION lib/kde4/kstplugins)
