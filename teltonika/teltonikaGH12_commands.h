//---------------------------------------------------------------------------

#ifndef teltonikaGH12_commandsH
#define teltonikaGH12_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "teltonikaGH12_export.h"

namespace VisiPlug { namespace Teltonika {

struct TELTONIKAGH12_EXPORT common_t
{
	unsigned char satellites;
  unsigned network;
  unsigned location;
  unsigned cell_id;
  unsigned rssi;
  unsigned battery;
  bool usb_connected;
  unsigned live_time;
  unsigned hdop;
  unsigned vdop;
  unsigned pdop;

	bool satellites_valid;
  bool network_valid;
  bool location_valid;
  bool cell_id_valid;
  bool rssi_valid;
  bool battery_valid;
  bool usb_connected_valid;
  bool live_time_valid;
  bool hdop_valid;
  bool vdop_valid;
  bool pdop_valid;

	common_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct TELTONIKAGH12_EXPORT track_t
{
  bool enable;
  bool add_param;
  int interval;
  int duration;

  track_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const track_t& rhs) const;
};

struct TELTONIKAGH12_EXPORT inf_t : public common_t
{
  std::string hwid;
  std::string version;
  int profile;
  
  inf_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const inf_t& rhs) const;
};

struct TELTONIKAGH12_EXPORT zone_t : public common_t
{
  bool inside;
  std::string zone_name;

  zone_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const zone_t& rhs) const;
};

} }//namespace
#endif

