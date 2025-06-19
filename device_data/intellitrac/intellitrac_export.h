#ifndef intellitrac_exportH
#define intellitrac_exportH
#include <class_export.h>

#ifdef INTELLITRAC_LIB
# define INTELLITRAC_EXPORT
#else
# ifdef INTELLITRAC_EXPORTING
#  define INTELLITRAC_EXPORT _DLL_EXPORT
# else
#  define INTELLITRAC_EXPORT _DLL_IMPORT
# endif
#endif

#endif

