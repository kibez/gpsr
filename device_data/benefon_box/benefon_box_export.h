#ifndef benefon_box_exportH
#define benefon_box_exportH
#include <class_export.h>

#ifdef BENEFON_BOX_EXPORTING
# define BENEFON_BOX_EXPORT _DLL_EXPORT
#else
# define BENEFON_BOX_EXPORT _DLL_IMPORT
#endif

#endif
