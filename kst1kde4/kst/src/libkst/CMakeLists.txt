
find_package(KDE4 REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include(KDE4Defaults)

include(MacroLibrary)

#include(ConvenienceLibs.cmake)

#include(ManualStuff.cmake)

#include(ConfigureChecks.cmake)

include_directories(${CMAKE_SOURCE_DIR}/kst/src/extdate ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ksttimers.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ksttimers.h)


########### next target ###############

set(kst_LIB_SRCS
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
#    sysinfo.c)

kde4_add_library(kst SHARED ${kst_LIB_SRCS})

target_link_libraries(kst ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(kst PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kst ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES  kstdatasource.h kstobject.h kst_export.h rwlock.h kstdateparser.h kstobjectcollection.h DESTINATION ${INCLUDE_INSTALL_DIR})



#original Makefile.am contents follow:

#
#if include_linux
#PROCPS_COPY=sysinfo.c psversion.c
#endif
#
#lib_LTLIBRARIES = libkst.la
#
#libkst_la_SOURCES = \
#	kstcodecs.cpp \
#	ksdebug.cpp \
#	kstobject.cpp \
#	kstdatasource.cpp \
#	stdinsource.cpp \
#	kstextension.cpp \
#	kstdatacollection.cpp \
#	kstdebug.cpp \
#	rwlock.cpp \
#	kstmath.cpp \
#	kstdateparser.cpp \
#	ksttimezones.cpp \
#	kstscalar.cpp $(PROCPS_COPY) \
#	kststring.cpp \
#	kstmatrix.cpp \
#	kstrmatrix.cpp \
#	kstsmatrix.cpp \
#	kstamatrix.cpp \
#	kstvector.cpp \
#	kstsvector.cpp \
#	kstavector.cpp \
#	kstrvector.cpp \
#	kstvectordefaults.cpp \
#	kstmatrixdefaults.cpp \
#	kstprimitive.cpp \
#	defaultprimitivenames.cpp \
#	kstwaitcondition_unix.cpp
#
#libkst_la_LDFLAGS = -version-info @KST_LIBKST_VERSION@ -no-undefined $(all_libraries)
#libkst_la_LIBADD = $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE) ../extdate/libkstextdate.la
#
#include_HEADERS = kstdatasource.h kstobject.h kst_export.h kstsharedptr.h rwlock.h kstdateparser.h kstwaitcondition.h kstobjectcollection.h
#
#INCLUDES=-I$(top_srcdir)/kst/src/extdate -I$(top_srcdir) $(all_includes)
#
#METASOURCES=AUTO
#
#kstdebug.lo: kstrevision.h
#
#kstrevision.h:
#	@if (which svn >/dev/null 2>&1); then \
#	if (svn info $(srcdir) >/dev/null 2>&1); then \
#	REV=`svn info $(srcdir) | grep ^Revision | cut -b 11-`; \
#	else REV="dist"; fi; else REV="dist"; fi; \
#	echo "#ifndef KSTREVISION" >kstrevision.h.tmp; \
#	echo "#define KSTREVISION \""$REV"\"" >>kstrevision.h.tmp; \
#	echo "#endif" >>kstrevision.h.tmp; \
#	if (diff -N kstrevision.h.tmp kstrevision.h >/dev/null 2>&1); \
#	then rm kstrevision.h.tmp; else mv kstrevision.h.tmp kstrevision.h; fi
#
#noinst_HEADERS = kstdataplugin.h
