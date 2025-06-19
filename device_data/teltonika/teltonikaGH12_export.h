#ifndef teltonikaGH12_exportH
#define teltonikaGH12_exportH
#include <class_export.h>

#ifdef TELTONIKAGH12_LIB
# define TELTONIKAGH12_EXPORT
#else
# ifdef TELTONIKAGH12_EXPORTING
#  define TELTONIKAGH12_EXPORT _DLL_EXPORT
# else
#  define TELTONIKAGH12_EXPORT _DLL_IMPORT
# endif
#endif

#endif

