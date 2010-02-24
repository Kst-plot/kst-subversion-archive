include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_exponential_weighted_LIB_SRCS kstfit_exponential_weighted.cpp)

ADD_LIBRARY(kstfit_exponential_weighted SHARED ${kstfit_exponential_weighted_LIB_SRCS})

target_link_libraries(kstfit_exponential_weighted ${GSL_LIBS})

set_target_properties(kstfit_exponential_weighted PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_exponential_weighted 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_exponential_weighted.xml DESTINATION lib/kde4/kstplugins)
