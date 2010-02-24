include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_gaussian_LIB_SRCS kstfit_gaussian.cpp)

ADD_LIBRARY(kstfit_gaussian SHARED ${kstfit_gaussian_LIB_SRCS})

target_link_libraries(kstfit_gaussian ${GSL_LIBS})

set_target_properties(kstfit_gaussian PROPERTIES 
	PREFIX ""
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_gaussian 
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_gaussian.xml DESTINATION lib/kde4/kstplugins)
