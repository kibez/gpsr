//---------------------------------------------------------------------------
#ifndef intellia1_commandsH
#define intellia1_commandsH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "intellia1_export.h"
#include "intellitrac_commands.h"

namespace VisiPlug { namespace Intellia1 {

const unsigned input_count=8;
const unsigned output_count=8;
const unsigned report_count=50;
const unsigned schedule_count=8;
const unsigned zone_count=50;

enum ReportT {rp_position,rp_log_position,rp_track_position,
           rp_ignition_alert=11,rp_input_base=12,
           rp_rfid=21,rp_barcode=35,rp_ign_on=50,rp_input_on_base=51,
           rp_user_base=100,
           rp_power_up=160,rp_engine,rp_speed_high,rp_speed_normal,
           rp_enter_zone,rp_exit_zone,
           rp_main_power_low,rp_main_power_absent,rp_bat_power_low,rp_bat_power_absent,
           rp_synchro=171,rp_gps_lock_fail,rp_gps_antena_fail,rp_home,
           rp_main_power_normal,rp_bat_power_normal,
           rp_tow,rp_gps_module_fail,rp_motion,rp_impact,rp_pre_impact,rp_post_impact,
           rp_idle_begin,rp_idle_end,rp_analog1,rp_analog2,
           rp_low_power_start=193,rp_low_power_end=194,
           rp_ccd=256
           };

typedef VisiPlug::Intellitrac::common_t common_t;

struct INTELLIA1_EXPORT hosts_t
{
	bool set;
  int index;
  std::vector<bool> fqdn;
  std::vector<std::string> host;
  std::vector<int> port;

	hosts_t(){set=true;index=1;}
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const hosts_t& rhs) const;
};

struct INTELLIA1_EXPORT mileage_t
{
	bool set;
  double val;
  bool ign_reset;
  bool egn_reset;

	mileage_t(){set=true;val=0.0;ign_reset=false;egn_reset=false;}
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const mileage_t& rhs) const;
};

struct INTELLIA1_EXPORT version_t
{
  std::string fw_version;
  std::string hw_version;
  std::string gsm_version;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const version_t& rhs) const;
};

enum GsmState{gsm_not_registered,gsm_registered,gsm_searching,gsm_registration_denied,gsm_unknown,gsm_registered_roaming};

struct INTELLIA1_EXPORT qust_t
{
  std::string op_name;
  int csq;
  int gprs_state;
  int gsm_state;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const qust_t& rhs) const;
};

struct INTELLIA1_EXPORT outs_t
{
	bool set;
	unsigned output_id;
	bool active;
	int first_duration;
	int next_duration;
	int toggle_times;
	unsigned char schedule;

	outs_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const outs_t& rhs) const;
};

struct INTELLIA1_EXPORT track_t
{
	enum{md_time=0x1,md_distance=0x2,md_ignition=0x4,md_heading=0x8};
	enum{act_log=0x1,act_track=0x2,act_COM=0x4,act_SMS=0x8};

	bool set;
	int mode;
	int timestamp;
	unsigned short distance;
	unsigned char heading;
  int destination;
  unsigned char schedule;
  int delay;
  int time_multiplier;
  bool persist;

	track_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const track_t& rhs) const;
};

struct INTELLIA1_EXPORT schedule_t
{
	enum{mon=0x1,tue=0x2,wed=0x4,thu=0x8,fri=0x10,sat=0x20,sun=0x40};

	bool set;
  int sched_id;
	int start_time;
	int duration;
	unsigned char day_mask;

	schedule_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const schedule_t& rhs) const;
};

struct INTELLIA1_EXPORT base_stations_t
{
  std::string mcc;
  std::string mnc;
  std::string lac;
  std::vector<std::string> cellid;
  std::vector<int> rxl;

	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const base_stations_t& rhs) const;
};

struct INTELLIA1_EXPORT event_setup_t
{
  enum FuncT{esf_ignition,esf_engine,esf_low_power,esf_power_on,esf_input,esf_zone,esf_home_zone,
             esf_idle,esf_tow,esf_motion,esf_impact,esf_speed,esf_gps_alert};

  static const int power_mask_go_low=1;
  static const int power_mask_wake_very_low=2;
  static const int power_mask_wake_low=4;

  enum ZoneEvents{ze_disable,ze_enter_exit,ze_enter,ze_exit};

  static const int action_logging=1;
  static const int action_polling=2;

  static const int force_mask_create_gprs=1;
  static const int force_mask_send_sms=2;
  static const int force_mask_serial=4;

	bool set;
  FuncT func;
  int option;
  int action;
  int schedule;
  int force;

  int index;
  int power_on_enable;
  int report_mode;

	event_setup_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const event_setup_t& rhs) const;
};

struct INTELLIA1_EXPORT zone_t
{
  enum FuncT{esf_set_zone,esf_clear_zone,esf_speed,esf_home};

  enum ZoneTypeT{zt_circle,zt_rectangle};

	bool set;
  FuncT func;
  int index;
  ZoneTypeT zone_type;

  double lat;
  double lon;
  unsigned enter_radius;
  unsigned exit_radius;

  double lat1;
  double lon1;
  double lat2;
  double lon2;

  unsigned min_move;
  unsigned delay;
  unsigned sustained_motion;

  bool enable;
  unsigned speed;

	zone_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const zone_t& rhs) const;
};

struct INTELLIA1_EXPORT tow_t
{
	bool set;
  unsigned distance;
	unsigned delay;
	unsigned fix_count;

	tow_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const tow_t& rhs) const;
};

struct INTELLIA1_EXPORT power_t
{
	bool set;
  unsigned main_full;
  unsigned main_low;
  unsigned main_fail;
  unsigned bat_full;
  unsigned bat_low;
  unsigned bat_fail;
  unsigned duration;
  bool led_off;

	power_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const power_t& rhs) const;
};

struct INTELLIA1_EXPORT power_save_t
{
	bool set;
  unsigned ignition_inactive;
  unsigned no_motion;
  unsigned no_communication;
  unsigned low_wakeup;
  unsigned low_on;
  unsigned low_transition;
  unsigned vlow_wakeup;
  unsigned vlow_on;

	power_save_t();
	void* pack() const;
	bool unpack(const void* d);
	bool operator==(const power_save_t& rhs) const;
};

} }//namespace
#endif

