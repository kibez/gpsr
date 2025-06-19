#ifndef fenix_exportH
#define fenix_exportH
#include <class_export.h>

#ifdef FENIX_LIB
# define FENIX_EXPORT
#else
# ifdef FENIX_EXPORTING
#  define FENIX_EXPORT _DLL_EXPORT
# else
#  define FENIX_EXPORT _DLL_IMPORT
# endif
#endif

#endif

