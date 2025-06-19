#ifndef drozd_exportH
#define drozd_exportH
#include <class_export.h>

#ifdef DROZD_LIB
# define DROZD_EXPORT
#else
# ifdef DROZD_EXPORTING
#  define DROZD_EXPORT _DLL_EXPORT
# else
#  define DROZD_EXPORT _DLL_IMPORT
# endif
#endif

#endif
 