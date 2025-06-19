#ifndef dedal_exportH
#define dedal_exportH
#include <class_export.h>

#ifdef DEDAL_LIB
#  define DEDAL_EXPORT
#else
#  ifdef DEDAL_EXPORTING
#    define DEDAL_EXPORT _DLL_EXPORT
#  else
#    define DEDAL_EXPORT _DLL_IMPORT
#  endif
#endif

#endif
