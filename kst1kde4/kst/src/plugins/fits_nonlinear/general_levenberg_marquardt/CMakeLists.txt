include_directories(${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR})

########### next target ###############

set(kstfit_general_levenberg_marquardt_LIB_SRCS kstfit_general_levenberg_marquardt.cpp)

ADD_LIBRARY(kstfit_general_levenberg_marquardt SHARED ${kstfit_general_levenberg_marquardt_LIB_SRCS})

target_link_libraries(kstfit_general_levenberg_marquardt ${GSL_LIBS} ${MUPARSER_LIBS})

set_target_properties(kstfit_general_levenberg_marquardt PROPERTIES 
	PREFIX ""
	COMPILE_FLAGS -fexceptions
	VERSION ${KST_LIBKST_VERSION} 
	SOVERSION 1)

install(TARGETS kstfit_general_levenberg_marquardt
        LIBRARY DESTINATION lib/kde4/kstplugins
        ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstfit_general_levenberg_marquardt.xml DESTINATION lib/kde4/kstplugins)
