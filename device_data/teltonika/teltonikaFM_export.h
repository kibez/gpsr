#ifndef teltonikaFM_exportH
#define teltonikaFM_exportH
#include <class_export.h>

#ifdef TELTONIKAFM_LIB
# define TELTONIKAFM_EXPORT
#else
# ifdef TELTONIKAFM_EXPORTING
#  define TELTONIKAFM_EXPORT _DLL_EXPORT
# else
#  define TELTONIKAFM_EXPORT _DLL_IMPORT
# endif
#endif

#endif

