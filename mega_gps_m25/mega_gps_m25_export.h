#ifndef mega_gps_m25H
#define mega_gps_m25H
#include <class_export.h>

#ifdef MEGA_GPS_M25_LIB
# define MEGA_GPS_M25_EXPORT
#else
# ifdef MEGA_GPS_M25_EXPORTING
#  define MEGA_GPS_M25_EXPORT _DLL_EXPORT
# else
#  define MEGA_GPS_M25_EXPORT _DLL_IMPORT
# endif
#endif

#endif

