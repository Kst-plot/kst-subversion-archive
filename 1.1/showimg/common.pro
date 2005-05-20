
include( $(QKW)/kwcommon.pro )

VERSION=094

DEFINES += \
SHOWIMG_NO_CDARCHIVE \
SHOWIMG_NO_SCAN_IMAGE

# dlls suffixes for given target

SHOWIMG_SUFFIX="$$KDEBUG""$$VERSION".lib


