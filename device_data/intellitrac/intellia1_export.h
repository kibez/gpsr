#ifndef intellia1_exportH
#define intellia1_exportH
#include <class_export.h>

#ifdef INTELLIA1_LIB
# define INTELLIA1_EXPORT
#else
# ifdef INTELLIA1_EXPORTING
#  define INTELLIA1_EXPORT _DLL_EXPORT
# else
#  define INTELLIA1_EXPORT _DLL_IMPORT
# endif
#endif

#endif

