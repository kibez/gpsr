#ifndef benefon_exportH
#define benefon_exportH
#include <class_export.h>

#ifdef BENEFON_EXPORTING
# define BENEFON_EXPORT _DLL_EXPORT
#else
# define BENEFON_EXPORT _DLL_IMPORT
#endif

#endif
