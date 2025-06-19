#ifndef radom_exportH
#define radom_exportH
#include <class_export.h>

#ifdef RADOM_EXPORTING
# define RADOM_EXPORT _DLL_EXPORT
#else
# define RADOM_EXPORT _DLL_IMPORT
#endif

#endif
