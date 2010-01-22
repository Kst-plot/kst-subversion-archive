#ifndef CONFIG_H
#define CONFIG_H

#cmakedefine HAVE_STRLCPY
#cmakedefine HAVE_STRLCAT
#cmakedefine HAVE_SETENV
#cmakedefine HAVE_UNSETENV
#cmakedefine HAVE_LINUX
#cmakedefine KST_HAVE_READLINE

#define KSTVERSION "${KST_VERSION}"

#endif
