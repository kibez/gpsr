#ifndef g200_exportH
#define g200_exportH
#include <class_export.h>

#ifdef G200_LIB
# define G200_EXPORT
#else
# ifdef G200_EXPORTING
#  define G200_EXPORT _DLL_EXPORT
# else
#  define G200_EXPORT _DLL_IMPORT
# endif
#endif

#endif

