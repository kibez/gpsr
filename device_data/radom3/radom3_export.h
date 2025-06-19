#ifndef radom3_exportH
#define radom3_exportH
#include <class_export.h>

#ifdef RADOM3_EXPORTING
# define RADOM3_EXPORT _DLL_EXPORT
#else
# define RADOM3_EXPORT _DLL_IMPORT
#endif

#endif
