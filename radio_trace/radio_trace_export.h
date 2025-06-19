#ifndef radio_trace_exportH
#define radio_trace_export.h
#include <class_export.h>

#ifdef RADIO_TRACE_LIB
#  define RADIO_TRACE_EXPORT
#else
#  ifdef RADIO_TRACE_EXPORTING
#    define RADIO_TRACE_EXPORT _DLL_EXPORT
#  else
#    define RADIO_TRACE_EXPORT _DLL_IMPORT
#  endif
#endif

#endif
