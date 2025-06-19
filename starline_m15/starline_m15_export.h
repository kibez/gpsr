#ifndef starline_m15_exportH
#define starline_m15_exportH
#include <class_export.h>

#ifdef STARLINE_M15_LIB
# define STARLINE_M15_EXPORT
#else
# ifdef STARLINE_M15_EXPORTING
#  define STARLINE_M15_EXPORT _DLL_EXPORT
# else
#  define STARLINE_M15_EXPORT _DLL_IMPORT
# endif
#endif

#endif
 