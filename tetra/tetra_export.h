#ifndef tetra_exportH
#define tetra_exportH
#include <class_export.h>

#ifdef TETRA_LIB
# define TETRA_EXPORT
#else
# ifdef TETRA_EXPORTING
#  define TETRA_EXPORT _DLL_EXPORT
# else
#  define TETRA_EXPORT _DLL_IMPORT
# endif
#endif

#endif

