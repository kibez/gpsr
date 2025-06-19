#ifndef pantrack_exportH
#define pantrack_exportH
#include <class_export.h>

#ifdef PANTRACK_LIB
#  define PANTRACK_EXPORT
#else
#  ifdef PANTRACK_EXPORTING
#    define PANTRACK_EXPORT _DLL_EXPORT
#  else
#    define PANTRACK_EXPORT _DLL_IMPORT
#  endif
#endif

#endif
