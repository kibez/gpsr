//---------------------------------------------------------------------------

#ifndef phantom_commandsH
#define phantom_commandsH

#include <vector>
#include <string>
#include "phantom_export.h"
#include "../easytrac/easytrac_commands.h"
//---------------------------------------------------------------------------

namespace VisiPlug { namespace Phantom {

struct /*KIBEZ PHANTOM_EXPORT*/ phantom_common_t : VisiPlug::EasyTrac::common_t
{
  unsigned int id;
  unsigned short sms_count;
  bool sos;

  bool sms_count_valid;

  phantom_common_t() : VisiPlug::EasyTrac::common_t() {id = sms_count = 0; sms_count_valid = sos = false;}
        void* pack() const;
	bool unpack(const void *d);
	bool operator==(const phantom_common_t& rhs) const;
};

struct PHANTOM_EXPORT set_timer_t
{
  enum ETimers{ tm_main = 0, tm_gps, tm_gsm };
  ETimers timer;
  unsigned short time;

  set_timer_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const set_timer_t& rhs) const;
};

struct /*KIBEZ PHANTOM_EXPORT*/ telnum_t
{
  std::string telnum;

  telnum_t(){}
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const telnum_t& rhs) const;
};

struct PHANTOM_EXPORT all_settings_t
{
  unsigned int id;
  std::string telnum;
  unsigned short main_timer;
  unsigned short gps_timer;
  unsigned short gsm_timer;
  std::string sw_ver;
  std::string hw_ver;
  std::string battery_volt;
  unsigned char percent_battery;
  std::string external_volt;
  unsigned short sms_counter;
  unsigned char gsm_level;
  unsigned char gps_satel;

  all_settings_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const all_settings_t& rhs) const;
};

}}

#endif
