#ifndef nd_exportH
#define nd_exportH
#include <class_export.h>

#ifdef ND_LIB
# define ND_EXPORT
#else
# ifdef ND_EXPORTING
#  define ND_EXPORT _DLL_EXPORT
# else
#  define ND_EXPORT _DLL_IMPORT
# endif
#endif

#endif

