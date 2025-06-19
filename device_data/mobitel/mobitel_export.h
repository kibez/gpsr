#ifndef mobitel_exportH
#define mobitel_exportH
#include <class_export.h>

#ifdef MOBITEL_LIB
#  define MOBITEL_EXPORT
#else
#  ifdef MOBITEL_EXPORTING
#    define MOBITEL_EXPORT _DLL_EXPORT
#  else
#    define MOBITEL_EXPORT _DLL_IMPORT
#  endif
#endif

#endif
