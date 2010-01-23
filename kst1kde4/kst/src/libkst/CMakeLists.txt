include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/extdate)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ksttimers.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ksttimers.h)

ADD_CUSTOM_COMMAND(
	PRE_BUILD 
	OUTPUT kstrevision.h 
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/kstrevision.h.sh ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET("kstrevision" ALL DEPENDS kstrevision.h)

########### next target ###############

set(kstbase_LIB_SRCS
#   ksdebug.cpp
    kstobject.cpp
    kstdatasource.cpp
    stdinsource.cpp
    kstextension.cpp
    kstdatacollection.cpp
    kstdebug.cpp
    rwlock.cpp
    kstmath.cpp
    kstdateparser.cpp
    kstscalar.cpp
#   $(PROCPS_COPY)
    kststring.cpp
    kstmatrix.cpp
    kstrmatrix.cpp
    kstsmatrix.cpp
    kstamatrix.cpp
    kstvector.cpp
    kstsvector.cpp
    kstavector.cpp
    kstrvector.cpp
    kstvectordefaults.cpp
    kstmatrixdefaults.cpp
    kstprimitive.cpp
    defaultprimitivenames.cpp)
#   kstwaitcondition_unix.cpp)

kde4_add_library(kstbase SHARED ${kstbase_LIB_SRCS})

target_link_libraries(kstbase ${KDE4_KDECORE_LIBS} kio kstextdate QtXml QtGui)

set_target_properties(kstbase PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS kstbase ${INSTALL_TARGETS_DEFAULT_ARGS})

ADD_DEPENDENCIES(kstbase kstrevision)

########### install files ###############

install(FILES  kstdatasource.h kstobject.h kst_export.h rwlock.h kstdateparser.h kstobjectcollection.h DESTINATION /include)

