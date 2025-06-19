
#ifndef drozd_notifyH
#define drozd_notifyH

#include "drozd_export.h"  
#include <string>

#define ET_CFREQU_INFO    0x0047
#define ET_CFREQU2_INFO   0x004A
#define ET_THHMDT_INFO    0x0051
#define ET_ACCOUNT_INFO   0x0052
#define ET_ENGINE_START   0x0053
#define ET_ENGINE_STOP    0x0054
#define ET_TRANS_STATE    0x0088
#define ET_FUEL_CHARGE    0x0090
#define ET_FUEL_DISCHARGE 0x0091
#define ET_FUEL_COUNTER   0x0094
#define ET_MECH_STATE     0x00A3
#define ET_THS1_INFO      0x00A8

namespace VisiPlug { namespace Drozd {

struct DROZD_EXPORT track_ev_t
{
  char satellites;
  char gsm_level;

  track_ev_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const track_ev_t& rhs) const;
};

struct DROZD_EXPORT fuel_change_t
{
	char chanel;
	short value;

	fuel_change_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const fuel_change_t& rhs) const;
};

struct DROZD_EXPORT fuel_counter_t
{
  char chanel;
  char fail;
  float value;
  float prev_value;

  fuel_counter_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const fuel_counter_t& rhs) const;
};

struct thhmdt_info_t
{
  char chanel;
  float temperature;
  float air_humidity;

  thhmdt_info_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const thhmdt_info_t& rhs) const;
};

struct account_info_t
{
  std::string data;

  account_info_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const account_info_t& rhs) const;
};

struct mech_state_t
{
  unsigned short serial_number;
  float ck;  // угол крена (±180∞)
  float ct;  // угол тангажа (±90∞)
  float temperature;
  unsigned short can_id;

  mech_state_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const mech_state_t& rhs) const;
};

struct ths1_info_t
{
  unsigned short can_id;
  char input_number;
  float temperature;

  ths1_info_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const ths1_info_t& rhs) const;
};

struct trans_state_t
{
  unsigned short serial_number;
  char state; // состо€ние дифференциала (0 Ц норм, 1 Ц блок)
  char speed_number; // номер передачи

  trans_state_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const trans_state_t& rhs) const;
};

struct engine_state_t
{
  unsigned short id;
  bool start; // true - start; false - stop

  engine_state_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const engine_state_t& rhs) const;
};

struct fuel_level_t
{
  unsigned short frequency;
  short full_fuel_level;
  short sensor1_fuel_level;
  unsigned char moveX;
  unsigned char moveY;
  unsigned char moveZ;
  unsigned char chanel;
  bool is_chanel;

  fuel_level_t();
  void* pack() const;
	bool unpack(const void *d);
	bool operator==(const fuel_level_t& rhs) const;
};

}}

#endif
