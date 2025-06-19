#ifndef rvcl_exportH
#define rvcl_exportH
#include <class_export.h>

#ifdef RVCL_LIB
# define RVCL_EXPORT
#else
# ifdef RVCL_EXPORTING
#  define RVCL_EXPORT _DLL_EXPORT
# else
#  define RVCL_EXPORT _DLL_IMPORT
# endif
#endif

#endif

