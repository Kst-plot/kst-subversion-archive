include_directories(${CMAKE_SOURCE_DIR} ${QT_INCLUDES})

########### next target ###############

set(kstextdate_LIB_SRCS
    kstextdatetime.cpp
    kstextcalendarsystem.cpp
    kstextcalendarsystemgregorian.cpp)

#kde4_automoc(${kstextdate_LIB_SRCS})

ADD_LIBRARY(kstextdate SHARED ${kstextdate_LIB_SRCS})

target_link_libraries(kstextdate QtCore)

set_target_properties(kstextdate PROPERTIES VERSION 1.2.0 SOVERSION 1)
install(TARGETS kstextdate 
	LIBRARY DESTINATION lib
	${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES  kstextdatetime.h kstextcalendarsystem.h kstextcalendarsystemgregorian.h DESTINATION include)
