//---------------------------------------------------------------------------

#ifndef mobitel_commandsH
#define mobitel_commandsH
#include <string>

#include "mobitel_export.h"
//---------------------------------------------------------------------------
namespace VisiPlug { namespace Mobitel {

struct MOBITEL_EXPORT password
{
	std::string old_password;
	std::string new_password;
	bool unpack(const void* d);
	void* pack();
	bool operator==(const password& val) const;
};

struct MOBITEL_EXPORT phone
{
	std::string number;
	bool unpack(const void* d);
	void* pack();
	bool operator==(const phone& val) const;
};

struct MOBITEL_EXPORT rele
{
	static const int count=4;
	static const int zero=0;
	static const int one=1;
	static const int reverse=2;
	static const int no_change=3;
	int v[4];

	bool unpack(const void* d);
  void* pack();

  rele();
  bool operator==(const rele& val) const;
  rele& operator=(const rele& val);
};

struct MOBITEL_EXPORT log_save_time
{
  int log_save_period;

  log_save_time();
  bool unpack(const void* d);
  void* pack();
  bool operator==(const log_save_time& val) const;
};

// Класс делится на две команды
// одна использует позицию, другая радиус
struct MOBITEL_EXPORT zone
{
	int radius;
	double latitude;
	double longitude;

	zone();
	bool unpack(const void* d);
	void* pack();
	bool operator==(const zone& val) const;
};

struct MOBITEL_EXPORT in_mask
{
	static const int count=8;

	static const int active_zero=0;
	static const int active_one=1;
	static const int active_toggle=2;
	static const int off=3;

	int v[8];

	in_mask();
	bool unpack(const void* d);
	void* pack();
	bool operator==(const in_mask& val) const;
	in_mask& operator=(const in_mask& val);
};

struct MOBITEL_EXPORT enable_guard_t
{
	bool enable_guard;
	enable_guard_t();
	bool unpack(const void* d);
	void* pack();
	bool operator==(const enable_guard_t& val) const;
};

struct MOBITEL_EXPORT enable_in_t
{
	bool enable_in;
	enable_in_t();
	bool unpack(const void* d);
	void* pack();
	bool operator==(const enable_in_t& val) const;
};

struct MOBITEL_EXPORT input
{
	static const int count=8;
	bool v[8];

  input();
  bool unpack(const void* d);
  void* pack();
  bool operator==(const input& val) const;
  input& operator=(const input& val);
};

struct MOBITEL_EXPORT log_distance
{
  int log_min_distance;

  log_distance();
  bool unpack(const void* d);
  void* pack();
  bool operator==(const log_distance& val) const;
};


struct MOBITEL_EXPORT log_alert_sms_period
{
  int alert_sms_period;

  log_alert_sms_period();
  bool unpack(const void* d);
  void* pack();
  bool operator==(const log_alert_sms_period& val) const;
};

struct MOBITEL_EXPORT log_state
{
  bool is_logging;
  int log_size;

  log_state();
  bool unpack(const void* d);
  void* pack();
  bool operator==(const log_state& val) const;
};

struct MOBITEL_EXPORT state :
   public log_state,
   public log_save_time, 
   public log_distance,
	 public enable_in_t,
	 public in_mask,
	 public rele,
   public enable_guard_t,
   public zone,
   public log_alert_sms_period
{
  bool unpack(const void* d);
  void* pack();
  bool operator==(const state& val) const;
};

} }//namespace

#endif
