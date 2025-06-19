#ifndef thuraya_exportH
#define thuraya_exportH
#include <class_export.h>

#ifdef THURAYA_EXPORTING
# define THURAYA_EXPORT _DLL_EXPORT
#else
# define THURAYA_EXPORT _DLL_IMPORT
#endif

#endif
