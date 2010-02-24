include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_exponential_LIB_SRCS kstfit_exponential.cpp)

ADD_LIBRARY(kstfit_exponential SHARED ${kstfit_exponential_LIB_SRCS})

target_link_libraries(kstfit_exponential ${GSL_LIBS})

set_target_properties(kstfit_exponential PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_exponential 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_exponential.xml DESTINATION lib/kde4/kstplugins)
