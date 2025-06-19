//---------------------------------------------------------------------------
#ifndef fenix_commandsH
#define fenix_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "fenix_export.h"

namespace VisiPlug { namespace Fenix {

struct FENIX_EXPORT common_t
{
  double battery;
  double temp;
  bool sleep_hour;
  bool gps_on;
  bool sms_on;
  unsigned sleep_time;
  unsigned gps_time;
  unsigned gsm_time;

  unsigned channel;
  unsigned rssi;
  unsigned country;
  unsigned network;
  unsigned bsic;
  unsigned cell_id;

  std::string balans;

	common_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const common_t& rhs) const;
};

struct FENIX_EXPORT cmd_t
{
  std::string cmd;
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const cmd_t& rhs) const{return cmd==rhs.cmd;}
};

struct sim_t : public cmd_t
{
  sim_t(){cmd="K--set";}
};

struct cfg_t : public cmd_t
{
  cfg_t(){cmd="B24set";}
};

struct FENIX_EXPORT integer_t
{
	int val;
	integer_t(){val=0;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const integer_t& rhs) const{return val==rhs.val;}
};

struct minutes_t : public integer_t
{
  minutes_t() {val=1;}
};

struct FENIX_EXPORT pwm_t
{
  bool is_hour;
	int val;
	pwm_t(){is_hour=true;val=1;}
	void* pack() const;
	bool unpack(const void* d);
	inline bool operator==(const pwm_t& rhs) const{return is_hour==rhs.is_hour&&val==rhs.val;}
};

struct FENIX_EXPORT gsm_t
{
  unsigned channel;
  unsigned rssi;
  unsigned quality;
  unsigned country;
  unsigned network;
  unsigned bsic;
  unsigned cell_id;
  unsigned lac;

  std::vector<unsigned> channels;
  std::vector<unsigned> levels;
  std::vector<unsigned> bsics;

	gsm_t();
	void* pack() const;
	bool unpack(const void *d);
	bool operator==(const gsm_t& rhs) const;
};

} }//namespace
#endif

