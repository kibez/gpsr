#ifndef wialonIPS_exportH
#define wialonIPS_exportH
#include <class_export.h>

#ifdef WIALONIPS_LIB
# define WIALONIPS_EXPORT
#else
# ifdef WIALONIPS_EXPORTING
#  define WIALONIPS_EXPORT _DLL_EXPORT
# else
#  define WIALONIPS_EXPORT _DLL_IMPORT
# endif
#endif

#endif

