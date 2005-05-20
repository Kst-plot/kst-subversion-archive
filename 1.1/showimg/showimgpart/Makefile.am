INCLUDES = -I$(top_srcdir)/showimg $(all_includes)

# These are not really libraries, but modules dynamically opened.
# So they should be installed in kde_module_dir, which is usually $kde_prefix/lib/kde3
kde_module_LTLIBRARIES = libshowimgpart.la

libshowimgpart_la_SOURCES = showimgpart.cpp
libshowimgpart_la_LIBADD = $(LIB_KPARTS) ../showimg/libshowimgcore.la
libshowimgpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

# Automatically generate moc files
METASOURCES = AUTO

# Install the XML-UI file to its correct location
showimgdir = $(kde_datadir)/showimgpart
showimg_DATA = showimgpart.rc

# Install the .desktop file into the kde_services directory
kde_services_DATA = showimgpart.desktop

