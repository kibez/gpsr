//---------------------------------------------------------------------------
#ifndef mega_gps_m25_notifyH
#define mega_gps_m25_notifyH
#include "mega_gps_m25_commands.h"
#include <vector>
#include <string>
//---------------------------------------------------------------------------
namespace VisiPlug { namespace MegaGps {


struct MEGA_GPS_M25_EXPORT connect_t
{
	std::string imei;

  unsigned soft_ver;
  unsigned sign;
  std::string phone_number;
  std::string sim_iccid;
  unsigned hw_ver;
  std::vector<unsigned char> bootinfo;

  bool soft_ver_valid;
  bool sign_valid;
  bool phone_number_valid;
  bool sim_iccid_valid;
  bool hw_ver_valid;
  bool bootinfo_valid;

  connect_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const connect_t& rhs) const;
};

// Aborigen 07.10.2012
struct MEGA_GPS_M25_EXPORT gps_valid_t
{
  unsigned gps_valid;

  gps_valid_t() {gps_valid = 0;}
  void* pack() const;
  bool unpack(const void* d);
  bool operator==(const gps_valid_t& rhs) const;
};
// End Aborigen

struct MEGA_GPS_M25_EXPORT synchro_t
{
  unsigned short packet_num;

  synchro_t() {packet_num = 0;}
  void* pack() const;
  bool unpack(const void* d);
  bool operator==(const synchro_t& rhs) const;
};

} }//namespace
#endif

