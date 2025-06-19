#ifndef phantom_exportH
#define phantom_exportH
#include <class_export.h>

#ifdef PHANTOM_LIB
# define PHANTOM_EXPORT
#else
# ifdef PHANTOM_EXPORTING
#  define PHANTOM_EXPORT _DLL_EXPORT
# else
#  define PHANTOM_EXPORT _DLL_IMPORT
# endif
#endif

#endif
