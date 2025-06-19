#ifndef common_device_exportH
#define common_device_exportH
#include <class_export.h>

#ifdef COMMON_DEVICE_LIB
#  define COMMON_DEVICE_EXPORT
#else
#  ifdef COMMON_DEVICE_EXPORTING
#    define COMMON_DEVICE_EXPORT _DLL_EXPORT
#  else
#    define COMMON_DEVICE_EXPORT _DLL_IMPORT
#  endif
#endif

#endif
