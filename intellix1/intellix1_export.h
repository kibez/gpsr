#ifndef intellix1_exportH
#define intellix1_exportH
#include <class_export.h>

#ifdef INTELLIX1_LIB
# define INTELLIX1_EXPORT
#else
# ifdef INTELLIX1_EXPORTING
#  define INTELLIX1_EXPORT _DLL_EXPORT
# else
#  define INTELLIX1_EXPORT _DLL_IMPORT
# endif
#endif

#endif

