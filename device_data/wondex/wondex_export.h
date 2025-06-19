#ifndef wondex_exportH
#define wondex_exportH
#include <class_export.h>

#ifdef WONDEX_LIB
# define WONDEX_EXPORT
#else
# ifdef WONDEX_EXPORTING
#  define WONDEX_EXPORT _DLL_EXPORT
# else
#  define WONDEX_EXPORT _DLL_IMPORT
# endif
#endif

#endif

