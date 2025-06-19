//---------------------------------------------------------------------------

#ifndef mega_gps_m25_commandsH
#define mega_gps_m25_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "mega_gps_m25_export.h"

namespace VisiPlug { namespace MegaGps {

struct MEGA_GPS_M25_EXPORT common_t
{
  unsigned dop;
  double v1;
  double v2;

  unsigned short lac;
  unsigned short ci;
  unsigned short mcc;
  unsigned short mnc;
  unsigned gsm_level;

  int inputs;
  int outputs;
  int temp;

	common_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct MEGA_GPS_M25_EXPORT enable_t
{
	bool val;

	enable_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const enable_t& rhs) const;
};

struct MEGA_GPS_M25_EXPORT set_timeouts_t
{
	unsigned short common; // GPS OFF or DATA NOT VALID
        unsigned short valid; // GPS DATA VALID - на стоянке
        unsigned short motion; // в движении
        unsigned char turn; // при повороте
        unsigned char angle; // минимальный угол поворота

	set_timeouts_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const set_timeouts_t& rhs) const;
};

} }//namespace
#endif

