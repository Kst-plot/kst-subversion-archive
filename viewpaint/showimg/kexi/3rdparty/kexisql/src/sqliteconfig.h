#ifdef _WIN32
# define SQLITE_PTR_SZ 4
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
# define SQLITE_PTR_SZ SIZEOF_CHAR_P
#endif

