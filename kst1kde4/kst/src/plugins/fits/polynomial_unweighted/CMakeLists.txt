include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_polynomial_unweighted_LIB_SRCS kstfit_polynomial_unweighted.cpp)

ADD_LIBRARY(kstfit_polynomial_unweighted SHARED ${kstfit_polynomial_unweighted_LIB_SRCS})

target_link_libraries(kstfit_polynomial_unweighted ${GSL_LIBS})

set_target_properties(kstfit_polynomial_unweighted PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_polynomial_unweighted 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_polynomial_unweighted.xml DESTINATION lib/kde4/kstplugins)
